/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <cstdint>
#include <string_view>

namespace roq {
namespace cme {
namespace channel_tester {
namespace flags {

struct Flags final {
  static std::string_view multicast_address();
  static uint16_t multicast_port();
  static std::string_view local_interface();
  static uint32_t filter_snapshot_from_incremental();
};

}  // namespace flags
}  // namespace channel_tester
}  // namespace cme
}  // namespace roq
