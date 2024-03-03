/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

namespace roq {
namespace cme {
namespace mdp {

enum class ConnectionType {
  HISTORICAL_REPLAY,
  INSTRUMENT_DEFINITION,
  MBP_MARKET_RECOVERY,
  MBOFD_MARKET_RECOVERY,
  INCREMENTAL,
};

}  // namespace mdp
}  // namespace cme
}  // namespace roq
