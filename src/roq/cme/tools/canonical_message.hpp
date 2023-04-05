/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <chrono>
#include <string_view>

namespace roq {
namespace cme {
namespace tools {

struct CanonicalMessage final {
  std::chrono::milliseconds request_timestamp = {};
  int64_t uuid = {};
  std::string_view session;
  std::string_view firm_id;
  std::string_view trading_system_name;
  std::string_view trading_version_id;
  std::string_view trading_system_vendor_id;
  uint64_t next_seq_no = {};
  std::chrono::seconds keep_alive_interval = {};
};

}  // namespace tools
}  // namespace cme
}  // namespace roq
