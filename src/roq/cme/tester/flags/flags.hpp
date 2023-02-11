/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <string_view>
#include <vector>

#include "roq/io/web/uri.hpp"

namespace roq {
namespace cme {
namespace tester {
namespace flags {

struct Flags final {
  static std::string_view auth_keys_file();
  static std::vector<std::string> const &auth_license_manager_uri();
  static roq::io::web::URI const &auth_proxy();
};

}  // namespace flags
}  // namespace tester
}  // namespace cme
}  // namespace roq
