/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <string>
#include <string_view>

#include "roq/api.hpp"

#include "roq/utils/container.hpp"

namespace roq {
namespace cme {
namespace protocol {
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
    utils::unordered_set<std::string> products;
    utils::unordered_map<std::string, Connection> connections;

    void clear() {
      label.clear();
      products.clear();
      connections.clear();
    }
  };

  struct Handler {
    virtual void operator()(uint16_t channel_id, Channel const &) = 0;
  };

  static void read(Handler &, std::string_view const &filename);

  static void dispatch(Handler &, std::string_view const &buffer);
};

}  // namespace mdp
}  // namespace protocol
}  // namespace cme
}  // namespace roq
