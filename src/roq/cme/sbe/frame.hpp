/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <fmt/format.h>

#include <cstdint>
#include <span>

namespace roq {
namespace cme {
namespace sbe {

struct Frame final {
  uint16_t packet_length = {};
  uint16_t channel_id = {};
  uint32_t sequence_number = {};

  static size_t size() { return 8; }

  template <typename Callback>
  static bool parse(std::span<std::byte const> const &buffer, Callback &&callback) {
    auto [result, frame] = parse_helper(buffer);
    if (result)
      callback(frame);
    return result;
  }

 private:
  static std::pair<bool, Frame> parse_helper(std::span<std::byte const> const &buffer);
};

}  // namespace sbe
}  // namespace cme
}  // namespace roq

template <>
struct fmt::formatter<roq::cme::sbe::Frame> {
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(roq::cme::sbe::Frame const &value, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(packet_length={}, )"
        R"(channel_id={}, )"
        R"(sequence_number={})"
        R"(}})"sv,
        value.packet_length,
        value.channel_id,
        value.sequence_number);
  }
};
