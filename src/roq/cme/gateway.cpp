/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/cme/gateway.hpp"

#include <utility>

#include "roq/core/charconv.hpp"

using namespace std::literals;

namespace roq {
namespace cme {

// === HELPERS ===

namespace {
auto create_channels(auto &settings) {
  std::vector<Channel> result;
  auto buffer_size = settings.multicast.buffer_size;
  auto buffer_depth = settings.multicast.buffer_depth;
  auto &channel_ids = settings.multicast.channel_ids;
  for (auto &channel_id : channel_ids)
    result.emplace_back(channel_id, buffer_size, buffer_depth);
  return result;
}

auto create_udp_incremental(auto &gateway, auto &context, auto &stream_id, auto &shared, auto &channels) {
  std::vector<std::unique_ptr<UDPIncremental>> result;
  for (auto &channel : channels) {
    result.emplace_back(
        std::make_unique<UDPIncremental>(gateway, context, ++stream_id, shared, channel, Priority::PRIMARY));
    result.emplace_back(
        std::make_unique<UDPIncremental>(gateway, context, ++stream_id, shared, channel, Priority::SECONDARY));
  }
  return result;
}

auto create_udp_instrument_definition(auto &gateway, auto &context, auto &stream_id, auto &shared, auto &channels) {
  std::vector<std::unique_ptr<UDPInstrumentDefinition>> result;
  if (std::empty(shared.settings.common.secdef_config_file)) {
    for (auto &channel : channels)
      result.emplace_back(std::make_unique<UDPInstrumentDefinition>(gateway, context, ++stream_id, shared, channel));
  } else {
    log::warn("The instrument definitions channel is not used when the secdef file was chosen"sv);
  }
  return result;
}

auto create_udp_mbp_market_recovery(auto &gateway, auto &context, auto &stream_id, auto &shared, auto &channels) {
  std::vector<std::unique_ptr<UDPMBPMarketRecovery>> result;
  for (auto &channel : channels)
    result.emplace_back(std::make_unique<UDPMBPMarketRecovery>(gateway, context, ++stream_id, shared, channel));
  return result;
}

auto create_udp_mbo_market_recovery(auto &gateway, auto &context, auto &stream_id, auto &shared, auto &channels) {
  std::vector<std::unique_ptr<UDPMBOMarketRecovery>> result;
  if (shared.settings.common.enable_market_by_order) {
    for (auto &channel : channels)
      result.emplace_back(std::make_unique<UDPMBOMarketRecovery>(gateway, context, ++stream_id, shared, channel));
  }
  return result;
}

template <typename R>
R create_accounts(auto const &config) {
  using result_type = std::remove_cvref<R>::type;
  result_type result;
  for (auto &[_, account] : config.accounts)
    result.try_emplace(account.name, std::make_unique<Account>(config, account.name));
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
      auto market_segment_id = core::charconv::from_string<uint8_t>(item);
      if (shared.get_market_segment(market_segment_id, [&](auto &market_segment) {
            auto address = fmt::format("tcp://{}:{}"sv, market_segment.primary_host_ip, shared.settings.ilink.port);
            io::web::URI uri{address};
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
      shared_{dispatcher, settings}, channels_{create_channels(settings)},
      udp_incremental_{create_udp_incremental(*this, context_, stream_id_, shared_, channels_)},
      udp_instrument_definition_{create_udp_instrument_definition(*this, context_, stream_id_, shared_, channels_)},
      udp_mbp_market_recovery_{create_udp_mbp_market_recovery(*this, context_, stream_id_, shared_, channels_)},
      udp_mbo_market_recovery_{create_udp_mbo_market_recovery(*this, context_, stream_id_, shared_, channels_)},
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

void Gateway::operator()(Event<Disconnected> const &event) {
  auto const &[message_info, disconnected] = event;
  log::warn(
      R"(Disconnected: source="{}", order_cancel_policy={})"sv,
      message_info.source_name,
      disconnected.order_cancel_policy);
}

uint16_t Gateway::operator()(
    Event<CreateOrder> const &event, oms::Order const &, [[maybe_unused]] std::string_view const &request_id) {
  assert(!std::empty(event.value.account));
  throw oms::NotSupported{"not supported"sv};
}

uint16_t Gateway::operator()(
    Event<ModifyOrder> const &event,
    oms::Order const &order,
    [[maybe_unused]] std::string_view const &request_id,
    [[maybe_unused]] std::string_view const &previous_request_id) {
  assert(!std::empty(event.value.account));
  assert(event.value.account == order.account);
  throw oms::NotSupported{"not supported"sv};
}

uint16_t Gateway::operator()(
    Event<CancelOrder> const &event,
    oms::Order const &order,
    [[maybe_unused]] std::string_view const &request_id,
    [[maybe_unused]] std::string_view const &previous_request_id) {
  assert(!std::empty(event.value.account));
  assert(event.value.account == order.account);
  throw oms::NotSupported{"not supported"sv};
}

uint16_t Gateway::operator()(Event<CancelAllOrders> const &event, [[maybe_unused]] std::string_view const &request_id) {
  assert(!std::empty(event.value.account));
  throw oms::NotSupported{"not supported"sv};
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

void Gateway::operator()(Trace<ReferenceData> const &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Gateway::operator()(Trace<MarketStatus> const &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Gateway::operator()(Trace<TopOfBook> const &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Gateway::operator()(Trace<MarketByPriceUpdate> const &event, bool is_last) {
  auto callback = []([[maybe_unused]] auto &market_by_price) {};
  dispatcher_(event, is_last, bids_, asks_, callback);
}

void Gateway::operator()(Trace<MarketByOrderUpdate> const &event, bool is_last) {
  auto callback = []([[maybe_unused]] auto &market_by_price) {};
  dispatcher_(event, is_last, orders_, callback);
}

void Gateway::operator()(Trace<TradeSummary> const &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Gateway::operator()(Trace<StatisticsUpdate> const &event, bool is_last) {
  dispatcher_(event, is_last);
}

template <typename... Args>
void Gateway::dispatch(Args &&...args) {
  auto helper = [&](auto &target) { target(std::forward<Args>(args)...); };
  for (auto &item : udp_incremental_)
    helper(*item);
  for (auto &item : udp_instrument_definition_)
    helper(*item);
  for (auto &item : udp_mbp_market_recovery_)
    helper(*item);
  for (auto &item : udp_mbo_market_recovery_)
    helper(*item);
  for (auto &[_, item] : order_entry_)
    helper(*item);
}

}  // namespace cme
}  // namespace roq
