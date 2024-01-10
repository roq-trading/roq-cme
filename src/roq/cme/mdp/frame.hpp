/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include <fmt/chrono.h>
#include <fmt/format.h>

#include <chrono>
#include <cstdint>
#include <span>

namespace roq {
namespace cme {
namespace mdp {

struct Frame final {
  uint32_t sequence_number = {};
  std::chrono::nanoseconds sending_time = {};

  static size_t size() { return 12; }

  template <typename Callback>
  static bool parse(std::span<std::byte const> const &buffer, Callback callback) {
    auto [result, frame] = parse_helper(buffer);
    if (result)
      callback(frame);
    return result;
  }

 private:
  static std::pair<bool, Frame> parse_helper(std::span<std::byte const> const &buffer);
};

}  // namespace mdp
}  // namespace cme
}  // namespace roq

template <>
struct fmt::formatter<roq::cme::mdp::Frame> {
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(roq::cme::mdp::Frame const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(sequence_number={}, )"
        R"(sending_time={})"
        R"(}})"sv,
        value.sequence_number,
        value.sending_time);
  }
};
