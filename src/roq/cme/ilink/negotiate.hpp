/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <fmt/chrono.h>
#include <fmt/format.h>

#include <chrono>
#include <span>
#include <string_view>

#include "roq/debug/hex/message.hpp"

namespace roq {
namespace cme {
namespace ilink {

struct Negotiate final {
  std::span<std::byte const> encode(std::span<std::byte> const &buffer) const;

  std::span<std::byte const> signature;
  int64_t uuid = {};
  std::chrono::milliseconds request_timestamp = {};
  std::string_view session;
  std::string_view firm;
};

}  // namespace ilink
}  // namespace cme
}  // namespace roq

template <>
struct fmt::formatter<roq::cme::ilink::Negotiate> {
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(roq::cme::ilink::Negotiate const &value, Context &context) const {
    using namespace std::literals;
    using namespace fmt::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(signature=[{}], )"
        R"(uuid={}, )"
        R"(reuqest_timestamp={}, )"
        R"(session="{}", )"
        R"(firm="{}", )"
        R"(}})"_cf,
        roq::debug::hex::Message{value.signature},
        value.uuid,
        value.request_timestamp,
        value.session,
        value.firm);
  }
};
