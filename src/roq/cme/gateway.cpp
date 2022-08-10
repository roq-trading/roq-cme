/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/cme/gateway.hpp"

#include <utility>

#include "roq/io/engine/context_factory.hpp"

#include "roq/cme/flags/common.hpp"
#include "roq/cme/flags/multicast.hpp"

using namespace std::literals;

namespace roq {
namespace cme {

namespace {
auto create_udp_instrument_definition(Gateway &gateway, io::Context &context, uint16_t &stream_id, Shared &shared) {
  if (std::empty(flags::Common::secdef_config_file()))
    return std::make_unique<UDPInstrumentDefinition>(
        gateway, context, stream_id, shared, flags::Multicast::multicast_channel_id());
  log::warn("The instrument definitions channel is not needed when the secdef file was chosen"sv);
  return std::unique_ptr<UDPInstrumentDefinition>{};
}

auto create_udp_mbp_market_recovery(Gateway &gateway, io::Context &context, uint16_t &stream_id, Shared &shared) {
  return std::make_unique<UDPMBPMarketRecovery>(
      gateway, context, stream_id, shared, flags::Multicast::multicast_channel_id());
}

auto create_udp_incremental(Gateway &gateway, io::Context &context, uint16_t &stream_id, Shared &shared) {
  return std::make_unique<UDPIncremental>(
      gateway, context, stream_id, shared, flags::Multicast::multicast_channel_id());
}
}  // namespace

Gateway::Gateway(server::Dispatcher &dispatcher, Config const &)
    : dispatcher_(dispatcher), context_(io::engine::ContextFactory::create_libevent()), shared_(dispatcher_),
      udp_instrument_definition_(create_udp_instrument_definition(*this, *context_, ++stream_id_, shared_)),
      udp_mbp_market_recovery_(create_udp_mbp_market_recovery(*this, *context_, ++stream_id_, shared_)),
      udp_incremental_(create_udp_incremental(*this, *context_, ++stream_id_, shared_)) {
}

void Gateway::operator()(Event<Start> const &event) {
  log::info("Starting the gateway..."sv);
  if (udp_instrument_definition_)
    (*udp_instrument_definition_)(event);
  if (udp_mbp_market_recovery_)
    (*udp_mbp_market_recovery_)(event);
  if (udp_incremental_)
    (*udp_incremental_)(event);
}

void Gateway::operator()(Event<Stop> const &event) {
  log::info("Stopping the gateway..."sv);
  if (udp_incremental_)
    (*udp_incremental_)(event);
  if (udp_mbp_market_recovery_)
    (*udp_mbp_market_recovery_)(event);
  if (udp_instrument_definition_)
    (*udp_instrument_definition_)(event);
}

void Gateway::operator()(Event<Timer> const &event) {
  if (udp_instrument_definition_)
    (*udp_instrument_definition_)(event);
  if (udp_mbp_market_recovery_)
    (*udp_mbp_market_recovery_)(event);
  if (udp_incremental_)
    (*udp_incremental_)(event);
  (*context_).drain();
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
  throw oms::NotSupported("not supported"sv);
}

uint16_t Gateway::operator()(
    Event<ModifyOrder> const &event,
    oms::Order const &order,
    [[maybe_unused]] std::string_view const &request_id,
    [[maybe_unused]] std::string_view const &previous_request_id) {
  assert(!std::empty(event.value.account));
  assert(event.value.account == order.account);
  throw oms::NotSupported("not supported"sv);
}

uint16_t Gateway::operator()(
    Event<CancelOrder> const &event,
    oms::Order const &order,
    [[maybe_unused]] std::string_view const &request_id,
    [[maybe_unused]] std::string_view const &previous_request_id) {
  assert(!std::empty(event.value.account));
  assert(event.value.account == order.account);
  throw oms::NotSupported("not supported"sv);
}

uint16_t Gateway::operator()(Event<CancelAllOrders> const &event, [[maybe_unused]] std::string_view const &request_id) {
  assert(!std::empty(event.value.account));
  throw oms::NotSupported("not supported"sv);
}

void Gateway::operator()(metrics::Writer &writer) {
  (*udp_instrument_definition_)(writer);
  (*udp_mbp_market_recovery_)(writer);
  (*udp_incremental_)(writer);
}

void Gateway::operator()(Trace<StreamStatus const> const &event) {
  dispatcher_(event);
}

void Gateway::operator()(Trace<ReferenceData const> const &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Gateway::operator()(Trace<MarketStatus const> const &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Gateway::operator()(Trace<TopOfBook const> const &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Gateway::operator()(Trace<MarketByPriceUpdate const> const &event, bool is_last, bool refresh) {
  dispatcher_(
      event, is_last, refresh, shared_.final_bids, shared_.final_asks, []([[maybe_unused]] auto &market_by_price) {});
}

void Gateway::operator()(Trace<TradeSummary const> const &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Gateway::operator()(Trace<StatisticsUpdate const> const &event, bool is_last) {
  dispatcher_(event, is_last);
}

}  // namespace cme
}  // namespace roq
