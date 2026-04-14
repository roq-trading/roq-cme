/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <fmt/format.h>

#include <cme/sbe/ilink/OrderCancelReject535.h>
#include <cme/sbe/ilink/OrderCancelReplaceReject536.h>

#include "roq/core/sbe/iterator.hpp"

#include "roq/cme/ilink/utils.hpp"

namespace roq {
namespace cme {
namespace ilink {}  // namespace ilink
}  // namespace cme
}  // namespace roq

// messages

template <>
struct fmt::formatter<::cme::sbe::ilink::OrderCancelReject535> {
  using value_type = ::cme::sbe::ilink::OrderCancelReject535;
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
        R"(text="{}", )"
        R"(exec_id={}, )"
        R"(sender_id={}, )"
        R"(cl_ord_id={}, )"
        R"(party_details_list_req_id={}, )"
        R"(ord_status={}, )"
        R"(cxl_rej_response_to={}, )"
        R"(order_id={}, )"
        R"(transact_time={}, )"
        R"(sending_time_epoch={}, )"
        R"(order_request_id={}, )"
        R"(location={}, )"
        R"(cxl_rej_reason={}, )"
        R"(delay_duration={}, )"
        R"(manual_order_indicator={}, )"
        R"(poss_retrans_flag={}, )"
        R"(split_msg={}, )"
        R"(liquidity_flag={}, )"
        R"(delay_to_time={})"
        R"(}})"sv,
        value.seqNum(),
        value.uUID(),
        value.text(),
        value.execID(),
        value.senderID(),
        value.clOrdID(),
        value.partyDetailsListReqID(),
        value.ordStatus(),
        value.cxlRejResponseTo(),
        value.orderID(),
        value.transactTime(),
        value.sendingTimeEpoch(),
        value.orderRequestID(),
        value.location(),
        value.cxlRejReason(),
        value.delayDuration(),
        value.manualOrderIndicator(),
        value.possRetransFlag(),
        value.splitMsg(),
        value.liquidityFlag(),
        value.delayToTime());
  }
};

template <>
struct fmt::formatter<::cme::sbe::ilink::OrderCancelReplaceReject536> {
  using value_type = ::cme::sbe::ilink::OrderCancelReplaceReject536;
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
        R"(text="{}", )"
        R"(exec_id={}, )"
        R"(sender_id={}, )"
        R"(cl_ord_id={}, )"
        R"(party_details_list_req_id={}, )"
        R"(ord_status={}, )"
        R"(cxl_rej_response_to={}, )"
        R"(order_id={}, )"
        R"(transact_time={}, )"
        R"(sending_time_epoch={}, )"
        R"(order_request_id={}, )"
        R"(location={}, )"
        R"(cxl_rej_reason={}, )"
        R"(delay_duration={}, )"
        R"(manual_order_indicator={}, )"
        R"(poss_retrans_flag={}, )"
        R"(split_msg={}, )"
        R"(liquidity_flag={}, )"
        R"(delay_to_time={})"
        R"(}})"sv,
        value.seqNum(),
        value.uUID(),
        value.text(),
        value.execID(),
        value.senderID(),
        value.clOrdID(),
        value.partyDetailsListReqID(),
        value.ordStatus(),
        value.cxlRejResponseTo(),
        value.orderID(),
        value.transactTime(),
        value.sendingTimeEpoch(),
        value.orderRequestID(),
        value.location(),
        value.cxlRejReason(),
        value.delayDuration(),
        value.manualOrderIndicator(),
        value.possRetransFlag(),
        value.splitMsg(),
        value.liquidityFlag(),
        value.delayToTime());
  }
};
