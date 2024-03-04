/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/cme/market_data/manager.hpp"

#include "roq/logging.hpp"

using namespace std::literals;

namespace roq {
namespace cme {
namespace market_data {

// === CONSTANTS ===

namespace {
auto const BUFFER_SIZE = 2048uz;  // >=1440 MTU
auto const BUFFER_DEPTH = 10uz;
}  // namespace

// === IMPLEMENTATION ===

Manager::Manager(Handler &handler, Config const &config, uint16_t &stream_id)
    : handler_{handler}, config_{config}, shared_{*this}, channel_{"344", BUFFER_SIZE, BUFFER_DEPTH},
      instrument_definition_{*this, shared_, ++stream_id, Priority::PRIMARY},
      mbp_market_recovery_{*this, shared_, channel_, ++stream_id, Priority::PRIMARY},
      mbofd_market_recovery_{*this, shared_, channel_, ++stream_id, Priority::PRIMARY},
      incremental_{*this, shared_, channel_, ++stream_id, Priority::PRIMARY} {
}

void Manager::start() {
}

void Manager::dispatch(
    mdp::ConnectionType connection_type,
    Priority,
    std::span<std::byte const> const &payload,
    TraceInfo const &trace_info) {
  switch (connection_type) {
    using enum mdp::ConnectionType;
    case HISTORICAL_REPLAY:
      log::fatal("Unexpected"sv);
      break;
    case INSTRUMENT_DEFINITION:
      instrument_definition_.dispatch(payload, trace_info);
      break;
    case MBP_MARKET_RECOVERY:
      mbp_market_recovery_.dispatch(payload, trace_info);
      break;
    case MBOFD_MARKET_RECOVERY:
      mbofd_market_recovery_.dispatch(payload, trace_info);
      break;
    case INCREMENTAL:
      incremental_.dispatch(payload, trace_info);
      break;
  }
}

void Manager::operator()(Trace<ReferenceData> const &event, bool is_last) {
  if (event.value.discard && !config_.cache_all_reference_data)
    return;
  handler_(event, is_last);
}

void Manager::operator()(Trace<MarketByPriceUpdate> const &event, bool is_last) {
  auto &[trace_info, market_by_price_update] = event;
  auto &market_by_price = handler_.get_market_by_price(market_by_price_update.exchange, market_by_price_update.symbol);
  market_by_price(market_by_price_update, bids_, asks_, [&](auto &market_by_price_update_2) {
    Trace event_2{trace_info, market_by_price_update_2};
    handler_(event_2, is_last);
  });
}

void Manager::operator()(Trace<MarketByOrderUpdate> const &event, bool is_last) {
  auto &[trace_info, market_by_order_update] = event;
  auto &market_by_order = handler_.get_market_by_order(market_by_order_update.exchange, market_by_order_update.symbol);
  market_by_order(market_by_order_update, orders_, [&](auto &market_by_order_update_2) {
    Trace event_2{trace_info, market_by_order_update_2};
    handler_(event_2, is_last);
  });
}

}  // namespace market_data
}  // namespace cme
}  // namespace roq
