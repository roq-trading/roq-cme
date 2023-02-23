/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <fmt/format.h>

#include <cme_ilink3/OrderCancelReject535.h>
#include <cme_ilink3/OrderCancelReplaceReject536.h>

#include "roq/core/sbe/iterator.hpp"

#include "roq/cme/ilink3/utils.hpp"

namespace roq {
namespace cme {
namespace ilink3 {}  // namespace ilink3
}  // namespace cme
}  // namespace roq

// messages

template <>
struct fmt::formatter<cme_ilink3::OrderCancelReject535> {
  using value_type = cme_ilink3::OrderCancelReject535;
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type &value, Context &context) const {
    using namespace std::literals;
    using namespace roq::cme::ilink3;
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
struct fmt::formatter<cme_ilink3::OrderCancelReplaceReject536> {
  using value_type = cme_ilink3::OrderCancelReplaceReject536;
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type &value, Context &context) const {
    using namespace std::literals;
    using namespace roq::cme::ilink3;
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
