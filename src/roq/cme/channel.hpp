/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <absl/container/flat_hash_map.h>
#include <absl/container/node_hash_map.h>

#include <string>

#include "roq/core/market/mbp_sequencer.hpp"

namespace roq {
namespace cme {

struct Channel final {
  explicit Channel(std::string_view const &channel_id) : channel_id(channel_id) {}

  Channel(Channel &&) = default;
  Channel(Channel const &) = delete;

  const std::string channel_id;

  // incremental
  uint32_t last_sequence = {};
  absl::flat_hash_map<int32_t, uint32_t> mbp_last_sequence;

  // recovery
  absl::node_hash_map<int32_t, core::market::MBP_Sequencer> mbp_collector;
  absl::flat_hash_map<int32_t, uint32_t> mbp_resubscribe;
};

}  // namespace cme
}  // namespace roq
