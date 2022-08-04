/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/cme/gateway.hpp"

#include <utility>

#include "roq/io/engine/context_factory.hpp"

#include "roq/cme/flags/common.hpp"
#include "roq/cme/flags/fix.hpp"
#include "roq/cme/flags/multicast.hpp"

using namespace std::literals;

namespace roq {
namespace cme {

namespace {
auto create_udp_events(Gateway &gateway, io::Context &context, uint16_t &stream_id, Shared &shared) {
  if (shared.has_multicast())
    return std::make_unique<UDPEvents>(gateway, context, stream_id, shared);
  return std::unique_ptr<UDPEvents>{};
}
}  // namespace

Gateway::Gateway(server::Dispatcher &dispatcher, Config const &config)
    : dispatcher_(dispatcher), context_(io::engine::ContextFactory::create_libevent()), shared_(dispatcher_),
      udp_events_(create_udp_events(*this, *context_, ++stream_id_, shared_)) {
  if (!flags::FIX::fix_cancel_on_disconnect())
    log::warn("Orders will *NOT* be cancelled on disconnect"sv);
}

void Gateway::operator()(Event<Start> const &event) {
  log::info("Starting the gateway..."sv);
  if (udp_events_)
    (*udp_events_)(event);
}

void Gateway::operator()(Event<Stop> const &event) {
  log::info("Stopping the gateway..."sv);
  if (udp_events_)
    (*udp_events_)(event);
}

void Gateway::operator()(Event<Timer> const &event) {
  if (udp_events_)
    (*udp_events_)(event);
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
    Event<CreateOrder> const &event, oms::Order const &order, std::string_view const &request_id) {
  assert(!std::empty(event.value.account));
  throw oms::NotSupported("not supported"sv);
}

uint16_t Gateway::operator()(
    Event<ModifyOrder> const &event,
    oms::Order const &order,
    std::string_view const &request_id,
    std::string_view const &previous_request_id) {
  assert(!std::empty(event.value.account));
  assert(event.value.account == order.account);
  throw oms::NotSupported("not supported"sv);
}

uint16_t Gateway::operator()(
    Event<CancelOrder> const &event,
    oms::Order const &order,
    std::string_view const &request_id,
    std::string_view const &previous_request_id) {
  assert(!std::empty(event.value.account));
  assert(event.value.account == order.account);
  throw oms::NotSupported("not supported"sv);
}

uint16_t Gateway::operator()(Event<CancelAllOrders> const &event, std::string_view const &request_id) {
  assert(!std::empty(event.value.account));
  throw oms::NotSupported("not supported"sv);
}

void Gateway::operator()(metrics::Writer &writer) {
}

void Gateway::operator()(Trace<StreamStatus const> const &event) {
  dispatcher_(event);
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

}  // namespace cme
}  // namespace roq
