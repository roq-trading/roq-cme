/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <cme_mdp3/SnapshotFullRefresh52.h>
#include <cme_mdp3/SnapshotFullRefreshLongQty69.h>
#include <cme_mdp3/SnapshotFullRefreshOrderBook53.h>
#include <cme_mdp3/SnapshotFullRefreshTCP61.h>
#include <cme_mdp3/SnapshotFullRefreshTCPLongQty68.h>

#include "roq/core/sbe/iterator.hpp"

#include "roq/cme/mdp3/utils.hpp"

namespace roq {
namespace cme {
namespace mdp3 {

template <>
inline size_t compute_length(cme_mdp3::SnapshotFullRefresh52 &value) {
  // NoMDEntries
  auto no_md_entries_length = value.noMDEntries().count();
  value.noMDEntries().forEach([](auto &e) { e.skip(); });
  return value.computeLength(no_md_entries_length);
}

template <>
inline size_t compute_length(cme_mdp3::SnapshotFullRefreshOrderBook53 &value) {
  // NoMDEntries
  auto no_md_entries_length = value.noMDEntries().count();
  value.noMDEntries().forEach([](auto &e) { e.skip(); });
  return value.computeLength(no_md_entries_length);
}

}  // namespace mdp3
}  // namespace cme
}  // namespace roq

// SnapshotFullRefresh52

template <>
struct fmt::formatter<cme_mdp3::SnapshotFullRefresh52::NoMDEntries> {
  using value_type = cme_mdp3::SnapshotFullRefresh52::NoMDEntries;
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
        roq::cme::mdp3::get_double(const_cast<value_type &>(value).mDEntryPx()),
        roq::cme::mdp3::get_int(value.mDEntrySize(), value.mDEntrySizeNullValue()),
        roq::cme::mdp3::get_int(value.numberOfOrders(), value.numberOfOrdersNullValue()),
        roq::cme::mdp3::get_int(value.mDPriceLevel(), value.mDPriceLevelNullValue()),
        value.tradingReferenceDate(),
        value.openCloseSettlFlag(),
        const_cast<value_type &>(value).settlPriceType(),
        value.mDEntryType());
  }
};

template <>
struct fmt::formatter<cme_mdp3::SnapshotFullRefresh52> {
  using value_type = cme_mdp3::SnapshotFullRefresh52;
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
        roq::cme::mdp3::get_double(value.highLimitPrice()),
        roq::cme::mdp3::get_double(value.lowLimitPrice()),
        roq::cme::mdp3::get_double(value.maxPriceVariation()),
        roq::cme::mdp3::Group{value.noMDEntries()});
  }
};

// SnapshotFullRefreshOrderBook53

template <>
struct fmt::formatter<cme_mdp3::SnapshotFullRefreshOrderBook53::NoMDEntries> {
  using value_type = cme_mdp3::SnapshotFullRefreshOrderBook53::NoMDEntries;
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
        roq::cme::mdp3::get_int(value.mDOrderPriority(), value.mDOrderPriorityNullValue()),
        const_cast<value_type &>(value).mDEntryPx(),
        value.mDDisplayQty(),
        value.mDEntryType());
  }
};

template <>
struct fmt::formatter<cme_mdp3::SnapshotFullRefreshOrderBook53> {
  using value_type = cme_mdp3::SnapshotFullRefreshOrderBook53;
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
        roq::cme::mdp3::Group{value.noMDEntries()});
  }
};

// SnapshotFullRefreshLongQty69

template <>
struct fmt::formatter<cme_mdp3::SnapshotFullRefreshLongQty69::NoMDEntries> {
  using value_type = cme_mdp3::SnapshotFullRefreshLongQty69::NoMDEntries;
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
        roq::cme::mdp3::get_int(value.mDEntrySize(), value.mDEntrySizeNullValue()),
        roq::cme::mdp3::get_int(value.numberOfOrders(), value.numberOfOrdersNullValue()),
        roq::cme::mdp3::get_int(value.mDPriceLevel(), value.mDPriceLevelNullValue()),
        value.openCloseSettlFlag(),
        value.mDEntryType());
  }
};

template <>
struct fmt::formatter<cme_mdp3::SnapshotFullRefreshLongQty69> {
  using value_type = cme_mdp3::SnapshotFullRefreshLongQty69;
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
        roq::cme::mdp3::Group{value.noMDEntries()});
  }
};
