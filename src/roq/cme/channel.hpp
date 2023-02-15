/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <absl/container/flat_hash_map.h>

#include <string>

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
  ReorderBuffer incremental;

  // MBP
  absl::flat_hash_map<int32_t, uint32_t> mbp_last_sequence;
  absl::flat_hash_map<int32_t, uint32_t> mbp_resubscribe;

  // MBO
  absl::flat_hash_map<int32_t, uint32_t> mbo_last_sequence;
  absl::flat_hash_map<int32_t, uint32_t> mbo_resubscribe;
};

}  // namespace cme
}  // namespace roq
