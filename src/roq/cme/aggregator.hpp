/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include "roq/logging.hpp"
#include "roq/mbp_update.hpp"

#include "roq/core/memory.hpp"

namespace roq {
namespace cme {

struct Aggregator final {
  Aggregator(size_t mbp_size) {
    bids_.reserve(mbp_size);
    asks_.reserve(mbp_size);
  }

  void reset() {
    previous_instrument_id_ = {};
    previous_change_id_ = {};
    skip_instrument_id_ = {};
    skip_change_id_ = {};
    bids_.clear();
    asks_.clear();
  }

  // any
  bool operator()(uint32_t sequence_number) {
    using namespace std::literals;
    auto result = true;
    if (sequence_number != previous_sequence_number_) {  // note! packed messages are allowed
      if (sequence_number != previous_sequence_number_ && sequence_number != (previous_sequence_number_ + 1))
          [[unlikely]] {
        if (sequence_number < previous_sequence_number_) [[unlikely]] {
          // not overflow?
          if (sequence_number != 0 || previous_sequence_number_ != std::numeric_limits<uint32_t>::max())
            result = false;
        } else if (sequence_number > 255 && previous_sequence_number_ == 0) {
          // note! relaxed when initializing -- could be wrong, but very unlikely
        } else {
          result = false;
        }
      }
      if (!result) [[unlikely]] {
        log::info<1>(
            "*** OUT OF SEQUENCE *** sequence_number={}, previous_sequence_number={}"sv,
            sequence_number,
            previous_sequence_number_);
      }
      previous_sequence_number_ = sequence_number;
    }
    return result;
  }

  // book or snapshot
  template <typename Callback>
  void operator()(
      uint32_t sequence_number, uint32_t instrument_id, uint64_t change_id, bool is_last, Callback callback) {
    using namespace std::literals;
    if ((*this)(sequence_number)) {
      // in sequence
      if (is_last) {
        // last in book
        if (previous_instrument_id_) {
          // prior updates
          if (instrument_id == previous_instrument_id_ && change_id == previous_change_id_) {
            if (!skip_instrument_id_) {
              log::info<3>("DEBUG: AGGREGATE instrument_id={}, change_id={}"sv, instrument_id, change_id);
              callback(bids_, asks_);
            } else {
              log::info<3>("DEBUG: SKIP instrument_id={}, change_id={}"sv, instrument_id, change_id);
            }
          } else {
            log::info<1>(
                "DEBUG: INCONSISTENT instrument_id={}(/{}), change_id={}(/{})"sv,
                instrument_id,
                previous_instrument_id_,
                change_id,
                previous_change_id_);
          }
        } else {
          // no prior updates
          log::info<3>("DEBUG: SIMPLE instrument_id={}, change_id={}"sv, instrument_id, change_id);
          callback(bids_, asks_);
        }
        reset();
      } else {
        // not last in book
        if (previous_instrument_id_) {
          if (!skip_instrument_id_ && (instrument_id != previous_instrument_id_ || change_id != previous_change_id_)) {
            log::info<1>(
                "DEBUG: INCONSISTENT instrument_id={}(/{}), change_id={}(/{})"sv,
                instrument_id,
                previous_instrument_id_,
                change_id,
                previous_change_id_);
            skip_instrument_id_ = instrument_id;
            skip_change_id_ = change_id;
          }
        } else {
          previous_instrument_id_ = instrument_id;
          previous_change_id_ = change_id;
        }
      }
    } else {
      // out of sequence
      if (is_last) {
        reset();
      } else {
        previous_instrument_id_ = instrument_id;
        previous_change_id_ = change_id;
      }
    }
  }

 private:
  uint32_t previous_sequence_number_ = {};
  uint32_t previous_instrument_id_ = {};
  uint64_t previous_change_id_ = {};
  uint32_t skip_instrument_id_ = {};
  uint64_t skip_change_id_ = {};
  // -- note! required here because book updates may span multiple packets
  core::page_aligned_vector<MBPUpdate> bids_, asks_;
};

}  // namespace cme
}  // namespace roq
