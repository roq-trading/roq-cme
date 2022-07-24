/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <cstdint>
#include <span>

// events
#include <deribit_multicast/Book.h>
#include <deribit_multicast/Instrument.h>
#include <deribit_multicast/Ticker.h>
#include <deribit_multicast/Trades.h>
// snapshot
#include <deribit_multicast/Snapshot.h>

#include "roq/trace.hpp"

#include "roq/deribit/sbe/frame.hpp"

namespace roq {
namespace deribit {
namespace sbe {

struct Parser final {
  struct Handler {
    // events
    virtual void operator()(Trace<deribit_multicast::Instrument> const &, Frame const &) = 0;
    virtual void operator()(Trace<deribit_multicast::Book> const &, Frame const &) = 0;
    virtual void operator()(Trace<deribit_multicast::Ticker> const &, Frame const &) = 0;
    virtual void operator()(Trace<deribit_multicast::Trades> const &, Frame const &) = 0;
    // snapshot
    virtual void operator()(Trace<deribit_multicast::Snapshot> const &, Frame const &) = 0;
  };

  static bool dispatch(Handler &, std::span<std::byte const> const &buffer, TraceInfo const &);
};

}  // namespace sbe
}  // namespace deribit
}  // namespace roq
