/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>

#include <string>
#include <string_view>

#include "roq/api.hpp"

namespace roq {
namespace cme {
namespace tools {

struct ConfReader final {
  struct Connection final {
    std::string type;
    std::string protocol;
    std::string ip;
    std::vector<std::string> host_ips;
    uint16_t port = {};
    std::string feed;
  };
  struct Channel final {
    std::string label;
    absl::flat_hash_set<std::string> products;
    absl::flat_hash_map<std::string, Connection> connections;
  };

  struct Handler {
    virtual void operator()(uint32_t channel_id, Channel &&) = 0;
  };

  static void read(Handler &, std::string_view const &filename);

  static void dispatch(Handler &, std::string_view const &buffer);
};

}  // namespace tools
}  // namespace cme
}  // namespace roq
