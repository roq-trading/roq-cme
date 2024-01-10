/* Copyright (c) 2017-2024, Hans Erik Thrane */

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

struct Establish final {
  std::span<std::byte const> encode(std::span<std::byte> const &buffer) const;

  std::span<std::byte const> hmac_signature;
  std::string_view access_key_id;
  std::string_view trading_system_name;
  std::string_view trading_system_version;
  std::string_view trading_system_vendor;
  uint64_t uuid = {};
  std::chrono::nanoseconds request_timestamp = {};
  uint32_t next_seq_no = {};
  std::string_view session;
  std::string_view firm;
  std::chrono::milliseconds keep_alive_interval = {};
};

}  // namespace ilink
}  // namespace cme
}  // namespace roq

template <>
struct fmt::formatter<roq::cme::ilink::Establish> {
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(roq::cme::ilink::Establish const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(hmac_signature=[{}], )"
        R"(access_key_id="{}", )"
        R"(trading_system_name="{}", )"
        R"(trading_system_version="{}", )"
        R"(trading_system_vendor="{}", )"
        R"(uuid={}, )"
        R"(request_timestamp={}, )"
        R"(next_seq_no={}, )"
        R"(session="{}", )"
        R"(firm="{}", )"
        R"(keep_alive_interval="{}", )"
        R"(}})"sv,
        roq::debug::hex::Message{value.hmac_signature},
        value.access_key_id,
        value.trading_system_name,
        value.trading_system_version,
        value.trading_system_vendor,
        value.uuid,
        value.request_timestamp,
        value.next_seq_no,
        value.session,
        value.firm,
        value.keep_alive_interval);
  }
};
