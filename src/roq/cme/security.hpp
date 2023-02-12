/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include "roq/api.hpp"

namespace roq {
namespace cme {

struct Security final {
  Exchange exchange;
  Symbol symbol;
  double display_factor = NaN;
  bool discard = {};
  uint32_t rpt_seq = {};  // conflated feed sends zero
  bool need_snapshot = false;

  // Security(Security const &) = delete;
  // void operator=(Security const &) = delete;

  void update_rpt_seq(uint32_t rpt_seq);
};

}  // namespace cme
}  // namespace roq
