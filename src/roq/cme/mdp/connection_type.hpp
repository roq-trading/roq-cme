/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

namespace roq {
namespace cme {
namespace mdp {

enum class ConnectionType {
  HISTORICAL_REPLAY,
  INSTRUMENT_REPLAY,
  SNAPSHOT,
  SNAPSHOT_MBO,
  INCREMENTAL,
};

}  // namespace mdp
}  // namespace cme
}  // namespace roq
