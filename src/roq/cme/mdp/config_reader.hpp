/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>

#include <string>
#include <string_view>

#include "roq/api.hpp"

namespace roq {
namespace cme {
namespace mdp {

struct ConfigReader final {
  struct Connection final {
    std::string type;
    std::string feed_type;
    std::string protocol;
    std::string ip;
    std::vector<std::string> host_ips;
    std::string port = {};
    std::string feed;
  };
  struct Channel final {
    std::string label;
    absl::flat_hash_set<std::string> products;
    absl::flat_hash_map<std::string, Connection> connections;
  };

  struct Handler {
    virtual void operator()(std::string_view const &channel_id, Channel const &) = 0;
  };

  static void read(Handler &, std::string_view const &filename);

  static void dispatch(Handler &, std::string_view const &buffer);
};

}  // namespace mdp
}  // namespace cme
}  // namespace roq
