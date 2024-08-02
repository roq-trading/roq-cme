/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include <string>

#include "roq/priority.hpp"

#include "roq/core/udp/buffer.hpp"

namespace roq {
namespace cme {

struct Channel final {
  Channel(std::string_view const &channel_id, size_t buffer_size, size_t buffer_depth);

  Channel(Channel const &) = delete;

  std::string const channel_id;

  std::string get_channel_name(std::string_view const &prefix, Priority = {}) const;

  struct ReorderBuffer final {
    std::pair<bool, uint32_t> last_sequence = {};
    core::udp::Buffer<uint32_t> buffer;
  } incremental;

  // sequencing

  struct {
    uint32_t first_sequence_number = {};
    uint32_t last_sequence_number = {};
  } sequence = {};

  void update_sequence_number(uint32_t sequence_number) {
    if (!sequence.first_sequence_number)
      sequence.first_sequence_number = sequence_number;
    sequence.last_sequence_number = sequence_number;
  }
};

}  // namespace cme
}  // namespace roq
