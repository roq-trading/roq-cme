/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <absl/container/node_hash_map.h>

#include <string>
#include <string_view>

#include "roq/priority.hpp"

#include "roq/cme/multicast/type.hpp"

namespace roq {
namespace cme {
namespace multicast {

struct Config final {
  struct Connection final {
    std::string multicast_address;
    uint16_t port = {};
  };

  explicit Config(std::string_view const &filename);

  template <typename Callback>
  bool find(std::string_view const &channel_id, Type type, Priority priority, Callback callback) const {
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

 private:
  // channel => type => feed => connectoin
  absl::node_hash_map<std::string, absl::node_hash_map<Type, absl::node_hash_map<Priority, Connection>>> connections_;
};

}  // namespace multicast
}  // namespace cme
}  // namespace roq
