/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/cme/mdp/frame.hpp"

#include "roq/logging.hpp"

#include "roq/utils/byte_order.hpp"

using namespace std::literals;

namespace roq {
namespace cme {
namespace mdp {

std::pair<bool, Frame> Frame::parse_helper(std::span<std::byte const> const &buffer) {
  if (std::size(buffer) < size()) {
    log::warn("Invalid message, size={}"sv, std::size(buffer));
    return {false, {}};
  }
  uint32_t sequence_number;
  std::memcpy(&sequence_number, &buffer[0], sizeof(sequence_number));
  uint64_t sending_time;
  std::memcpy(&sending_time, &buffer[4], sizeof(sending_time));
  return {
      true,
      {
          .sequence_number = utils::little_endian_to_host(sequence_number),
          .sending_time = std::chrono::nanoseconds{utils::little_endian_to_host(sending_time)},
      },
  };
}

}  // namespace mdp
}  // namespace cme
}  // namespace roq
