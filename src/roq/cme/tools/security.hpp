/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include <fmt/format.h>

#include "roq/api.hpp"

#include "roq/market/mbp/sequencer.hpp"

#include "roq/market/mbo/sequencer.hpp"

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
    market::mbp::Sequencer sequencer;
    uint32_t resubscribe = {};  // XXX move to sequencer
  } mbp = {};

  struct {
    uint32_t no_chunks = {};
    uint32_t last_chunk = {};
    std::vector<MBOUpdate> orders;
    market::mbo::Sequencer sequencer;
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
      if (current_chunk == mbo.last_chunk) {
        // same
      } else if (current_chunk == (mbo.last_chunk + uint32_t{1})) {
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

template <>
struct fmt::formatter<roq::cme::tools::Security> {
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(roq::cme::tools::Security const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(exchange="{}", )"
        R"(symbol="{}", )"
        R"(display_factor={}, )"
        R"(discard={})"
        R"(}})"sv,
        value.exchange,
        value.symbol,
        value.display_factor,
        value.discard);
  }
};
