/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include "roq/api.hpp"

#include "roq/core/mbp/sequencer.hpp"

#include "roq/core/mbo/sequencer.hpp"

#include "roq/logging.hpp"

namespace roq {
namespace cme {
namespace tools {

struct Security final {
  Exchange exchange;
  Symbol symbol;

  double display_factor = NaN;
  bool discard = {};

  uint32_t rpt_seq = {};  // conflated feed sends zero

  bool update_rpt_seq(uint32_t rpt_seq);

  // sequencing

  struct {
    core::mbp::Sequencer sequencer;
    uint32_t resubscribe = {};  // XXX move to sequencer
  } mbp = {};

  struct {
    uint32_t no_chunks = {};
    uint32_t last_chunk = {};
    std::vector<MBOUpdate> orders;
    core::mbo::Sequencer sequencer;
    uint32_t resubscribe = {};  // XXX move to sequencer
  } mbo = {};

  template <typename Callback>
  bool update_mbo_snapshot(uint32_t current_chunk, uint32_t no_chunks, Callback callback) {
    using namespace std::literals;
    log::info(
        "DEBUG mbo={{no_chunks={}, last_chunk={}, len(orders)={}}}"sv,
        mbo.no_chunks,
        mbo.last_chunk,
        std::size(mbo.orders));
    auto reset = [&]() {
      mbo.no_chunks = {};
      mbo.last_chunk = {};
      mbo.orders.clear();
    };
    if (mbo.no_chunks) {
      if (current_chunk == (mbo.last_chunk + uint32_t{1})) {
        if (current_chunk == no_chunks) {
          callback(mbo.orders, true);
          reset();
          return true;
        } else {
          mbo.last_chunk = current_chunk;
          callback(mbo.orders, false);
        }
      } else {
        reset();
      }
    } else if (current_chunk == uint32_t{1}) {
      mbo.no_chunks = no_chunks;
      mbo.last_chunk = current_chunk;
      auto last = current_chunk == no_chunks;
      callback(mbo.orders, last);
    }
    return false;
  }
};

}  // namespace tools
}  // namespace cme
}  // namespace roq
