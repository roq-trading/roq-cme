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

#include "roq/core/symbols.hpp"

#include "roq/core/limit/rate_limiter.hpp"

#include "roq/core/stack/buffer.hpp"

#include "roq/cme/multicast/config.hpp"

namespace roq {
namespace cme {

struct Shared final {
  explicit Shared(server::Dispatcher &);

  Shared(Shared &&) = default;
  Shared(Shared const &) = delete;

  std::pair<std::string, uint16_t> get_multicast_config(
      std::string_view const &channel_id, multicast::Type, Priority) const;

  std::string_view next_request_id();

  auto discard_symbol(std::string_view const &name) const { return dispatcher_.discard_symbol(name); }

  template <typename... Args>
  auto update_order(Args &&...args) {
    return dispatcher_.update_order(std::forward<Args>(args)...);
  }

  template <typename... Args>
  auto operator()(Args &&...args) {
    return dispatcher_(std::forward<Args>(args)...);
  }

 private:
  multicast::Config multicast_config_;

 public:
  std::vector<Fill> fills;
  std::vector<MBPUpdate> bids, asks;
  std::vector<Trade> trades;
  std::vector<Statistics> statistics;

  absl::flat_hash_map<Symbol, double> multiplier;

 private:
  server::Dispatcher &dispatcher_;
  uint32_t request_id_ = 0;
  core::stack::Buffer<char, 32> stack_buffer_;

 public:
  core::limit::RateLimiter rate_limiter;
  absl::flat_hash_set<std::string> all_currencies;
  absl::flat_hash_set<Symbol> all_symbols;
  core::Symbols symbols;

  // EXPERIMENTAL
  struct Security final {
    Exchange exchange;
    Symbol symbol;
    double display_factor = NaN;
    bool discard = {};
  };
  absl::node_hash_map<int32_t, Security> securities;
};

}  // namespace cme
}  // namespace roq
