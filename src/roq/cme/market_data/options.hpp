/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include <chrono>
#include <string_view>

namespace roq {
namespace cme {
namespace market_data {

struct Options final {
  bool cache_all_reference_data = {};
  std::string_view local_interface;
  std::chrono::nanoseconds multicast_timeout = {};
};

}  // namespace market_data
}  // namespace cme
}  // namespace roq
