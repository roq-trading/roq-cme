/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/cme/gateway.hpp"

#include <utility>

#include "roq/utils/charconv.hpp"

using namespace std::literals;

namespace roq {
namespace cme {

// === HELPERS ===

namespace {
auto create_market_data_manager(auto &dispatcher, auto &settings, auto &shared, auto &stream_id) {
  auto options = market_data::Options{
      .cache_all_reference_data = settings.filter.all_reference_data,
      .enable_market_by_order = settings.common.enable_market_by_order,
      .mbp_to_mbo_clear_price_level = settings.test.mbp_to_mbo_clear_price_level,
      .filter_snapshot_from_incremental = settings.common.filter_snapshot_from_incremental,
      .local_interface = settings.multicast.local_interface,
      .multicast_timeout = settings.multicast.timeout,
  };
  return market_data::Manager{dispatcher, options, settings.multicast.channel_ids, shared.mdp_config_, stream_id};
}

template <typename R>
auto create_mdp_receivers(auto &settings, auto &context, auto &shared, auto &manager) {
  using result_type = std::remove_cvref<R>::type;
  result_type result;
  auto helper_1 = [&](auto channel_id, auto connection_type) {
    result.emplace_back(
        std::make_unique<MDPReceiver>(context, shared, manager, channel_id, connection_type, Priority::PRIMARY));
  };
  auto helper_2 = [&](auto channel_id, auto connection_type) {
    result.emplace_back(
        std::make_unique<MDPReceiver>(context, shared, manager, channel_id, connection_type, Priority::PRIMARY));
    result.emplace_back(
        std::make_unique<MDPReceiver>(context, shared, manager, channel_id, connection_type, Priority::SECONDARY));
  };
  for (auto channel_id : settings.multicast.channel_ids) {
    if (std::empty(shared.settings.common.secdef_config_file)) {
      helper_1(channel_id, mdp::ConnectionType::INSTRUMENT_DEFINITION);
    } else {
      log::warn("The instrument definitions channel is not used when the secdef file was chosen"sv);
    }
    helper_1(channel_id, mdp::ConnectionType::MBP_MARKET_RECOVERY);
    helper_1(channel_id, mdp::ConnectionType::MBOFD_MARKET_RECOVERY);
    helper_2(channel_id, mdp::ConnectionType::INCREMENTAL);
  }
  return result;
}

template <typename R>
R create_accounts(auto const &config) {
  using result_type = std::remove_cvref<R>::type;
  result_type result;
  for (auto &[_, account] : config.accounts)
    result.try_emplace(static_cast<std::string_view>(account.name), std::make_unique<Account>(config, account.name));
  return result;
}

template <typename R>
R create_order_entry(auto &gateway, auto &context, auto &stream_id, auto &accounts, auto &shared) {
  using result_type = std::remove_cvref<R>::type;
  result_type result;
  auto &market_segment_ids = shared.settings.ilink.market_segment_ids;
  if (!std::empty(market_segment_ids)) {
    auto &firm_id = shared.settings.ilink.firm_id;
    if (std::empty(firm_id))
      log::fatal("Unexpected: --ilink_firm_id is required"sv);
    for (auto &item : market_segment_ids) {
      auto market_segment_id = utils::from_chars<uint16_t>(item);
      if (shared.get_market_segment(market_segment_id, [&](auto &market_segment) {
            auto uri = io::web::URI::create("tcp"sv, market_segment.primary_host_ip, shared.settings.ilink.port);
            log::info("DEBUG market_segment_id={}, uri={}"sv, market_segment_id, uri);
            // XXX **not** by account
            for (auto &[name, account] : accounts)
              result.try_emplace(
                  name,
                  std::make_unique<OrderEntry>(
                      gateway, context, ++stream_id, *account, shared, market_segment_id, uri));
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

Gateway::Gateway(server::Dispatcher &dispatcher, Settings const &settings, Config const &config, io::Context &context)
    : dispatcher_{dispatcher}, accounts_{create_accounts<decltype(accounts_)>(config)}, context_{context},
      shared_{dispatcher, settings}, manager_{create_market_data_manager(dispatcher_, settings, shared_, stream_id_)},
      mdp_receivers_{create_mdp_receivers<decltype(mdp_receivers_)>(settings, context_, shared_, manager_)},
      order_entry_{create_order_entry<decltype(order_entry_)>(*this, context_, stream_id_, accounts_, shared_)} {
}

void Gateway::operator()(Event<Start> const &event) {
  log::info("Starting..."sv);
  dispatch(event);
}

void Gateway::operator()(Event<Stop> const &event) {
  log::info("Stopping..."sv);
  dispatch(event);
}

void Gateway::operator()(Event<Timer> const &event) {
  dispatch(event);
}

void Gateway::operator()(Event<Connected> const &) {
}

void Gateway::operator()(Event<Disconnected> const &) {
}

uint16_t Gateway::operator()(
    Event<CreateOrder> const &event, server::oms::Order const &order, std::string_view const &request_id) {
  assert(!std::empty(event.value.account));
  return get_order_entry(event.value.account)(event, order, request_id);
}

uint16_t Gateway::operator()(
    Event<ModifyOrder> const &event,
    server::oms::Order const &order,
    std::string_view const &request_id,
    std::string_view const &previous_request_id) {
  assert(!std::empty(event.value.account));
  assert(event.value.account == order.account);
  return get_order_entry(event.value.account)(event, order, request_id, previous_request_id);
}

uint16_t Gateway::operator()(
    Event<CancelOrder> const &event,
    server::oms::Order const &order,
    std::string_view const &request_id,
    std::string_view const &previous_request_id) {
  assert(!std::empty(event.value.account));
  assert(event.value.account == order.account);
  return get_order_entry(event.value.account)(event, order, request_id, previous_request_id);
}

uint16_t Gateway::operator()(Event<CancelAllOrders> const &event, std::string_view const &request_id) {
  assert(!std::empty(event.value.account));
  return get_order_entry(event.value.account)(event, request_id);
}

void Gateway::operator()(metrics::Writer &writer) {
  dispatch(writer);
}

void Gateway::operator()(Trace<StreamStatus> const &event) {
  dispatcher_(event);
}

void Gateway::operator()(Trace<ExternalLatency> const &event) {
  dispatcher_(event);
}

template <typename... Args>
void Gateway::dispatch(Args &&...args) {
  auto helper = [&](auto &target) { target(std::forward<Args>(args)...); };
  for (auto &[_, item] : order_entry_)
    helper(*item);
}

OrderEntry &Gateway::get_order_entry(std::string_view const &account) {
  auto iter = order_entry_.find(account);
  if (iter == std::end(order_entry_)) [[unlikely]]
    throw RuntimeError{R"(Unknown account="{}")"sv, account};
  return *(*iter).second;
}

}  // namespace cme
}  // namespace roq
