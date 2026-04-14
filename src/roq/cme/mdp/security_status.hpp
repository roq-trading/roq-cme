/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <cme/sbe/mdp/SecurityStatus30.h>

#include "roq/cme/mdp/utils.hpp"

namespace roq {
namespace cme {
namespace mdp {}  // namespace mdp
}  // namespace cme

template <>
inline constexpr std::string_view get_name<::cme::sbe::mdp::SecurityStatus30>() {
  using namespace std::literals;
  return "security_status_30"sv;
}
}  // namespace roq

// messages

// SecurityStatus30

template <>
struct fmt::formatter<::cme::sbe::mdp::SecurityStatus30> {
  using value_type = ::cme::sbe::mdp::SecurityStatus30;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type &value, format_context &context) const {
    using namespace std::literals;
    using namespace roq::cme::mdp;
    value.sbeRewind();  // note!
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(version={}, )"
        R"(transact_time={}, )"
        R"(security_group="{}", )"
        R"(asset="{}", )"
        R"(security_id={}, )"
        R"(trade_date={}, )"
        R"(match_event_indicator={}, )"
        R"(security_trading_status={}, )"
        R"(halt_reason={}, )"
        R"(security_trading_event={} )"
        R"(}})"sv,
        value.actingVersion(),
        value.transactTime(),
        get_string_view(value.securityGroup(), value.securityGroupLength()),
        get_string_view(value.asset(), value.assetLength()),
        value.securityID(),
        value.tradeDate(),
        value.matchEventIndicator(),
        value.securityTradingStatus(),
        value.haltReason(),
        value.securityTradingEvent());
  }
};
