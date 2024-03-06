/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include <vector>

#include "roq/api.hpp"

#include "roq/cache/market_by_order.hpp"
#include "roq/cache/market_by_price.hpp"

namespace roq {
namespace cme {
namespace market_data {

struct Dispatcher {
  virtual bool discard_symbol(std::string_view const &symbol) = 0;

  virtual void operator()(Trace<StreamStatus> const &) = 0;
  virtual void operator()(Trace<ExternalLatency> const &) = 0;
  virtual void operator()(Trace<ReferenceData> const &, bool is_last) = 0;
  virtual void operator()(Trace<MarketStatus> const &, bool is_last) = 0;
  virtual void operator()(Trace<TopOfBook> const &, bool is_last) = 0;
  virtual void operator()(Trace<MarketByPriceUpdate> const &, bool is_last) = 0;
  virtual void operator()(Trace<MarketByOrderUpdate> const &, bool is_last) = 0;
  virtual void operator()(Trace<TradeSummary> const &, bool is_last) = 0;
  virtual void operator()(Trace<StatisticsUpdate> const &, bool is_last) = 0;

  // note! allows the user to validate the full order book before publishing
  template <typename Callback>
  inline void operator()(
      Trace<MarketByPriceUpdate> const &event,
      bool is_last,
      std::vector<MBPUpdate> &bids,
      std::vector<MBPUpdate> &asks,
      Callback callback) {
    auto &trace_info = event.trace_info;
    auto &market_by_price_update = event.value;
    auto &market_by_price = get_market_by_price(market_by_price_update.exchange, market_by_price_update.symbol);
    // apply update to cached object
    market_by_price(market_by_price_update, bids, asks, [&](auto &market_by_price_update_2) {
      // callback with cached object (only allowing validation, no modifications)
      callback(std::as_const(market_by_price));
      // dispatch final update
      Trace event_2{trace_info, market_by_price_update_2};
      (*this)(event_2, is_last);
    });
  }

  // note! allows the user to apply updates to a snapshot before publishing
  template <typename Callback>
  inline void operator()(Trace<MarketByPriceUpdate> const &event, bool is_last, Callback callback) {
    auto &[trace_info, market_by_price_update] = event;
    auto &market_by_price = get_market_by_price(market_by_price_update.exchange, market_by_price_update.symbol);
    // apply update to cached object
    market_by_price(market_by_price_update);
    // callback with cached object (allowing changes)
    callback(market_by_price);
    // dispatch snapshot
    market_by_price.create_snapshot(bids_, asks_, [&](auto &market_by_price_update_2) {
      Trace event_2{trace_info, market_by_price_update_2};
      (*this)(event_2, is_last);
    });
  }

  // note! allows the user to validate the full order book before publishing
  template <typename Callback>
  inline void operator()(
      Trace<MarketByOrderUpdate> const &event, bool is_last, std::vector<MBOUpdate> &orders, Callback callback) {
    auto &trace_info = event.trace_info;
    auto &market_by_order_update = event.value;
    auto &market_by_order = get_market_by_order(market_by_order_update.exchange, market_by_order_update.symbol);
    // apply update to cached object
    market_by_order(market_by_order_update, orders, [&](auto &market_by_order_update_2) {
      // callback with cached object (only allowing validation, no modifications)
      callback(std::as_const(market_by_order));
      // dispatch final update
      Trace event_2{trace_info, market_by_order_update_2};
      (*this)(event_2, is_last);
    });
  }

  // note! allows the user to apply updates to a snapshot before publishing
  template <typename Callback>
  inline void operator()(Trace<MarketByOrderUpdate> const &event, bool is_last, Callback callback) {
    auto &[trace_info, market_by_order_update] = event;
    auto &market_by_order = get_market_by_order(market_by_order_update.exchange, market_by_order_update.symbol);
    // apply update to cached object
    market_by_order(market_by_order_update);
    // callback with cached object (allowing changes)
    callback(market_by_order);
    // dispatch snapshot
    market_by_order.create_snapshot(orders_, [&](auto &market_by_order_update_2) {
      Trace event_2{trace_info, market_by_order_update_2};
      (*this)(event_2, is_last);
    });
  }

 protected:
  virtual cache::MarketByPrice &get_market_by_price(
      std::string_view const &exchange, std::string_view const &symbol) = 0;
  virtual cache::MarketByOrder &get_market_by_order(
      std::string_view const &exchange, std::string_view const &symbol) = 0;

 private:
  std::vector<MBPUpdate> bids_, asks_;
  std::vector<MBOUpdate> orders_;
};

}  // namespace market_data
}  // namespace cme
}  // namespace roq
