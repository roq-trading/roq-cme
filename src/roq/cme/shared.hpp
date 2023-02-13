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

#include "roq/cme/security.hpp"

#include "roq/cme/multicast/config.hpp"

namespace roq {
namespace cme {

struct Shared final {
  server::Dispatcher &dispatcher_;
  multicast::Config multicast_config_;

  absl::node_hash_map<int32_t, Security> securities;
  absl::flat_hash_map<std::string, absl::flat_hash_set<int32_t>> security_groups;

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
    std::vector<MBOUpdate> bids, asks;
    auto &clear() {
      bids.clear();
      asks.clear();
      return *this;
    }
    bool empty() const { return std::empty(bids) && std::empty(asks); }
  } mbo;
  std::vector<Trade> trades;
  std::vector<Statistics> statistics;

 public:
  explicit Shared(server::Dispatcher &);

  Shared(Shared &&) = default;
  Shared(Shared const &) = delete;

  std::pair<std::string, uint16_t> get_multicast_config(
      std::string_view const &channel_id, multicast::Type, Priority) const;

  auto discard_symbol(std::string_view const &name) const { return dispatcher_.discard_symbol(name); }

  template <typename... Args>
  auto operator()(Args &&...args) {
    return dispatcher_(std::forward<Args>(args)...);
  }

  // security

  bool has_security(int32_t security_id) { return securities.find(security_id) != std::end(securities); }

  template <typename Callback>
  void create_security(
      std::string_view const &security_group, int32_t security_id, Security &&security, Callback callback) {
    if (!security.discard)
      security_groups[security_group].insert(security_id);
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
