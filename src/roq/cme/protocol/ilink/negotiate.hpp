/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <fmt/chrono.h>
#include <fmt/format.h>

#include <chrono>
#include <span>
#include <string_view>

#include "roq/utils/debug/hex/message.hpp"

namespace roq {
namespace cme {
namespace protocol {
namespace ilink {

struct Negotiate final {
  std::span<std::byte const> encode(std::span<std::byte> const &buffer) const;

  std::span<std::byte const> hmac_signature;
  std::string_view access_key_id;
  uint64_t uuid = {};
  std::chrono::nanoseconds request_timestamp = {};
  std::string_view session;
  std::string_view firm;
};

}  // namespace ilink
}  // namespace protocol
}  // namespace cme
}  // namespace roq

template <>
struct fmt::formatter<roq::cme::protocol::ilink::Negotiate> {
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(roq::cme::protocol::ilink::Negotiate const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(hmac_signature=[{}], )"
        R"(access_key_id="{}", )"
        R"(uuid={}, )"
        R"(request_timestamp={}, )"
        R"(session="{}", )"
        R"(firm="{}")"
        R"(}})"sv,
        roq::utils::debug::hex::Message{value.hmac_signature},
        value.access_key_id,
        value.uuid,
        value.request_timestamp,
        value.session,
        value.firm);
  }
};
