/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include <chrono>
#include <string_view>

namespace roq {
namespace cme {
namespace market_data {

struct Options final {
  // server
  bool cache_all_reference_data = {};
  // mdp
  bool enable_market_by_order = {};
  bool mbp_to_mbo_clear_price_level = {};
  size_t filter_snapshot_from_incremental = {};
  // network
  std::string_view local_interface;
  std::chrono::nanoseconds multicast_timeout = {};
  // misc
  std::string_view secdef_config_file;
  std::chrono::nanoseconds pcap_first_timestamp = {};
};

}  // namespace market_data
}  // namespace cme
}  // namespace roq
