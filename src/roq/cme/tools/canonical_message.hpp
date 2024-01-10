/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include <fmt/chrono.h>
#include <fmt/compile.h>
#include <fmt/format.h>

#include <chrono>
#include <string_view>

namespace roq {
namespace cme {
namespace tools {

struct CanonicalMessage final {
  using time_type = std::chrono::nanoseconds;

  time_type request_timestamp = {};
  uint64_t uuid = {};
  std::string_view session;
  std::string_view firm_id;
  std::string_view trading_system_name;
  std::string_view trading_system_version;
  std::string_view trading_system_vendor;
  uint32_t next_seq_no = {};
  std::chrono::milliseconds keep_alive_interval = {};
};

}  // namespace tools
}  // namespace cme
}  // namespace roq

template <>
struct fmt::formatter<roq::cme::tools::CanonicalMessage> {
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(roq::cme::tools::CanonicalMessage const &value, format_context &context) const {
    using namespace std::literals;
    if (std::empty(value.trading_system_name)) {
      return fmt::format_to(
          context.out(),
          R"({{)"
          R"(timestamp={}, )"
          R"(uuid={}, )"
          R"(session="{}", )"
          R"(firm_id="{}")"
          R"(}})"sv,
          value.request_timestamp,
          value.uuid,
          value.session,
          value.firm_id);
    } else {
      return fmt::format_to(
          context.out(),
          R"({{)"
          R"(timestamp={}, )"
          R"(uuid={}, )"
          R"(session="{}", )"
          R"(firm_id="{}", )"
          R"(trading_system_name="{}", )"
          R"(trading_system_version="{}", )"
          R"(trading_system_vendor="{}", )"
          R"(next_seq_no={}, )"
          R"(keep_alive_interval={})"
          R"(}})"sv,
          value.request_timestamp,
          value.uuid,
          value.session,
          value.firm_id,
          value.trading_system_name,
          value.trading_system_version,
          value.trading_system_vendor,
          value.next_seq_no,
          value.keep_alive_interval);
    }
  }
};
