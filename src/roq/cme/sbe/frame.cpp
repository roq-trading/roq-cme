/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/deribit/sbe/frame.hpp"

#include "roq/logging.hpp"

#include "roq/core/byte_order.hpp"

using namespace std::literals;

namespace roq {
namespace deribit {
namespace sbe {

std::pair<bool, Frame> Frame::parse_helper(std::span<std::byte const> const &buffer) {
  if (std::size(buffer) < size()) {
    log::warn("Invalid message, size={}"sv, std::size(buffer));
    return {false, {}};
  }
  uint16_t packet_length;
  std::memcpy(&packet_length, &buffer[0], sizeof(packet_length));
  uint16_t channel_id;
  std::memcpy(&channel_id, &buffer[2], sizeof(channel_id));
  uint32_t sequence_number;
  std::memcpy(&sequence_number, &buffer[4], sizeof(sequence_number));
  return {
      true,
      {
          .packet_length = core::little_endian_to_host(packet_length),
          .channel_id = core::little_endian_to_host(channel_id),
          .sequence_number = core::little_endian_to_host(sequence_number),
      },
  };
}

}  // namespace sbe
}  // namespace deribit
}  // namespace roq
