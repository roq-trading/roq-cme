/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <cme_mdp/MDIncrementalRefreshBook46.h>

#include "roq/core/sbe/iterator.hpp"

#include "roq/cme/sbe/utils.hpp"

namespace roq {
namespace cme {
namespace sbe {

template <>
inline size_t compute_length(cme_mdp::MDIncrementalRefreshBook46 &value) {
  // NoMDEntries
  auto no_md_entries_length = value.noMDEntries().count();
  value.sbeRewind();  // wtf!
  value.noMDEntries().forEach([](auto &e) { e.skip(); });
  // NoOrderIDEntries
  auto no_order_id_entries_length = value.noOrderIDEntries().count();
  value.noOrderIDEntries().forEach([](auto &e) { e.skip(); });
  return value.computeLength(no_md_entries_length, no_order_id_entries_length);
}

}  // namespace sbe
}  // namespace cme
}  // namespace roq

// MDIncrementalRefreshBook46

template <>
struct fmt::formatter<cme_mdp::MDIncrementalRefreshBook46::NoMDEntries> {
  using value_type = cme_mdp::MDIncrementalRefreshBook46::NoMDEntries;
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type const &value, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(md_entry_px={}, )"
        R"(md_entry_size={}, )"
        R"(}})"sv,
        const_cast<value_type &>(value).mDEntryPx(),
        value.mDEntrySize());
  }
};

template <>
struct fmt::formatter<cme_mdp::MDIncrementalRefreshBook46::NoOrderIDEntries> {
  using value_type = cme_mdp::MDIncrementalRefreshBook46::NoOrderIDEntries;
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type const &value, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(order_id={}, )"
        R"(md_order_priority={}, )"
        R"(}})"sv,
        value.orderID(),
        value.mDOrderPriority());
  }
};

template <>
struct fmt::formatter<cme_mdp::MDIncrementalRefreshBook46> {
  using value_type = cme_mdp::MDIncrementalRefreshBook46;
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type &value, Context &context) const {
    using namespace std::literals;
    value.sbeRewind();  // note!
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(transact_time={}, )"
        R"(match_event_indicator={}, )"
        R"(no_md_entries=[{}], )"
        R"(no_order_id_entries=[{}])"
        R"(}})"sv,
        value.transactTime(),
        value.matchEventIndicator(),
        fmt::join(roq::core::sbe::iterator{value.noMDEntries()}, roq::core::sbe::sentinel{}, ", "sv),
        fmt::join(roq::core::sbe::iterator{value.noOrderIDEntries()}, roq::core::sbe::sentinel{}, ", "sv));
  }
};
