/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <chrono>
#include <string_view>

namespace roq {
namespace cme {
namespace tools {

struct CanonicalMessage final {
  std::chrono::nanoseconds request_timestamp = {};
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
