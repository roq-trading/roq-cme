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
  void reset_rpt_seq();

  struct {
    uint32_t no_chunks = {};
    uint32_t last_chunk = {};
    std::vector<MBOUpdate> bids, asks;
  } mbo = {};

  template <typename Callback>
  bool update_mbo_snapshot(uint32_t current_chunk, uint32_t no_chunks, Callback callback) {
    auto reset = [&]() {
      mbo.no_chunks = {};
      mbo.last_chunk = {};
      mbo.bids.clear();
      mbo.asks.clear();
    };
    if (mbo.no_chunks) {
      if (current_chunk == (mbo.last_chunk + uint32_t{1})) {
        if (current_chunk == no_chunks) {
          callback(mbo.bids, mbo.asks, true);
          reset();
          return true;
        } else {
          mbo.last_chunk = current_chunk;
          callback(mbo.bids, mbo.asks, false);
        }
      } else {
        reset();
      }
    } else if (current_chunk == uint32_t{1}) {
      mbo.no_chunks = no_chunks;
      mbo.last_chunk = current_chunk;
    }
    return false;
  }
};

}  // namespace cme
}  // namespace roq
