/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <absl/container/node_hash_map.h>

#include <string>
#include <utility>
#include <vector>

#include "roq/api.hpp"
#include "roq/server.hpp"

#include "roq/cme/settings.hpp"

#include "roq/cme/tools/security.hpp"

#include "roq/cme/mdp/config.hpp"

#include "roq/cme/ilink/config_reader.hpp"

namespace roq {
namespace cme {

struct Shared final {
  server::Dispatcher &dispatcher_;
  Settings const &settings;

  mdp::Config mdp_config_;

 private:
  absl::flat_hash_map<uint8_t, ilink::ConfigReader::MarketSegment> ilink_config_;

 public:
  template <typename Callback>
  bool get_market_segment(uint8_t market_segment_id, Callback callback) {
    auto iter = ilink_config_.find(market_segment_id);
    if (iter == std::end(ilink_config_))
      return false;
    callback((*iter).second);
    return true;
  }

  absl::node_hash_map<int32_t, tools::Security> securities;
  absl::flat_hash_map<std::string, absl::flat_hash_set<int32_t>> security_groups;
  absl::flat_hash_map<uint8_t, absl::flat_hash_map<std::string, int32_t>> market_segments;

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

 public:
  Shared(server::Dispatcher &, Settings const &);

  Shared(Shared &&) = default;
  Shared(Shared const &) = delete;

  std::pair<std::string, uint16_t> get_multicast_config(
      std::string_view const &channel_id, mdp::ConnectionType, Priority) const;

  auto discard_symbol(std::string_view const &name) const { return dispatcher_.discard_symbol(name); }

  template <typename... Args>
  auto update_order(Args &&...args) {
    return dispatcher_.update_order(std::forward<Args>(args)...);
  }

  template <typename... Args>
  auto operator()(Args &&...args) {
    return dispatcher_(std::forward<Args>(args)...);
  }

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
      market_segments[market_segment_id].try_emplace(security.symbol, security_id);
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

  // buffer
  std::vector<std::byte> buffer;
};

}  // namespace cme
}  // namespace roq
