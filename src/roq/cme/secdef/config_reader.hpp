/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/api.hpp"

namespace roq {
namespace cme {
namespace secdef {

struct ConfigReader final {
  struct SecDef final {
    uint32_t security_id = {};
    uint8_t market_segment_id = {};
    // note! orderd to (mostly) align with ReferenceData
    std::string_view exchange;
    std::string_view symbol;
    std::string_view security_type;
    std::string_view currency;
    int32_t multiplier = {};
    uint32_t min_trade_vol = {};
    uint32_t max_trade_vol = {};
    double min_price_increment = NaN;
    double display_factor = NaN;
    std::string_view asset;
    bool discard = false;
  };

  struct Handler {
    virtual void operator()(SecDef const &) = 0;
  };

  static void read(Handler &, std::string_view const &filename);

  static void dispatch(Handler &, std::string_view const &buffer);
};

}  // namespace secdef
}  // namespace cme
}  // namespace roq
