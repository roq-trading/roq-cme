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
  using time_type = std::chrono::nanoseconds;

  std::span<std::byte const> encode(std::span<std::byte> const &buffer) const;

  std::span<std::byte const> hmac_signature;
  std::string_view access_key_id;
  uint64_t uuid = {};
  time_type request_timestamp = {};
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
        R"(hmac_signature=[{}], )"
        R"(access_key_id="{}", )"
        R"(uuid={}, )"
        R"(request_timestamp={}, )"
        R"(session="{}", )"
        R"(firm="{}")"
        R"(}})"_cf,
        roq::debug::hex::Message{value.hmac_signature},
        value.access_key_id,
        value.uuid,
        value.request_timestamp,
        value.session,
        value.firm);
  }
};
