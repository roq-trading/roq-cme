/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include <string>
#include <string_view>
#include <tuple>

#include "roq/priority.hpp"

#include "roq/utils/container.hpp"

#include "roq/cme/mdp/connection_type.hpp"

namespace roq {
namespace cme {
namespace mdp {

struct Config final {
  struct Connection final {
    std::string multicast_address;
    uint16_t port = {};
  };

  Config(std::string_view const &filename, bool verbose);

  template <typename Callback>
  bool find(std::string_view const &channel_id, ConnectionType type, Priority priority, Callback callback) const {
    auto iter_1 = connections_.find(channel_id);
    if (iter_1 == std::end(connections_))
      return false;
    auto &tmp_1 = (*iter_1).second;
    auto iter_2 = tmp_1.find(type);
    if (iter_2 == std::end(tmp_1))
      return false;
    auto &tmp_2 = (*iter_2).second;
    auto iter_3 = tmp_2.find(priority);
    if (iter_3 == std::end(tmp_2))
      return false;
    callback((*iter_3).second);
    return true;
  }

  template <typename Callback>
  bool find(std::string_view const &address, uint16_t port, Callback callback) const {
    auto iter_1 = connection_types_.find(address);
    if (iter_1 != std::end(connection_types_)) {
      auto &tmp_1 = (*iter_1).second;
      auto iter_2 = tmp_1.find(port);
      if (iter_2 != std::end(tmp_1)) {
        auto [channel_id, connection_type, priority] = (*iter_2).second;
        callback(channel_id, connection_type, priority);
        return true;
      }
    }
    return false;
  }

 private:
  // channel_id(string) => type => feed => connection
  utils::unordered_map<std::string, utils::unordered_map<ConnectionType, utils::unordered_map<Priority, Connection>>>
      connections_;

  // address ==> port ==> {channel_id, connection type, priority}
  utils::unordered_map<std::string, utils::unordered_map<uint16_t, std::tuple<uint16_t, ConnectionType, Priority>>>
      connection_types_;
};

}  // namespace mdp
}  // namespace cme
}  // namespace roq
