/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/cme/gateway/controller.hpp"

#include <utility>

#include "roq/utils/charconv/from_chars.hpp"

#include "roq/server/oms/exceptions.hpp"

using namespace std::literals;

namespace roq {
namespace cme {
namespace gateway {

// === HELPERS ===

namespace {
auto create_market_data_manager(auto &dispatcher, auto &settings, auto &security_definitions, auto &shared, auto &stream_id) {
  auto options = market_data::Options{
      .cache_all_reference_data = true,
      .enable_market_by_order = settings.misc.enable_market_by_order,
      .filter_snapshot_from_incremental = settings.misc.filter_snapshot_from_incremental,
      .local_interface = settings.multicast.local_interface,
      .multicast_timeout = settings.multicast.timeout,
      .secdef_config_file = settings.misc.secdef_config_file,
      .pcap_first_timestamp = {},
  };
  return market_data::Manager{dispatcher, options, security_definitions, settings.multicast.channel_ids, shared.mdp_config, stream_id};
}

template <typename R>
auto create_mdp_receivers(auto &settings, auto &context, auto &shared, auto &manager) {
  using result_type = std::remove_cvref_t<R>;
  result_type result;
  auto helper_1 = [&](auto channel_id, auto connection_type) {
    result.emplace_back(std::make_unique<MDPReceiver>(context, shared, manager, channel_id, connection_type, Priority::PRIMARY));
  };
  auto helper_2 = [&](auto channel_id, auto connection_type) {
    result.emplace_back(std::make_unique<MDPReceiver>(context, shared, manager, channel_id, connection_type, Priority::PRIMARY));
    result.emplace_back(std::make_unique<MDPReceiver>(context, shared, manager, channel_id, connection_type, Priority::SECONDARY));
  };
  for (auto channel_id : settings.multicast.channel_ids) {
    if (std::empty(shared.settings.misc.secdef_config_file)) {
      helper_1(channel_id, protocol::mdp::ConnectionType::INSTRUMENT_DEFINITION);
    } else {
      log::warn("The instrument definitions channel is not used when the secdef file was chosen"sv);
    }
    helper_1(channel_id, protocol::mdp::ConnectionType::MBP_MARKET_RECOVERY);
    helper_1(channel_id, protocol::mdp::ConnectionType::MBOFD_MARKET_RECOVERY);
    helper_2(channel_id, protocol::mdp::ConnectionType::INCREMENTAL);
  }
  return result;
}

template <typename R>
R create_accounts(auto const &config) {
  using result_type = std::remove_cvref_t<R>;
  result_type result;
  for (auto &[_, account] : config.accounts) {
    result.try_emplace(static_cast<std::string_view>(account.name), std::make_unique<Account>(config, account.name));
  }
  return result;
}

template <typename R>
R create_order_entry(auto &gateway, auto &context, auto &stream_id, auto &accounts, auto &shared) {
  using result_type = std::remove_cvref_t<R>;
  result_type result;
  auto &market_segment_ids = shared.settings.ilink.market_segment_ids;
  if (!std::empty(market_segment_ids)) {
    auto &firm_id = shared.settings.ilink.firm_id;
    if (std::empty(firm_id)) {
      log::fatal("Unexpected: --ilink_firm_id is required"sv);
    }
    for (auto &item : market_segment_ids) {
      auto market_segment_id = utils::charconv::from_chars<uint16_t>(item);
      if (shared.get_market_segment(market_segment_id, [&](auto &market_segment) {
            auto uri = io::web::URI::create("tcp"sv, market_segment.primary_host_ip, shared.settings.ilink.port);
            log::info("DEBUG market_segment_id={}, uri={}"sv, market_segment_id, uri);
            // XXX **not** by account
            for (auto &[name, account] : accounts) {
              auto obj = std::make_unique<OrderEntry>(gateway, context, ++stream_id, *account, shared, market_segment_id, uri);
              result.try_emplace(name, std::move(obj));
            }
          })) {
      } else {
        log::fatal("Unexpected: can't find market_segment_id={}"sv, market_segment_id);
      }
    }
  }
  return result;
}
}  // namespace

// === IMPLEMENTATION ===

std::unique_ptr<server::Handler> Controller::create(server::Dispatcher &dispatcher, Settings const &settings, Config const &config, io::Context &context) {
  return std::make_unique<Controller>(dispatcher, settings, config, context);
}

Controller::Controller(server::Dispatcher &dispatcher, Settings const &settings, Config const &config, io::Context &context)
    : dispatcher_{dispatcher}, accounts_{create_accounts<decltype(accounts_)>(config)}, context_{context},
      security_definitions_{*this, settings.misc.secdef_config_file}, shared_{dispatcher, settings, security_definitions_},
      market_data_{create_market_data_manager(dispatcher_, settings, security_definitions_, shared_, stream_id_)},
      mdp_receivers_{create_mdp_receivers<decltype(mdp_receivers_)>(settings, context_, shared_, market_data_)},
      order_entry_{create_order_entry<decltype(order_entry_)>(*this, context_, stream_id_, accounts_, shared_)} {
}

// server::Handler

void Controller::operator()(Event<Start> const &event) {
  log::info("Starting..."sv);
  dispatch(event);
}

void Controller::operator()(Event<Stop> const &event) {
  log::info("Stopping..."sv);
  dispatch(event);
}

void Controller::operator()(Event<Timer> const &event) {
  dispatch(event);
}

void Controller::operator()(Event<Control> const &event) {
  auto &[message_info, control] = event;
  switch (control.action) {
    using enum Action;
    case UNDEFINED:
      assert(false);
      break;
    case ENABLE:
      dispatcher_(State::ENABLED);
      break;
    case DISABLE:
      dispatcher_(State::DISABLED);
      break;
  }
}

void Controller::operator()(Event<Connected> const &) {
}

void Controller::operator()(Event<Disconnected> const &) {
}

void Controller::operator()(Event<Subscribe> const &) {
}

uint16_t Controller::operator()(
    Event<CreateOrder> const &event, server::oms::Order const &order, server::oms::RefData const &ref_data, std::string_view const &request_id) {
  assert(!std::empty(event.value.account));
  return get_order_entry(event.value.account)(event, order, ref_data, request_id);
}

uint16_t Controller::operator()(
    Event<ModifyOrder> const &event,
    server::oms::Order const &order,
    server::oms::RefData const &ref_data,
    std::string_view const &request_id,
    std::string_view const &previous_request_id) {
  assert(!std::empty(event.value.account));
  assert(event.value.account == order.account);
  return get_order_entry(event.value.account)(event, order, ref_data, request_id, previous_request_id);
}

uint16_t Controller::operator()(
    Event<CancelOrder> const &event,
    server::oms::Order const &order,
    server::oms::RefData const &ref_data,
    std::string_view const &request_id,
    std::string_view const &previous_request_id) {
  assert(!std::empty(event.value.account));
  assert(event.value.account == order.account);
  return get_order_entry(event.value.account)(event, order, ref_data, request_id, previous_request_id);
}

uint16_t Controller::operator()(Event<CancelAllOrders> const &event, std::string_view const &request_id) {
  assert(!std::empty(event.value.account));
  return get_order_entry(event.value.account)(event, request_id);
}

uint16_t Controller::operator()(Event<MassQuote> const &) {
  throw server::oms::NotSupported{"not supported"sv};
}

uint16_t Controller::operator()(Event<CancelQuotes> const &) {
  throw server::oms::NotSupported{"not supported"sv};
}

void Controller::operator()(metrics::Writer &writer) const {
  dispatch_helper(*this, writer);
}

// streams

void Controller::operator()(Trace<StreamStatus> const &event) {
  dispatcher_(event);
}

void Controller::operator()(Trace<ExternalLatency> const &event) {
  dispatcher_(event);
}

// utilities

template <typename... Args>
void Controller::dispatch(Args &&...args) {
  dispatch_helper(*this, std::forward<Args>(args)...);
}

template <typename... Args>
void Controller::dispatch_helper(auto &self, Args &&...args) {
  auto helper = [&](auto &target) { target(std::forward<Args>(args)...); };
  helper(self.market_data_);
  for (auto &[_, item] : self.order_entry_) {
    helper(*item);
  }
}

OrderEntry &Controller::get_order_entry(std::string_view const &account) {
  auto iter = order_entry_.find(account);
  if (iter == std::end(order_entry_)) [[unlikely]] {
    throw RuntimeError{R"(Unknown account="{}")"sv, account};
  }
  return *(*iter).second;
}

}  // namespace gateway
}  // namespace cme
}  // namespace roq
