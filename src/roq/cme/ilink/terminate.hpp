/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include <fmt/chrono.h>
#include <fmt/format.h>

#include <chrono>
#include <span>
#include <string_view>

#include "roq/debug/hex/message.hpp"

#include <cme_ilink/SplitMsg.h>

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
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(roq::cme::ilink::Terminate const &value, Context &context) const {
    using namespace std::literals;
    using namespace fmt::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(reason="{}", )"
        R"(uuid={}, )"
        R"(request_timestamp={}, )"
        R"(error_codes={}, )"
        R"(split_msg="{}")"
        R"(}})"_cf,
        value.reason,
        value.uuid,
        value.request_timestamp,
        value.error_codes,
        value.split_msg);
  }
};
