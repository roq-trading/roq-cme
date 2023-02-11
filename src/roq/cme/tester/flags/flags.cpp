/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/cme/tester/flags/flags.hpp"

#include <absl/flags/flag.h>

#include <string>

#include "roq/core/flags/non_empty.hpp"
#include "roq/core/flags/non_zero.hpp"

ABSL_FLAG(  //
    roq::core::flags::NonEmpty,
    multicast_address,
    {},
    "multicast address");

ABSL_FLAG(  //
    roq::core::flags::NonZero<uint32_t>,
    multicast_port,
    {},
    "multicast port");

ABSL_FLAG(  //
    roq::core::flags::NonEmpty,
    local_interface,
    {},
    "local interface");

namespace roq {
namespace cme {
namespace tester {
namespace flags {

std::string_view Flags::multicast_address() {
  static const std::string result = absl::GetFlag(FLAGS_multicast_address);
  return result;
}

uint16_t Flags::multicast_port() {
  static const uint16_t result = absl::GetFlag(FLAGS_multicast_port);
  return result;
}

std::string_view Flags::local_interface() {
  static const std::string result = absl::GetFlag(FLAGS_local_interface);
  return result;
}

}  // namespace flags
}  // namespace tester
}  // namespace cme
}  // namespace roq
