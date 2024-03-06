/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include <string>
#include <utility>
#include <vector>

#include "roq/api.hpp"

#include "roq/utils/container.hpp"

#include "roq/cache/market_by_order.hpp"
#include "roq/cache/market_by_price.hpp"

#include "roq/cme/tools/security.hpp"

#include "roq/cme/market_data/dispatcher.hpp"
#include "roq/cme/market_data/options.hpp"

namespace roq {
namespace cme {
namespace market_data {

struct Shared final {
 private:
  Dispatcher &dispatcher_;

 public:
  template <typename... Args>
  auto operator()(Args &&...args) {
    return dispatcher_(std::forward<Args>(args)...);
  }

  utils::unordered_map<int32_t, tools::Security> securities;
  utils::unordered_map<std::string, utils::unordered_set<int32_t>> security_groups;
  utils::unordered_map<uint8_t, utils::unordered_map<std::string, int32_t>> market_segments;

 private:
  struct {
    std::vector<MBPUpdate> bids, asks;
    auto &clear() {
      bids.clear();
      asks.clear();
      return *this;
    }
    bool empty() const { return std::empty(bids) && std::empty(asks); }
  } mbp;
  struct {
    std::vector<MBOUpdate> orders;
    auto &clear() {
      orders.clear();
      return *this;
    }
    bool empty() const { return std::empty(orders); }
  } mbo;
  std::vector<Trade> trades;
  std::vector<Statistics> statistics;
  std::vector<Fill> fills;

 public:
  Shared(Dispatcher &, Options const &);

  Shared(Shared &&) = default;
  Shared(Shared const &) = delete;

  Options const options;

  auto discard_symbol(std::string_view const &name) const { return dispatcher_.discard_symbol(name); }

  // security

  bool has_security(int32_t security_id) { return securities.find(security_id) != std::end(securities); }

  template <typename Callback>
  void create_security(
      std::string_view const &security_group,
      uint8_t market_segment_id,
      int32_t security_id,
      tools::Security &&security,
      Callback callback) {
    if (!security.discard) {
      security_groups[security_group].insert(security_id);
      market_segments[market_segment_id].try_emplace(static_cast<std::string_view>(security.symbol), security_id);
    }
    auto iter = securities.try_emplace(security_id, std::move(security)).first;
    callback((*iter).second);
  }

  template <typename Callback>
  bool get_security(int32_t security_id, Callback callback) {
    auto iter = securities.find(security_id);
    if (iter == std::end(securities))
      return false;
    auto &security = (*iter).second;
    if (security.discard)
      return false;
    callback(security);
    return true;
  }

  template <typename Callback>
  bool get_security_incl_discard(int32_t security_id, Callback callback) {
    auto iter = securities.find(security_id);
    if (iter == std::end(securities))
      return false;
    auto &security = (*iter).second;
    callback(security);
    return true;
  }

  template <typename Callback>
  void get_securities(Callback callback) {
    for (auto &[security_id, security] : securities)
      if (!security.discard)
        callback(security);
  }

  // security group

  template <typename Callback>
  bool get_security_group(std::string_view const &security_group, Callback callback) {
    auto iter = security_groups.find(security_group);
    if (iter == std::end(security_groups))
      return false;
    auto &security_ids = (*iter).second;
    for (auto &security_id : security_ids)
      callback(security_id);
    return true;
  }

  // symbol

  template <typename Callback>
  bool find_security_id(uint8_t market_segment_id, std::string_view const &symbol, Callback callback) {
    auto iter_1 = market_segments.find(market_segment_id);
    if (iter_1 == std::end(market_segments))
      return false;
    auto &symbols = (*iter_1).second;
    auto iter_2 = symbols.find(symbol);
    if (iter_2 == std::end(symbols))
      return false;
    callback((*iter_2).second);
    return true;
  }

  // cache

  auto &get_mbp() { return mbp.clear(); }

  auto &get_mbo() { return mbo.clear(); }

  auto &get_trades() {
    trades.clear();
    return trades;
  }

  auto &get_statistics() {
    statistics.clear();
    return statistics;
  }

  auto &get_fills() {
    fills.clear();
    return fills;
  }

  // buffer
  std::vector<std::byte> buffer;
};

}  // namespace market_data
}  // namespace cme
}  // namespace roq
