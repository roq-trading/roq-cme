/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <absl/container/flat_hash_map.h>
#include <absl/container/node_hash_map.h>

#include <string>

#include "roq/core/mbp/sequencer.hpp"

#include "roq/core/mbo/sequencer.hpp"

#include "roq/core/udp/buffer.hpp"

namespace roq {
namespace cme {

struct Channel final {
  Channel(std::string_view const &channel_id, size_t buffer_size, size_t buffer_depth);

  Channel(Channel &&) = default;
  Channel(Channel const &) = delete;

  std::string const channel_id;

  struct ReorderBuffer final {
    std::pair<bool, uint32_t> last_sequence = {};
    core::udp::Buffer<uint32_t> buffer;
  };

  // ReorderBuffer instrument_definition;
  ReorderBuffer incremental;
  // ReorderBuffer mbp_market_recovery;
  ReorderBuffer mbo_market_recovery;

  // incremental
  absl::flat_hash_map<int32_t, uint32_t> mbp_last_sequence;
  absl::flat_hash_map<int32_t, uint32_t> mbo_last_sequence;

  // MBP recovery
  absl::node_hash_map<int32_t, core::mbp::Sequencer> mbp_collector;
  absl::flat_hash_map<int32_t, uint32_t> mbp_resubscribe;

  // MBO recovery
  absl::node_hash_map<int32_t, core::mbo::Sequencer> mbo_collector;
  absl::flat_hash_map<int32_t, uint32_t> mbo_resubscribe;
};

}  // namespace cme
}  // namespace roq
