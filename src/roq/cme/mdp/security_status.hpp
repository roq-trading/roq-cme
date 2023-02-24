/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <cme_mdp3/SecurityStatus30.h>

#include "roq/cme/mdp/utils.hpp"

namespace roq {
namespace cme {
namespace mdp {}  // namespace mdp
}  // namespace cme
}  // namespace roq

// messages

// SecurityStatus30

template <>
struct fmt::formatter<cme_mdp3::SecurityStatus30> {
  using value_type = cme_mdp3::SecurityStatus30;
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type &value, Context &context) const {
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
