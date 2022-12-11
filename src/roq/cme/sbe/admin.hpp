/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <fmt/format.h>

#include <cme_mdp/AdminHeartbeat12.h>
#include <cme_mdp/ChannelReset4.h>

#include "roq/cme/sbe/utils.hpp"

namespace roq {
namespace cme {
namespace sbe {}  // namespace sbe
}  // namespace cme
}  // namespace roq

// messages

// ChannelReset4

template <>
struct fmt::formatter<cme_mdp::ChannelReset4::NoMDEntries> {
  using value_type = cme_mdp::ChannelReset4::NoMDEntries;
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
        R"(md_update_action={}, )"
        R"(md_entry_type={}, )"
        R"(appl_id={})"
        R"(}})"sv,
        value.mDUpdateAction(),
        value.mDEntryType(),
        value.applID());
  }
};

template <>
struct fmt::formatter<cme_mdp::ChannelReset4> {
  using value_type = cme_mdp::ChannelReset4;
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type const &value, Context &context) const {
    using namespace std::literals;
    const_cast<value_type &>(value).sbeRewind();  // note!
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(transact_time={}, )"
        R"(match_event_indicator={}, )"
        R"(no_md_entries=[{}])"
        R"(}})"sv,
        value.transactTime(),
        const_cast<value_type &>(value).matchEventIndicator(),
        roq::cme::sbe::Group{const_cast<value_type &>(value).noMDEntries()});
  }
};

// AdminHeartbeat12

template <>
struct fmt::formatter<cme_mdp::AdminHeartbeat12> {
  using value_type = cme_mdp::AdminHeartbeat12;
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type const &, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(}})"sv);
  }
};
