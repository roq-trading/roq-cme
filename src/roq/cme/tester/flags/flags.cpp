/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/cme/tester/flags/flags.hpp"

#include <absl/flags/flag.h>

#include <string>

#include "roq/core/flags/flags.hpp"

using namespace std::literals;

ABSL_FLAG(  //
    roq::core::flags::Path,
    auth_keys_file,
    ""s,
    "file containing user's public and private keys (path)"s);

ABSL_FLAG(  //
    std::vector<std::string>,
    auth_license_manager_uri,
    std::vector<std::string>({"https://auth-1.roq-trading.com"s, "https://auth-2.roq-trading.com"s}),
    "license manager end-point(s) (comma-separated list)"s);

ABSL_FLAG(  //
    roq::core::flags::URI,
    auth_proxy,
    {},
    "proxy end-point (URI)"s);

namespace roq {
namespace cme {
namespace tester {
namespace flags {

std::string_view Flags::auth_keys_file() {
  static const std::string result{absl::GetFlag(FLAGS_auth_keys_file)};
  return result;
}

std::vector<std::string> const &Flags::auth_license_manager_uri() {
  static const std::vector<std::string> result{absl::GetFlag(FLAGS_auth_license_manager_uri)};
  return result;
}

roq::io::web::URI const &Flags::auth_proxy() {
  static const roq::io::web::URI result{absl::GetFlag(FLAGS_auth_proxy)};
  return result;
}

}  // namespace flags
}  // namespace tester
}  // namespace cme
}  // namespace roq
