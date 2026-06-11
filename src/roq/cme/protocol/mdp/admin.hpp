/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <fmt/format.h>

#include <cme/sbe/mdp/AdminHeartbeat12.h>
#include <cme/sbe/mdp/ChannelReset4.h>

#include "roq/cme/protocol/mdp/utils.hpp"

namespace roq {
namespace cme {
namespace protocol {
namespace mdp {}
}  // namespace protocol
}  // namespace cme

template <>
inline constexpr std::string_view get_name<::cme::sbe::mdp::ChannelReset4>() {
  using namespace std::literals;
  return "channel_reset_4"sv;
}

template <>
inline constexpr std::string_view get_name<::cme::sbe::mdp::AdminHeartbeat12>() {
  using namespace std::literals;
  return "admin_heartbeat_12"sv;
}
}  // namespace roq

// messages

// ChannelReset4

template <>
struct fmt::formatter<::cme::sbe::mdp::ChannelReset4::NoMDEntries> {
  using value_type = ::cme::sbe::mdp::ChannelReset4::NoMDEntries;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type const &value, format_context &context) const {
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
struct fmt::formatter<::cme::sbe::mdp::ChannelReset4> {
  using value_type = ::cme::sbe::mdp::ChannelReset4;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type const &value, format_context &context) const {
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
        roq::cme::protocol::mdp::Group{const_cast<value_type &>(value).noMDEntries()});
  }
};

// AdminHeartbeat12

template <>
struct fmt::formatter<::cme::sbe::mdp::AdminHeartbeat12> {
  using value_type = ::cme::sbe::mdp::AdminHeartbeat12;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type const &, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(}})"sv);
  }
};
