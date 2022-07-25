/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <absl/container/node_hash_map.h>

#include <string>
#include <utility>

#include "roq/api.hpp"
#include "roq/server.hpp"

#include "roq/core/memory.hpp"
#include "roq/core/symbols.hpp"

#include "roq/core/limit/rate_limiter.hpp"

#include "roq/core/stack/buffer.hpp"

#include "roq/core/market/mbp_sequencer.hpp"

namespace roq {
namespace cme {

struct Shared final {
  explicit Shared(server::Dispatcher &);

  Shared(Shared &&) = default;
  Shared(Shared const &) = delete;

  bool has_multicast() const { return multicast_; }

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

 public:
  core::page_aligned_vector<Fill> fills;
  core::page_aligned_vector<MBPUpdate> bids, asks, final_bids, final_asks;
  core::page_aligned_vector<Trade> trades;
  core::page_aligned_vector<Statistics> statistics;

  absl::flat_hash_map<Symbol, double> multiplier;

 private:
  server::Dispatcher &dispatcher_;
  uint32_t request_id_ = 0;
  core::stack::Buffer<char, 32> stack_buffer_;
  bool const multicast_;

 public:
  core::limit::RateLimiter rate_limiter;
  absl::flat_hash_set<std::string> all_currencies;
  absl::flat_hash_set<Symbol> all_symbols;
  core::Symbols symbols;
  absl::node_hash_map<Symbol, core::market::MBP_Sequencer> mbp_collector;
};

}  // namespace cme
}  // namespace roq
