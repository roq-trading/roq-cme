/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include <fmt/chrono.h>
#include <fmt/format.h>

#include <chrono>
#include <span>
#include <string_view>

#include "roq/debug/hex/message.hpp"

#include <cme_ilink/SplitMsg.h>

#include "roq/cme/ilink/utils.hpp"

namespace roq {
namespace cme {
namespace ilink {

struct Terminate final {
  std::span<std::byte const> encode(std::span<std::byte> const &buffer) const;

  std::string_view reason;
  uint64_t uuid = {};
  std::chrono::nanoseconds request_timestamp = {};
  uint16_t error_codes = {};
  cme_ilink::SplitMsg::Value split_msg = {};
};

}  // namespace ilink
}  // namespace cme
}  // namespace roq

template <>
struct fmt::formatter<roq::cme::ilink::Terminate> {
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(roq::cme::ilink::Terminate const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(reason="{}", )"
        R"(uuid={}, )"
        R"(request_timestamp={}, )"
        R"(error_codes={}, )"
        R"(split_msg="{}")"
        R"(}})"sv,
        value.reason,
        value.uuid,
        value.request_timestamp,
        value.error_codes,
        value.split_msg);
  }
};
