/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/cme/gateway.hpp"

#include <utility>

#include "roq/cme/flags/common.hpp"
#include "roq/cme/flags/multicast.hpp"

using namespace std::literals;

namespace roq {
namespace cme {

// === HELPERS ===

namespace {
auto create_channels() {
  std::vector<Channel> result;
  auto &channel_ids = flags::Multicast::multicast_channel_ids();
  for (auto &channel_id : channel_ids)
    result.emplace_back(
        channel_id, flags::Multicast::multicast_buffer_size(), flags::Multicast::multicast_buffer_depth());
  return result;
}

auto create_udp_instrument_definition(auto &gateway, auto &context, auto &stream_id, auto &shared, auto &channels) {
  std::vector<std::unique_ptr<UDPInstrumentDefinition>> result;
  if (std::empty(flags::Common::secdef_config_file())) {
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
}  // namespace

// === IMPLEMENTATION ===

Gateway::Gateway(server::Dispatcher &dispatcher, Config const &, io::Context &context)
    : dispatcher_{dispatcher}, context_{context}, shared_{dispatcher_}, channels_{create_channels()},
      udp_instrument_definition_{create_udp_instrument_definition(*this, context_, stream_id_, shared_, channels_)},
      udp_mbp_market_recovery_{create_udp_mbp_market_recovery(*this, context_, stream_id_, shared_, channels_)},
      udp_incremental_{create_udp_incremental(*this, context_, stream_id_, shared_, channels_)} {
}

void Gateway::operator()(Event<Start> const &event) {
  log::info("Starting..."sv);
  for (auto &item : udp_instrument_definition_)
    (*item)(event);
  for (auto &item : udp_mbp_market_recovery_)
    (*item)(event);
  for (auto &item : udp_incremental_)
    (*item)(event);
}

void Gateway::operator()(Event<Stop> const &event) {
  log::info("Stopping..."sv);
  for (auto &item : udp_incremental_)
    (*item)(event);
  for (auto &item : udp_mbp_market_recovery_)
    (*item)(event);
  for (auto &item : udp_instrument_definition_)
    (*item)(event);
}

void Gateway::operator()(Event<Timer> const &event) {
  for (auto &item : udp_instrument_definition_)
    (*item)(event);
  for (auto &item : udp_mbp_market_recovery_)
    (*item)(event);
  for (auto &item : udp_incremental_)
    (*item)(event);
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
  for (auto &item : udp_instrument_definition_)
    (*item)(writer);
  for (auto &item : udp_mbp_market_recovery_)
    (*item)(writer);
  for (auto &item : udp_incremental_)
    (*item)(writer);
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

void Gateway::operator()(Trace<MarketByPriceUpdate> const &event, bool is_last, bool refresh) {
  dispatcher_(
      event, is_last, refresh, shared_.final_bids, shared_.final_asks, []([[maybe_unused]] auto &market_by_price) {});
}

void Gateway::operator()(Trace<TradeSummary> const &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Gateway::operator()(Trace<StatisticsUpdate> const &event, bool is_last) {
  dispatcher_(event, is_last);
}

}  // namespace cme
}  // namespace roq
