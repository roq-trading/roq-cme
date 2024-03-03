/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/cme/market_data/manager.hpp"

#include "roq/logging.hpp"

using namespace std::literals;

namespace roq {
namespace cme {
namespace market_data {

// === IMPLEMENTATION ===

Manager::Manager(Handler &handler)
    : handler_{handler}, shared_{*this}, channel_{"344", 1024 * 1024, 10}, instrument_definition_{*this, shared_},
      market_by_price_recovery_{*this, shared_, channel_}, market_by_order_recovery_{*this, shared_, channel_},
      incremental_{*this, shared_} {
}

void Manager::dispatch(
    mdp::ConnectionType connection_type,
    std::span<std::byte const> const &payload,
    TraceInfo const &trace_info,
    uint16_t stream_id) {
  switch (connection_type) {
    using enum mdp::ConnectionType;
    case HISTORICAL_REPLAY:
      log::fatal("Unexpected"sv);
      break;
    case INSTRUMENT_REPLAY:
      instrument_definition_.dispatch(payload, trace_info, stream_id);
      break;
    case SNAPSHOT:
      market_by_price_recovery_.dispatch(payload, trace_info, stream_id);
      break;
    case SNAPSHOT_MBO:
      market_by_order_recovery_.dispatch(payload, trace_info, stream_id);
      break;
    case INCREMENTAL:
      incremental_.dispatch(payload, trace_info, stream_id);
      break;
  }
}

}  // namespace market_data
}  // namespace cme
}  // namespace roq
