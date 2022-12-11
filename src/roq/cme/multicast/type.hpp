/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

namespace roq {
namespace cme {
namespace multicast {

enum class Type {
  HISTORICAL_REPLAY,
  INSTRUMENT_REPLAY,
  SNAPSHOT,
  SNAPSHOT_MBO,
  INCREMENTAL,
};

}  // namespace multicast
}  // namespace cme
}  // namespace roq
