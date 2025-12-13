/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <string>
#include <string_view>

#include "roq/api.hpp"

namespace roq {
namespace cme {
namespace ilink {

struct ConfigReader final {
  struct MarketSegment final {
    std::string label;
    std::string protocol;
    std::string primary_host_ip;
    std::string backup_host_ip;
    std::string dr_primary_host_ip;
    std::string dr_backup_host_ip;
    std::string to_cme_schema_version = {};    // XXX uint8_t
    std::string from_cme_schema_version = {};  // XXX uint8_t
  };

  struct Handler {
    virtual void operator()(uint8_t market_segment_id, MarketSegment const &) = 0;
  };

  static void read(Handler &, std::string_view const &filename);

  static void dispatch(Handler &, std::string_view const &buffer);
};

}  // namespace ilink
}  // namespace cme
}  // namespace roq
