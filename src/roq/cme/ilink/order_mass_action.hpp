/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include <fmt/format.h>

#include <cme_ilink/OrderMassActionReport562.h>

#include "roq/core/sbe/iterator.hpp"

#include "roq/cme/ilink/utils.hpp"

namespace roq {
namespace cme {
namespace ilink {}  // namespace ilink
}  // namespace cme
}  // namespace roq

// messages

template <>
struct fmt::formatter<cme_ilink::OrderMassActionReport562> {
  using value_type = cme_ilink::OrderMassActionReport562;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type &value, format_context &context) const {
    using namespace std::literals;
    using namespace roq::cme::ilink;
    value.sbeRewind();  // note!
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(seq_num={}, )"
        R"(uuid={}, )"
        R"(sender_id="{}", )"
        R"(party_details_list_req_id={}, )"
        R"(transact_time={}, )"
        R"(sending_time_epoch={}, )"
        R"(order_request_id={}, )"
        R"(mass_action_report_id={}, )"
        R"(mass_action_type={}, )"
        R"(security_group="{}", )"
        R"(location="{}", )"
        R"(security_id={}, )"
        R"(delay_duration={}, )"
        R"(mass_action_response={}, )"
        R"(manual_order_indicator={}, )"
        R"(mass_action_scope={}, )"
        R"(total_affected_orders={}, )"
        R"(last_fragment={}, )"
        R"(mass_action_reject_reason={}, )"
        R"(market_segment_id={}, )"
        R"(mass_cancel_request_type={}, )"
        R"(side={}, )"
        R"(ord_type={}, )"
        R"(time_in_force={}, )"
        R"(split_msg={}, )"
        R"(liquidity_flag={}, )"
        R"(poss_retrans_flag={}, )"
        R"(delay_to_time={}, )"
        R"(orig_order_user="{}", )"
        R"(cancel_text="{}", )"
        // R"(no_affected_orders=[{}])"
        R"(}})"sv,
        value.seqNum(),
        value.uUID(),
        value.senderID(),
        value.partyDetailsListReqID(),
        value.transactTime(),
        value.sendingTimeEpoch(),
        value.orderRequestID(),
        value.massActionReportID(),
        value.massActionType(),
        value.securityGroup(),
        value.location(),
        value.securityID(),
        value.delayDuration(),
        magic_enum::enum_name(value.massActionResponse()),
        magic_enum::enum_name(value.manualOrderIndicator()),
        magic_enum::enum_name(value.massActionScope()),
        value.totalAffectedOrders(),
        value.lastFragment(),
        value.massActionRejectReason(),
        value.marketSegmentID(),
        magic_enum::enum_name(value.massCancelRequestType()),
        magic_enum::enum_name(value.side()),
        magic_enum::enum_name(value.ordType()),
        magic_enum::enum_name(value.timeInForce()),
        magic_enum::enum_name(value.splitMsg()),
        magic_enum::enum_name(value.liquidityFlag()),
        magic_enum::enum_name(value.possRetransFlag()),
        value.delayToTime(),
        value.origOrderUser(),
        value.cancelText());
  }
};
