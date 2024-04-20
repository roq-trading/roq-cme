/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "roq/utils/container.hpp"

#include "roq/cme/tools/security.hpp"

namespace roq {
namespace cme {
namespace market_data {

struct SecurityDefinitions final {
  struct Dispatcher {
    virtual bool discard_symbol(std::string_view const &name) = 0;
  };

  SecurityDefinitions(Dispatcher &, std::string_view const &secdef_config_file);

  SecurityDefinitions(SecurityDefinitions &&) = default;
  SecurityDefinitions(SecurityDefinitions const &) = delete;

  bool has_security(int32_t security_id) { return securities_.find(security_id) != std::end(securities_); }

  template <typename Callback>
  void create_security(
      std::string_view const &security_group,
      uint8_t market_segment_id,
      int32_t security_id,
      tools::Security &&security,
      Callback callback) {
    if (!security.discard) {
      security_groups_[security_group].insert(security_id);
      market_segments_[market_segment_id].try_emplace(static_cast<std::string_view>(security.symbol), security_id);
    }
    auto iter = securities_.try_emplace(security_id, std::move(security)).first;
    callback((*iter).second);
  }

  template <typename Callback>
  bool get_security(int32_t security_id, Callback callback) {
    auto iter = securities_.find(security_id);
    if (iter == std::end(securities_))
      return false;
    auto &security = (*iter).second;
    if (security.discard)
      return false;
    callback(security);
    return true;
  }

  template <typename Callback>
  bool get_security_incl_discard(int32_t security_id, Callback callback) {
    auto iter = securities_.find(security_id);
    if (iter == std::end(securities_))
      return false;
    auto &security = (*iter).second;
    callback(security);
    return true;
  }

  template <typename Callback>
  void get_securities_(Callback callback) {
    for (auto &[security_id, security] : securities_)
      if (!security.discard)
        callback(security);
  }

  // security group

  template <typename Callback>
  bool get_security_group(std::string_view const &security_group, Callback callback) {
    auto iter = security_groups_.find(security_group);
    if (iter == std::end(security_groups_))
      return false;
    auto &security_ids = (*iter).second;
    for (auto &security_id : security_ids)
      callback(security_id);
    return true;
  }

  // symbol

  template <typename Callback>
  bool find_security_id(uint8_t market_segment_id, std::string_view const &symbol, Callback callback) {
    auto iter_1 = market_segments_.find(market_segment_id);
    if (iter_1 == std::end(market_segments_))
      return false;
    auto &symbols = (*iter_1).second;
    auto iter_2 = symbols.find(symbol);
    if (iter_2 == std::end(symbols))
      return false;
    callback((*iter_2).second);
    return true;
  }

 private:
  utils::unordered_map<int32_t, tools::Security> securities_;
  utils::unordered_map<std::string, utils::unordered_set<int32_t>> security_groups_;
  utils::unordered_map<uint8_t, utils::unordered_map<std::string, int32_t>> market_segments_;
};

}  // namespace market_data
}  // namespace cme
}  // namespace roq
