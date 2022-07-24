/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/core/json/buffer.hpp"
#include "roq/core/json/parser.hpp"

#include "roq/server.hpp"

// public
#include "roq/deribit/json/ticker.hpp"

// private
#include "roq/deribit/json/changes.hpp"
#include "roq/deribit/json/instrument_state.hpp"
#include "roq/deribit/json/order.hpp"
#include "roq/deribit/json/platform_state.hpp"
#include "roq/deribit/json/portfolio.hpp"
#include "roq/deribit/json/quote.hpp"
#include "roq/deribit/json/trades_2.hpp"

namespace roq {
namespace deribit {
namespace json {

struct Parser final {
  struct Handler {
    // public
    virtual void operator()(Trace<PlatformState const> const &) = 0;
    virtual void operator()(Trace<InstrumentState const> const &) = 0;
    virtual void operator()(Trace<Quote const> const &) = 0;
    virtual void operator()(Trace<Ticker const> const &) = 0;
    // private
    virtual void operator()(Trace<Portfolio const> const &) = 0;
    virtual void operator()(Trace<Changes const> const &) = 0;
    virtual void operator()(Trace<Order const> const &) = 0;
    virtual void operator()(Trace<Trades2 const> const &) = 0;
  };

  static void dispatch(Handler &, core::json::Value &, core::json::Buffer &, TraceInfo const &);
};

}  // namespace json
}  // namespace deribit
}  // namespace roq
