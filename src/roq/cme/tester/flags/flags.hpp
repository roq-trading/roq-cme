/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <cstdint>
#include <string_view>

namespace roq {
namespace cme {
namespace tester {
namespace flags {

struct Flags final {
  static std::string_view multicast_address();
  static uint16_t multicast_port();
  static std::string_view local_interface();
};

}  // namespace flags
}  // namespace tester
}  // namespace cme
}  // namespace roq
