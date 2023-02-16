/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/cme/security.hpp"

#include "roq/logging.hpp"

using namespace std::literals;

namespace roq {
namespace cme {

bool Security::update_rpt_seq(uint32_t rpt_seq) {
  if (rpt_seq == 0)  // conflated feed
    return false;
  log::info<5>("rpt_seq={}, last_rpt_seq={}"sv, rpt_seq, (*this).rpt_seq);
  auto next = (*this).rpt_seq + 1;
  auto result = rpt_seq != next;
  if (result) {
    log::warn(
        R"(*** RESUBSCRIBE *** exchange="{}", sybmol="{}", rpt_seq={}, prev={})"sv,
        exchange,
        symbol,
        rpt_seq,
        (*this).rpt_seq);
  }
  (*this).rpt_seq = rpt_seq;
  return result;
}

}  // namespace cme
}  // namespace roq
