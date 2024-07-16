/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include <string>
#include <utility>
#include <vector>

#include "roq/api.hpp"

#include "roq/utils/container.hpp"

#include "roq/server/md/dispatcher.hpp"

#include "roq/cme/tools/security.hpp"

#include "roq/cme/market_data/options.hpp"
#include "roq/cme/market_data/security_definitions.hpp"

namespace roq {
namespace cme {
namespace market_data {

struct Shared final {
  Shared(server::md::Dispatcher &, Options const &, SecurityDefinitions &);

  Shared(Shared &&) = default;
  Shared(Shared const &) = delete;

 private:
  server::md::Dispatcher &dispatcher_;

 public:
  struct {
    std::vector<MBPUpdate> bids, asks;
    std::vector<MBOUpdate> orders;
  } cache;

 public:
  template <typename... Args>
  auto operator()(Args &&...args) {
    return dispatcher_(std::forward<Args>(args)...);
  }

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
  void read_secdef(std::string_view const &config_file, std::chrono::nanoseconds first_timestamp);

  Options const options;

  auto discard_symbol(std::string_view const &name) const { return dispatcher_.discard_symbol(name); }

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

  SecurityDefinitions &security_definitions;
  // buffer
  std::vector<std::byte> buffer;

  // - incremental / refresh book
  std::vector<std::tuple<int32_t, Side, double, UpdateAction>> md_entries_;
  // - incremental / trade summary
  std::vector<std::tuple<int32_t, Side, double, int32_t, size_t, uint32_t>> trade_summary_;
  std::vector<int32_t> security_ids_;
  std::vector<std::pair<uint64_t, int32_t>> orders_;
  std::chrono::nanoseconds transact_time_ = {};
  size_t total_number_of_orders_ = {};
};

}  // namespace market_data
}  // namespace cme
}  // namespace roq
