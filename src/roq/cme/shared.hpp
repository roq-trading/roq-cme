/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include <string>
#include <utility>
#include <vector>

#include "roq/api.hpp"
#include "roq/server.hpp"

#include "roq/utils/container.hpp"

#include "roq/cme/settings.hpp"

#include "roq/cme/tools/security.hpp"

#include "roq/cme/mdp/config.hpp"

#include "roq/cme/ilink/config_reader.hpp"

#include "roq/cme/market_data/security_definitions.hpp"

namespace roq {
namespace cme {

struct Shared final {
 public:
  Shared(server::Dispatcher &, Settings const &, market_data::SecurityDefinitions &);

  Shared(Shared &&) = default;
  Shared(Shared const &) = delete;

  template <typename Callback>
  bool get_market_segment(uint8_t market_segment_id, Callback callback) {
    auto iter = ilink_config_.find(market_segment_id);
    if (iter == std::end(ilink_config_))
      return false;
    callback((*iter).second);
    return true;
  }

  std::pair<std::string, uint16_t> get_multicast_config(uint16_t channel_id, mdp::ConnectionType, Priority) const;

  template <typename... Args>
  auto update_order(Args &&...args) {
    return dispatcher_.update_order(std::forward<Args>(args)...);
  }

  template <typename... Args>
  auto operator()(Args &&...args) {
    return dispatcher_(std::forward<Args>(args)...);
  }

  auto &get_fills() {
    fills_.clear();
    return fills_;
  }

 private:
  server::Dispatcher &dispatcher_;

 public:
  Settings const &settings;
  market_data::SecurityDefinitions &security_definitions;
  std::vector<std::byte> buffer;
  mdp::Config mdp_config;

 private:
  utils::unordered_map<uint8_t, ilink::ConfigReader::MarketSegment> ilink_config_;
  std::vector<Fill> fills_;
};

}  // namespace cme
}  // namespace roq
