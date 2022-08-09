/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <cme_mdp/SnapshotFullRefresh52.h>
#include <cme_mdp/SnapshotFullRefreshLongQty69.h>
#include <cme_mdp/SnapshotFullRefreshOrderBook53.h>
#include <cme_mdp/SnapshotFullRefreshTCP61.h>
#include <cme_mdp/SnapshotFullRefreshTCPLongQty68.h>

#include "roq/core/sbe/iterator.hpp"

#include "roq/cme/sbe/utils.hpp"

namespace roq {
namespace cme {
namespace sbe {

template <>
inline size_t compute_length(cme_mdp::SnapshotFullRefresh52 &value) {
  // NoMDEntries
  auto no_md_entries_length = value.noMDEntries().count();
  value.noMDEntries().forEach([](auto &e) { e.skip(); });
  return value.computeLength(no_md_entries_length);
}

}  // namespace sbe
}  // namespace cme
}  // namespace roq

// SnapshotFullRefresh52

template <>
struct fmt::formatter<cme_mdp::SnapshotFullRefresh52::NoMDEntries> {
  using value_type = cme_mdp::SnapshotFullRefresh52::NoMDEntries;
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
        R"(number_of_orders={}, )"
        R"(md_price_level={}, )"
        R"(trading_reference_date={}, )"
        R"(open_close_settl_flag={}, )"
        R"(settl_price_type={}, )"
        R"(md_entry_type={})"
        R"(}})"sv,
        const_cast<value_type &>(value).mDEntryPx(),
        value.mDEntrySize(),
        value.numberOfOrders(),
        value.mDPriceLevel(),
        value.tradingReferenceDate(),
        value.openCloseSettlFlag(),
        const_cast<value_type &>(value).settlPriceType(),
        value.mDEntryType());
  }
};

template <>
struct fmt::formatter<cme_mdp::SnapshotFullRefresh52> {
  using value_type = cme_mdp::SnapshotFullRefresh52;
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
        R"(version={}, )"
        R"(last_msg_seq_num_processed={}, )"
        R"(tot_num_reports={}, )"
        R"(security_id={}, )"
        R"(rpt_seq={}, )"
        R"(transact_time={}, )"
        R"(last_update_time={}, )"
        R"(trade_date={}, )"
        R"(md_security_trading_status={}, )"
        R"(high_limit_price={}, )"
        R"(low_limit_price={}, )"
        R"(max_price_variation={}, )"
        R"(no_md_entries=[{}])"
        R"(}})"sv,
        value.actingVersion(),
        value.lastMsgSeqNumProcessed(),
        value.totNumReports(),
        value.securityID(),
        value.rptSeq(),
        value.transactTime(),
        value.lastUpdateTime(),
        value.tradeDate(),
        value.mDSecurityTradingStatus(),
        value.highLimitPrice(),
        value.lowLimitPrice(),
        value.maxPriceVariation(),
        roq::cme::sbe::Group{value.noMDEntries()});
  }
};

// SnapshotFullRefreshOrderBook53

template <>
struct fmt::formatter<cme_mdp::SnapshotFullRefreshOrderBook53::NoMDEntries> {
  using value_type = cme_mdp::SnapshotFullRefreshOrderBook53::NoMDEntries;
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
        R"(md_entry_px={}, )"
        R"(md_display_qty={}, )"
        R"(md_entry_type={})"
        R"(}})"sv,
        value.orderID(),
        value.mDOrderPriority(),
        const_cast<value_type &>(value).mDEntryPx(),
        value.mDDisplayQty(),
        value.mDEntryType());
  }
};

template <>
struct fmt::formatter<cme_mdp::SnapshotFullRefreshOrderBook53> {
  using value_type = cme_mdp::SnapshotFullRefreshOrderBook53;
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
        R"(version={}, )"
        R"(last_msg_seq_num_processed={}, )"
        R"(tot_num_reports={}, )"
        R"(security_id={}, )"
        R"(no_chunks={}, )"
        R"(current_chunk={}, )"
        R"(transact_time={}, )"
        R"(no_md_entries=[{}])"
        R"(}})"sv,
        value.actingVersion(),
        value.lastMsgSeqNumProcessed(),
        value.totNumReports(),
        value.securityID(),
        value.noChunks(),
        value.currentChunk(),
        value.transactTime(),
        roq::cme::sbe::Group{value.noMDEntries()});
  }
};

// SnapshotFullRefreshLongQty69

template <>
struct fmt::formatter<cme_mdp::SnapshotFullRefreshLongQty69::NoMDEntries> {
  using value_type = cme_mdp::SnapshotFullRefreshLongQty69::NoMDEntries;
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
        R"(number_of_orderes={}, )"
        R"(md_price_level={}, )"
        R"(open_close_settl_flag={}, )"
        R"(md_entry_type={})"
        R"(}})"sv,
        const_cast<value_type &>(value).mDEntryPx(),
        value.mDEntrySize(),
        value.numberOfOrders(),
        value.mDPriceLevel(),
        value.openCloseSettlFlag(),
        value.mDEntryType());
  }
};

template <>
struct fmt::formatter<cme_mdp::SnapshotFullRefreshLongQty69> {
  using value_type = cme_mdp::SnapshotFullRefreshLongQty69;
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
        R"(version={}, )"
        R"(last_msg_seq_num_processed={}, )"
        R"(tot_num_reports={}, )"
        R"(security_id={}, )"
        R"(rpt_seq={}, )"
        R"(transact_time={}, )"
        R"(last_update_time={}, )"
        R"(trade_date={}, )"
        R"(md_security_trading_status={}, )"
        R"(high_limit_price={}, )"
        R"(low_limit_price={}, )"
        R"(max_price_variation={}, )"
        R"(no_md_entries=[{}])"
        R"(}})"sv,
        value.actingVersion(),
        value.lastMsgSeqNumProcessed(),
        value.totNumReports(),
        value.securityID(),
        value.rptSeq(),
        value.transactTime(),
        value.lastUpdateTime(),
        value.tradeDate(),
        value.mDSecurityTradingStatus(),
        value.highLimitPrice(),
        value.lowLimitPrice(),
        value.maxPriceVariation(),
        roq::cme::sbe::Group{value.noMDEntries()});
  }
};
