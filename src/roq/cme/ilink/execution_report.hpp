/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <fmt/format.h>

#include <cme_ilink/ExecutionReportCancel534.h>
#include <cme_ilink/ExecutionReportModify531.h>
#include <cme_ilink/ExecutionReportNew522.h>
#include <cme_ilink/ExecutionReportPendingCancel564.h>
#include <cme_ilink/ExecutionReportPendingReplace565.h>
#include <cme_ilink/ExecutionReportReject523.h>
#include <cme_ilink/ExecutionReportStatus532.h>
#include <cme_ilink/ExecutionReportTradeOutright525.h>
#include <cme_ilink/ExecutionReportTradeSpread526.h>
#include <cme_ilink/ExecutionReportTradeSpreadLeg527.h>

#include "roq/core/sbe/iterator.hpp"

#include "roq/cme/ilink/utils.hpp"

namespace roq {
namespace cme {
namespace ilink {}  // namespace ilink
}  // namespace cme
}  // namespace roq

// messages

template <>
struct fmt::formatter<cme_ilink::ExecutionReportNew522> {
  using value_type = cme_ilink::ExecutionReportNew522;
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
        R"(exec_id="{}", )"
        R"(sender_id="{}", )"
        R"(cl_ord_id="{}", )"
        R"(party_details_list_req_id={}, )"
        R"(order_id={}, )"
        R"(price={}, )"
        R"(stop_px={}, )"
        R"(transact_time={}, )"
        R"(sending_time_epoch={}, )"
        R"(order_request_id={}, )"
        R"(cross_id={}, )"
        R"(host_cross_id={}, )"
        R"(location="{}", )"
        R"(security_id={}, )"
        R"(order_qty={}, )"
        R"(min_qty={}, )"
        R"(display_qty={}, )"
        R"(expire_date={}, )"
        R"(delay_duration={}, )"
        R"(ord_status={}, )"
        R"(exec_type={}, )"
        R"(ord_type={}, )"
        R"(side={}, )"
        R"(time_in_force={}, )"
        R"(manual_order_indicator={}, )"
        R"(poss_retrans_flag={}, )"
        R"(split_msg={}, )"
        R"(cross_type={}, )"
        R"(exec_inst={}, )"
        R"(execution_mode={}, )"
        R"(liquidity_flag={}, )"
        R"(managed_order={}, )"
        R"(short_sale_type={}, )"
        R"(delay_to_time={}, )"
        R"(discretion_price={})"
        R"(}})"sv,
        value.seqNum(),
        value.uUID(),
        value.execID(),
        value.senderID(),
        value.clOrdID(),
        value.partyDetailsListReqID(),
        value.orderID(),
        value.price(),
        value.stopPx(),
        value.transactTime(),
        value.sendingTimeEpoch(),
        value.orderRequestID(),
        value.crossID(),
        value.hostCrossID(),
        value.location(),
        value.securityID(),
        value.orderQty(),
        value.minQty(),
        value.displayQty(),
        value.expireDate(),
        value.delayDuration(),
        value.ordStatus(),
        value.execType(),
        value.ordType(),
        value.side(),
        value.timeInForce(),
        value.manualOrderIndicator(),
        value.possRetransFlag(),
        value.splitMsg(),
        value.crossType(),
        value.execInst(),
        value.executionMode(),
        value.liquidityFlag(),
        value.managedOrder(),
        value.shortSaleType(),
        value.delayToTime(),
        value.discretionPrice());
  }
};

template <>
struct fmt::formatter<cme_ilink::ExecutionReportReject523> {
  using value_type = cme_ilink::ExecutionReportReject523;
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
        R"(exec_id="{}", )"
        R"(sender_id="{}", )"
        R"(cl_ord_id="{}", )"
        R"(party_details_list_req_id={}, )"
        R"(order_id={}, )"
        R"(price={}, )"
        R"(stop_px={}, )"
        R"(transact_time={}, )"
        R"(sending_time_epoch={}, )"
        R"(order_request_id={}, )"
        R"(cross_id={}, )"
        R"(host_cross_id={}, )"
        R"(location="{}", )"
        R"(security_id={}, )"
        R"(order_qty={}, )"
        R"(min_qty={}, )"
        R"(display_qty={}, )"
        R"(ord_rej_reason={}, )"
        R"(expire_date={}, )"
        R"(delay_duration={}, )"
        R"(ord_status={}, )"
        R"(exec_type={}, )"
        R"(ord_type={}, )"
        R"(side={}, )"
        R"(time_in_force={}, )"
        R"(manual_order_indicator={}, )"
        R"(poss_retrans_flag={}, )"
        R"(split_msg={}, )"
        R"(cross_type={}, )"
        R"(exec_inst={}, )"
        R"(execution_mode={}, )"
        R"(liquidity_flag={}, )"
        R"(managed_order={}, )"
        R"(short_sale_type={}, )"
        R"(delay_to_time={}, )"
        R"(discretion_price={})"
        R"(}})"sv,
        value.seqNum(),
        value.uUID(),
        value.text(),
        value.execID(),
        value.senderID(),
        value.clOrdID(),
        value.partyDetailsListReqID(),
        value.orderID(),
        value.price(),
        value.stopPx(),
        value.transactTime(),
        value.sendingTimeEpoch(),
        value.orderRequestID(),
        value.crossID(),
        value.hostCrossID(),
        value.location(),
        value.securityID(),
        value.orderQty(),
        value.minQty(),
        value.displayQty(),
        value.ordRejReason(),
        value.expireDate(),
        value.delayDuration(),
        value.ordStatus(),
        value.execType(),
        value.ordType(),
        value.side(),
        value.timeInForce(),
        value.manualOrderIndicator(),
        value.possRetransFlag(),
        value.splitMsg(),
        value.crossType(),
        value.execInst(),
        value.executionMode(),
        value.liquidityFlag(),
        value.managedOrder(),
        value.shortSaleType(),
        value.delayToTime(),
        value.discretionPrice());
  }
};

template <>
struct fmt::formatter<cme_ilink::ExecutionReportModify531> {
  using value_type = cme_ilink::ExecutionReportModify531;
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
        R"(exec_id="{}", )"
        R"(sender_id="{}", )"
        R"(cl_ord_id="{}", )"
        R"(party_details_list_req_id={}, )"
        R"(order_id={}, )"
        R"(price={}, )"
        R"(stop_px={}, )"
        R"(transact_time={}, )"
        R"(sending_time_epoch={}, )"
        R"(order_request_id={}, )"
        R"(cross_id={}, )"
        R"(host_cross_id={}, )"
        R"(location="{}", )"
        R"(security_id={}, )"
        R"(order_qty={}, )"
        R"(cum_qty={}, )"
        R"(leaves_qty={}, )"
        R"(min_qty={}, )"
        R"(display_qty={}, )"
        R"(expire_date={}, )"
        R"(delay_duration={}, )"
        R"(ord_status={}, )"
        R"(exec_type={}, )"
        R"(ord_type={}, )"
        R"(side={}, )"
        R"(time_in_force={}, )"
        R"(manual_order_indicator={}, )"
        R"(poss_retrans_flag={}, )"
        R"(split_msg={}, )"
        R"(cross_type={}, )"
        R"(exec_inst={}, )"
        R"(execution_mode={}, )"
        R"(liquidity_flag={}, )"
        R"(managed_order={}, )"
        R"(short_sale_type={}, )"
        R"(delay_to_time={}, )"
        R"(discretion_price={})"
        R"(}})"sv,
        value.seqNum(),
        value.uUID(),
        value.execID(),
        value.senderID(),
        value.clOrdID(),
        value.partyDetailsListReqID(),
        value.orderID(),
        value.price(),
        value.stopPx(),
        value.transactTime(),
        value.sendingTimeEpoch(),
        value.orderRequestID(),
        value.crossID(),
        value.hostCrossID(),
        value.location(),
        value.securityID(),
        value.orderQty(),
        value.cumQty(),
        value.leavesQty(),
        value.minQty(),
        value.displayQty(),
        value.expireDate(),
        value.delayDuration(),
        value.ordStatus(),
        value.execType(),
        value.ordType(),
        value.side(),
        value.timeInForce(),
        value.manualOrderIndicator(),
        value.possRetransFlag(),
        value.splitMsg(),
        value.crossType(),
        value.execInst(),
        value.executionMode(),
        value.liquidityFlag(),
        value.managedOrder(),
        value.shortSaleType(),
        value.delayToTime(),
        value.discretionPrice());
  }
};

template <>
struct fmt::formatter<cme_ilink::ExecutionReportStatus532> {
  using value_type = cme_ilink::ExecutionReportStatus532;
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
        R"(exec_id="{}", )"
        R"(sender_id="{}", )"
        R"(cl_ord_id="{}", )"
        R"(party_details_list_req_id={}, )"
        R"(order_id={}, )"
        R"(price={}, )"
        R"(stop_px={}, )"
        R"(transact_time={}, )"
        R"(sending_time_epoch={}, )"
        R"(order_request_id={}, )"
        R"(ord_status_req_id={}, )"
        R"(mass_status_req_id={}, )"
        R"(cross_id={}, )"
        R"(host_cross_id={}, )"
        R"(location="{}", )"
        R"(security_id={}, )"
        R"(order_qty={}, )"
        R"(min_qty={}, )"
        R"(display_qty={}, )"
        R"(expire_date={}, )"
        R"(ord_status={}, )"
        R"(exec_type={}, )"
        R"(ord_type={}, )"
        R"(side={}, )"
        R"(time_in_force={}, )"
        R"(manual_order_indicator={}, )"
        R"(poss_retrans_flag={}, )"
        R"(last_rpt_requested={}, )"
        R"(cross_type={}, )"
        R"(exec_inst={}, )"
        R"(execution_mode={}, )"
        R"(liquidity_flag={}, )"
        R"(managed_order={}, )"
        R"(short_sale_type={}, )"
        R"(discretion_price={})"
        R"(}})"sv,
        value.seqNum(),
        value.uUID(),
        value.text(),
        value.execID(),
        value.senderID(),
        value.clOrdID(),
        value.partyDetailsListReqID(),
        value.orderID(),
        value.price(),
        value.stopPx(),
        value.transactTime(),
        value.sendingTimeEpoch(),
        value.orderRequestID(),
        value.ordStatusReqID(),
        value.massStatusReqID(),
        value.crossID(),
        value.hostCrossID(),
        value.location(),
        value.securityID(),
        value.orderQty(),
        value.minQty(),
        value.displayQty(),
        value.expireDate(),
        value.ordStatus(),
        value.execType(),
        value.ordType(),
        value.side(),
        value.timeInForce(),
        value.manualOrderIndicator(),
        value.possRetransFlag(),
        value.lastRptRequested(),
        value.crossType(),
        value.execInst(),
        value.executionMode(),
        value.liquidityFlag(),
        value.managedOrder(),
        value.shortSaleType(),
        value.discretionPrice());
  }
};

template <>
struct fmt::formatter<cme_ilink::ExecutionReportCancel534> {
  using value_type = cme_ilink::ExecutionReportCancel534;
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
        R"(exec_id="{}", )"
        R"(sender_id="{}", )"
        R"(cl_ord_id="{}", )"
        R"(party_details_list_req_id={}, )"
        R"(order_id={}, )"
        R"(price={}, )"
        R"(stop_px={}, )"
        R"(transact_time={}, )"
        R"(sending_time_epoch={}, )"
        R"(order_request_id={}, )"
        R"(cross_id={}, )"
        R"(host_cross_id={}, )"
        R"(location="{}", )"
        R"(security_id={}, )"
        R"(order_qty={}, )"
        R"(cum_qty={}, )"
        R"(min_qty={}, )"
        R"(display_qty={}, )"
        R"(expire_date={}, )"
        R"(delay_duration={}, )"
        R"(ord_status={}, )"
        R"(exec_type={}, )"
        R"(ord_type={}, )"
        R"(side={}, )"
        R"(time_in_force={}, )"
        R"(manual_order_indicator={}, )"
        R"(poss_retrans_flag={}, )"
        R"(split_msg={}, )"
        R"(exec_restatement_reason={}, )"
        R"(cross_type={}, )"
        R"(exec_inst={}, )"
        R"(execution_mode={}, )"
        R"(liquidity_flag={}, )"
        R"(managed_order={}, )"
        R"(short_sale_type={}, )"
        R"(delay_to_time={}, )"
        R"(discretion_price={})"
        R"(}})"sv,
        value.seqNum(),
        value.uUID(),
        value.execID(),
        value.senderID(),
        value.clOrdID(),
        value.partyDetailsListReqID(),
        value.orderID(),
        value.price(),
        value.stopPx(),
        value.transactTime(),
        value.sendingTimeEpoch(),
        value.orderRequestID(),
        value.crossID(),
        value.hostCrossID(),
        value.location(),
        value.securityID(),
        value.orderQty(),
        value.cumQty(),
        value.minQty(),
        value.displayQty(),
        value.expireDate(),
        value.delayDuration(),
        value.ordStatus(),
        value.execType(),
        value.ordType(),
        value.side(),
        value.timeInForce(),
        value.manualOrderIndicator(),
        value.possRetransFlag(),
        value.splitMsg(),
        value.execRestatementReason(),
        value.crossType(),
        value.execInst(),
        value.executionMode(),
        value.liquidityFlag(),
        value.managedOrder(),
        value.shortSaleType(),
        value.delayToTime(),
        value.discretionPrice());
  }
};

template <>
struct fmt::formatter<cme_ilink::ExecutionReportPendingCancel564> {
  using value_type = cme_ilink::ExecutionReportPendingCancel564;
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
        R"(exec_id="{}", )"
        R"(sender_id="{}", )"
        R"(cl_ord_id="{}", )"
        R"(party_details_list_req_id={}, )"
        R"(order_id={}, )"
        R"(price={}, )"
        R"(transact_time={}, )"
        R"(sending_time_epoch={}, )"
        R"(order_request_id={}, )"
        R"(location="{}", )"
        R"(security_id={}, )"
        R"(order_qty={}, )"
        R"(cum_qty={}, )"
        R"(leaves_qty={}, )"
        R"(min_qty={}, )"
        R"(display_qty={}, )"
        R"(expire_date={}, )"
        R"(ord_status={}, )"
        R"(exec_type={}, )"
        R"(ord_type={}, )"
        R"(side={}, )"
        R"(time_in_force={}, )"
        R"(manual_order_indicator={}, )"
        R"(poss_retrans_flag={}, )"
        R"(split_msg={}, )"
        R"(liquidity_flag={}, )"
        R"(delay_to_time={}, )"
        R"(discretion_price={})"
        R"(}})"sv,
        value.seqNum(),
        value.uUID(),
        value.execID(),
        value.senderID(),
        value.clOrdID(),
        value.partyDetailsListReqID(),
        value.orderID(),
        value.price(),
        value.transactTime(),
        value.sendingTimeEpoch(),
        value.orderRequestID(),
        value.location(),
        value.securityID(),
        value.orderQty(),
        value.cumQty(),
        value.leavesQty(),
        value.minQty(),
        value.displayQty(),
        value.expireDate(),
        value.ordStatus(),
        value.execType(),
        value.ordType(),
        value.side(),
        value.timeInForce(),
        value.manualOrderIndicator(),
        value.possRetransFlag(),
        value.splitMsg(),
        value.liquidityFlag(),
        value.delayToTime(),
        value.discretionPrice());
  }
};

template <>
struct fmt::formatter<cme_ilink::ExecutionReportPendingReplace565> {
  using value_type = cme_ilink::ExecutionReportPendingReplace565;
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
        R"(exec_id="{}", )"
        R"(sender_id="{}", )"
        R"(cl_ord_id="{}", )"
        R"(party_details_list_req_id={}, )"
        R"(order_id={}, )"
        R"(price={}, )"
        R"(transact_time={}, )"
        R"(sending_time_epoch={}, )"
        R"(order_request_id={}, )"
        R"(location="{}", )"
        R"(security_id={}, )"
        R"(order_qty={}, )"
        R"(cum_qty={}, )"
        R"(leaves_qty={}, )"
        R"(min_qty={}, )"
        R"(display_qty={}, )"
        R"(expire_date={}, )"
        R"(ord_status={}, )"
        R"(exec_type={}, )"
        R"(ord_type={}, )"
        R"(side={}, )"
        R"(time_in_force={}, )"
        R"(manual_order_indicator={}, )"
        R"(poss_retrans_flag={}, )"
        R"(split_msg={}, )"
        R"(liquidity_flag={}, )"
        R"(short_sale_type={}, )"
        R"(delay_to_time={}, )"
        R"(discretion_price={})"
        R"(}})"sv,
        value.seqNum(),
        value.uUID(),
        value.execID(),
        value.senderID(),
        value.clOrdID(),
        value.partyDetailsListReqID(),
        value.orderID(),
        value.price(),
        value.transactTime(),
        value.sendingTimeEpoch(),
        value.orderRequestID(),
        value.location(),
        value.securityID(),
        value.orderQty(),
        value.cumQty(),
        value.leavesQty(),
        value.minQty(),
        value.displayQty(),
        value.expireDate(),
        value.ordStatus(),
        value.execType(),
        value.ordType(),
        value.side(),
        value.timeInForce(),
        value.manualOrderIndicator(),
        value.possRetransFlag(),
        value.splitMsg(),
        value.liquidityFlag(),
        value.shortSaleType(),
        value.delayToTime(),
        value.discretionPrice());
  }
};

template <>
struct fmt::formatter<cme_ilink::ExecutionReportTradeOutright525> {
  using value_type = cme_ilink::ExecutionReportTradeOutright525;
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
        R"(exec_id="{}", )"
        R"(sender_id="{}", )"
        R"(cl_ord_id="{}", )"
        R"(party_details_list_req_id={}, )"
        R"(last_px={}, )"
        R"(order_id={}, )"
        R"(price={}, )"
        R"(stop_px={}, )"
        R"(transact_time={}, )"
        R"(sending_time_epoch={}, )"
        R"(order_request_id={}, )"
        R"(sec_exec_id={}, )"
        R"(cross_id={}, )"
        R"(host_cross_id={}, )"
        R"(location="{}", )"
        R"(security_id={}, )"
        R"(order_qty={}, )"
        R"(last_qty={}, )"
        R"(cum_qty={}, )"
        R"(md_trade_entry_id={}, )"
        R"(side_trade_id={}, )"
        R"(trade_link_id={}, )"
        R"(leaves_qty={}, )"
        R"(trade_date={}, )"
        R"(expire_date={}, )"
        R"(ord_status={}, )"
        R"(exec_type={}, )"
        R"(ord_type={}, )"
        R"(side={}, )"
        R"(time_in_force={}, )"
        R"(manual_order_indicator={}, )"
        R"(poss_retrans_flag={}, )"
        R"(aggressor_indicator={}, )"
        R"(cross_type={}, )"
        R"(exec_inst={}, )"
        R"(execution_mode={}, )"
        R"(liquidity_flag={}, )"
        R"(managed_order={}, )"
        R"(short_sale_type={}, )"
        R"(ownership={}, )"
        R"(discretion_price={}, )"
        R"(trd_type={}, )"
        R"(exec_restatement_reason={}, )"
        R"(settl_date={}, )"
        R"(maturity_date={}, )"
        R"(calculated_ccy_last_qty={}, )"
        R"(gross_trade_amt={}, )"
        R"(benchmark_price={}, )"
        R"(no_fills=[...], )"
        R"(no_order_events=[...])"
        R"(}})"sv,
        value.seqNum(),
        value.uUID(),
        value.execID(),
        value.senderID(),
        value.clOrdID(),
        value.partyDetailsListReqID(),
        value.lastPx(),
        value.orderID(),
        value.price(),
        value.stopPx(),
        value.transactTime(),
        value.sendingTimeEpoch(),
        value.orderRequestID(),
        value.secExecID(),
        value.crossID(),
        value.hostCrossID(),
        value.location(),
        value.securityID(),
        value.orderQty(),
        value.lastQty(),
        value.cumQty(),
        value.mDTradeEntryID(),
        value.sideTradeID(),
        value.tradeLinkID(),
        value.leavesQty(),
        value.tradeDate(),
        value.expireDate(),
        value.ordStatus(),
        value.execType(),
        value.ordType(),
        value.side(),
        value.timeInForce(),
        value.manualOrderIndicator(),
        value.possRetransFlag(),
        value.aggressorIndicator(),
        value.crossType(),
        value.execInst(),
        value.executionMode(),
        value.liquidityFlag(),
        value.managedOrder(),
        value.shortSaleType(),
        value.ownership(),
        value.discretionPrice(),
        value.trdType(),
        value.execRestatementReason(),
        value.settlDate(),
        value.maturityDate(),
        value.calculatedCcyLastQty(),
        value.grossTradeAmt(),
        value.benchmarkPrice());
    // + noFills()
    // + noOrderEvents()
  }
};

template <>
struct fmt::formatter<cme_ilink::ExecutionReportTradeSpread526> {
  using value_type = cme_ilink::ExecutionReportTradeSpread526;
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
        R"(exec_id="{}", )"
        R"(sender_id="{}", )"
        R"(cl_ord_id="{}", )"
        R"(party_details_list_req_id={}, )"
        R"(last_px={}, )"
        R"(order_id={}, )"
        R"(price={}, )"
        R"(stop_px={}, )"
        R"(transact_time={}, )"
        R"(sending_time_epoch={}, )"
        R"(order_request_id={}, )"
        R"(sec_exec_id={}, )"
        R"(cross_id={}, )"
        R"(host_cross_id={}, )"
        R"(location="{}", )"
        R"(security_id={}, )"
        R"(order_qty={}, )"
        R"(last_qty={}, )"
        R"(cum_qty={}, )"
        R"(md_trade_entry_id={}, )"
        R"(side_trade_id={}, )"
        R"(leaves_qty={}, )"
        R"(trade_date={}, )"
        R"(expire_date={}, )"
        R"(ord_status={}, )"
        R"(exec_type={}, )"
        R"(ord_type={}, )"
        R"(side={}, )"
        R"(time_in_force={}, )"
        R"(manual_order_indicator={}, )"
        R"(poss_retrans_flag={}, )"
        R"(aggressor_indicator={}, )"
        R"(cross_type={}, )"
        R"(total_num_securities={}, )"
        R"(exec_inst={}, )"
        R"(execution_mode={}, )"
        R"(liquidity_flag={}, )"
        R"(short_sale_type={}, )"
        R"(no_fills=[...], )"
        R"(no_legs=[...], )"
        R"(no_order_events=[...])"
        R"(}})"sv,
        value.seqNum(),
        value.uUID(),
        value.execID(),
        value.senderID(),
        value.clOrdID(),
        value.partyDetailsListReqID(),
        value.lastPx(),
        value.orderID(),
        value.price(),
        value.stopPx(),
        value.transactTime(),
        value.sendingTimeEpoch(),
        value.orderRequestID(),
        value.secExecID(),
        value.crossID(),
        value.hostCrossID(),
        value.location(),
        value.securityID(),
        value.orderQty(),
        value.lastQty(),
        value.cumQty(),
        value.mDTradeEntryID(),
        value.sideTradeID(),
        value.leavesQty(),
        value.tradeDate(),
        value.expireDate(),
        value.ordStatus(),
        value.execType(),
        value.ordType(),
        value.side(),
        value.timeInForce(),
        value.manualOrderIndicator(),
        value.possRetransFlag(),
        value.aggressorIndicator(),
        value.crossType(),
        value.totalNumSecurities(),
        value.execInst(),
        value.executionMode(),
        value.liquidityFlag(),
        value.shortSaleType());
    // + noFills()
    // + noLegs()
    // + noOrderEvents()
  }
};

template <>
struct fmt::formatter<cme_ilink::ExecutionReportTradeSpreadLeg527> {
  using value_type = cme_ilink::ExecutionReportTradeSpreadLeg527;
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
        R"(exec_id="{}", )"
        R"(sender_id="{}", )"
        R"(cl_ord_id="{}", )"
        R"(volatility="{}", )"
        R"(party_details_list_req_id={}, )"
        R"(last_px={}, )"
        R"(order_id={}, )"
        R"(underlying_px={}, )"
        R"(transact_time={}, )"
        R"(sending_time_epoch={}, )"
        R"(sec_exec_id={}, )"
        R"(location="{}", )"
        R"(option_delta="{}", )"
        R"(time_to_expiration="{}", )"
        R"(risk_free_rate="{}", )"
        R"(security_id={}, )"
        R"(last_qty={}, )"
        R"(cum_qty={}, )"
        R"(side_trade_id={}, )"
        R"(trade_date={}, )"
        R"(ord_status={}, )"
        R"(exec_type={}, )"
        R"(ord_type={}, )"
        R"(side={}, )"
        R"(poss_retrans_flag={}, )"
        R"(settl_date={}, )"
        R"(calculated_ccy_last_qty={}, )"
        R"(gross_trade_amt={}, )"
        R"(no_fills=[...], )"
        R"(no_order_events=[...])"
        R"(}})"sv,
        value.seqNum(),
        value.uUID(),
        value.execID(),
        value.senderID(),
        value.clOrdID(),
        value.volatility(),
        value.partyDetailsListReqID(),
        value.lastPx(),
        value.orderID(),
        value.underlyingPx(),
        value.transactTime(),
        value.sendingTimeEpoch(),
        value.secExecID(),
        value.location(),
        value.optionDelta(),
        value.timeToExpiration(),
        value.riskFreeRate(),
        value.securityID(),
        value.lastQty(),
        value.cumQty(),
        value.sideTradeID(),
        value.tradeDate(),
        value.ordStatus(),
        value.execType(),
        value.ordType(),
        value.side(),
        value.possRetransFlag(),
        value.settlDate(),
        value.calculatedCcyLastQty(),
        value.grossTradeAmt());
    // + noFills()
    // + noOrderEvents()
  }
};
