/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include <cme_mdp/MDIncrementalRefreshBook46.h>
#include <cme_mdp/MDIncrementalRefreshBookLongQty64.h>
#include <cme_mdp/MDIncrementalRefreshDailyStatistics49.h>
#include <cme_mdp/MDIncrementalRefreshLimitsBanding50.h>
#include <cme_mdp/MDIncrementalRefreshOrderBook47.h>
#include <cme_mdp/MDIncrementalRefreshSessionStatistics51.h>
#include <cme_mdp/MDIncrementalRefreshSessionStatisticsLongQty67.h>
#include <cme_mdp/MDIncrementalRefreshTradeSummary48.h>
#include <cme_mdp/MDIncrementalRefreshTradeSummaryLongQty65.h>
#include <cme_mdp/MDIncrementalRefreshVolume37.h>
#include <cme_mdp/MDIncrementalRefreshVolumeLongQty66.h>

#include "roq/name.hpp"

#include "roq/core/sbe/iterator.hpp"

#include "roq/cme/mdp/utils.hpp"

namespace roq {
namespace cme {
namespace mdp {

/*
virtual void operator()(Trace<cme_mdp::MDIncrementalRefreshVolume37> const &, Frame const &) = 0;
*/

template <>
inline size_t compute_length(cme_mdp::MDIncrementalRefreshBook46 &value) {
  // NoMDEntries
  auto no_md_entries_length = value.noMDEntries().count();
  value.sbeRewind();  // wtf!
  value.noMDEntries().forEach([](auto &e) { e.skip(); });
  // NoOrderIDEntries
  auto no_order_id_entries_length = value.noOrderIDEntries().count();
  value.noOrderIDEntries().forEach([](auto &e) { e.skip(); });
  return value.computeLength(no_md_entries_length, no_order_id_entries_length);
}

/*
MDIncrementalRefreshOrderBook47
MDIncrementalRefreshTradeSummary48
MDIncrementalRefreshDailyStatistics49
MDIncrementalRefreshLimitsBanding50
MDIncrementalRefreshSessionStatistics51
MDIncrementalRefreshBookLongQty64
MDIncrementalRefreshTradeSummaryLongQty65
MDIncrementalRefreshVolumeLongQty66
MDIncrementalRefreshSessionStatisticsLongQty67
*/
}  // namespace mdp
}  // namespace cme

template <>
inline constexpr std::string_view get_name<cme_mdp::MDIncrementalRefreshVolume37>() {
  using namespace std::literals;
  return "md_incremental_refresh_volume_37"sv;
}

template <>
inline constexpr std::string_view get_name<cme_mdp::MDIncrementalRefreshBook46>() {
  using namespace std::literals;
  return "md_incremental_refresh_book_46"sv;
}

template <>
inline constexpr std::string_view get_name<cme_mdp::MDIncrementalRefreshOrderBook47>() {
  using namespace std::literals;
  return "md_incremental_refresh_order_book_47"sv;
}

template <>
inline constexpr std::string_view get_name<cme_mdp::MDIncrementalRefreshTradeSummary48>() {
  using namespace std::literals;
  return "md_incremental_refresh_trade_summary_48"sv;
}

template <>
inline constexpr std::string_view get_name<cme_mdp::MDIncrementalRefreshDailyStatistics49>() {
  using namespace std::literals;
  return "md_incremental_refresh_daily_statistics_49"sv;
}

template <>
inline constexpr std::string_view get_name<cme_mdp::MDIncrementalRefreshLimitsBanding50>() {
  using namespace std::literals;
  return "md_incremental_refresh_limits_banding_50"sv;
}

template <>
inline constexpr std::string_view get_name<cme_mdp::MDIncrementalRefreshSessionStatistics51>() {
  using namespace std::literals;
  return "md_incremental_refresh_session_statistics_51"sv;
}

template <>
inline constexpr std::string_view get_name<cme_mdp::MDIncrementalRefreshBookLongQty64>() {
  using namespace std::literals;
  return "md_incremental_refresh_book_long_qty_64"sv;
}

template <>
inline constexpr std::string_view get_name<cme_mdp::MDIncrementalRefreshTradeSummaryLongQty65>() {
  using namespace std::literals;
  return "md_incremental_refresh_trade_summary_long_qty_65"sv;
}

template <>
inline constexpr std::string_view get_name<cme_mdp::MDIncrementalRefreshVolumeLongQty66>() {
  using namespace std::literals;
  return "md_incremental_refresh_volume_long_qty_66"sv;
}

template <>
inline constexpr std::string_view get_name<cme_mdp::MDIncrementalRefreshSessionStatisticsLongQty67>() {
  using namespace std::literals;
  return "md_incremental_refresh_session_statistics_long_qty_67"sv;
}
}  // namespace roq

// MDIncrementalRefreshVolume37

template <>
struct fmt::formatter<cme_mdp::MDIncrementalRefreshVolume37::NoMDEntries> {
  using value_type = cme_mdp::MDIncrementalRefreshVolume37::NoMDEntries;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(md_entry_size={}, )"
        R"(security_id={}, )"
        R"(rpt_seq={}, )"
        R"(md_update_action={}, )"
        R"(md_entry_type={})"
        R"(}})"sv,
        value.mDEntrySize(),
        value.securityID(),
        value.rptSeq(),
        value.mDUpdateAction(),
        value.mDEntryType());
  }
};

template <>
struct fmt::formatter<cme_mdp::MDIncrementalRefreshVolume37> {
  using value_type = cme_mdp::MDIncrementalRefreshVolume37;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type &value, format_context &context) const {
    using namespace std::literals;
    value.sbeRewind();  // note!
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(version={}, )"
        R"(transact_time={}, )"
        R"(match_event_indicator={}, )"
        R"(no_md_entries=[{}])"
        R"(}})"sv,
        value.actingVersion(),
        value.transactTime(),
        value.matchEventIndicator(),
        roq::cme::mdp::Group{value.noMDEntries()});
  }
};

// MDIncrementalRefreshBook46

template <>
struct fmt::formatter<cme_mdp::MDIncrementalRefreshBook46::NoMDEntries> {
  using value_type = cme_mdp::MDIncrementalRefreshBook46::NoMDEntries;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(md_entry_px={}, )"
        R"(md_entry_size={}, )"
        R"(security_id={}, )"
        R"(rpt_seq={}, )"
        R"(number_of_orders={}, )"
        R"(md_price_level={}, )"
        R"(md_update_action={}, )"
        R"(md_entry_type={}, )"
        R"(tradeable_size={})"
        R"(}})"sv,
        const_cast<value_type &>(value).mDEntryPx(),
        roq::cme::mdp::get_int(value.mDEntrySize(), value.mDEntrySizeNullValue()),
        value.securityID(),
        value.rptSeq(),
        roq::cme::mdp::get_int(value.numberOfOrders(), value.numberOfOrdersNullValue()),
        value.mDPriceLevel(),
        value.mDUpdateAction(),
        value.mDEntryType(),
        roq::cme::mdp::get_int(value.tradeableSize(), value.tradeableSizeNullValue()));
  }
};

template <>
struct fmt::formatter<cme_mdp::MDIncrementalRefreshBook46::NoOrderIDEntries> {
  using value_type = cme_mdp::MDIncrementalRefreshBook46::NoOrderIDEntries;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(order_id={}, )"
        R"(md_order_priority={}, )"
        R"(md_display_qty={}, )"
        R"(reference_id={}, )"
        R"(order_update_action={})"
        R"(}})"sv,
        value.orderID(),
        roq::cme::mdp::get_int(value.mDOrderPriority(), value.mDOrderPriorityNullValue()),
        roq::cme::mdp::get_int(value.mDDisplayQty(), value.mDDisplayQtyNullValue()),
        roq::cme::mdp::get_int(value.referenceID(), value.referenceIDNullValue()),
        value.orderUpdateAction());
  }
};

template <>
struct fmt::formatter<cme_mdp::MDIncrementalRefreshBook46> {
  using value_type = cme_mdp::MDIncrementalRefreshBook46;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type &value, format_context &context) const {
    using namespace std::literals;
    value.sbeRewind();  // note!
    fmt::format_to(
        context.out(),
        R"({{)"
        R"(version={}, )"
        R"(transact_time={}, )"
        R"(match_event_indicator={}, )"
        R"(no_md_entries=[{})"sv,
        value.actingVersion(),
        value.transactTime(),
        value.matchEventIndicator(),
        roq::cme::mdp::Group{value.noMDEntries()});
    return fmt::format_to(
        context.out(),
        "], "
        "no_order_id_entries=[{}]"
        "}}"sv,
        roq::cme::mdp::Group{value.noOrderIDEntries()});
  }
};

// MDIncrementalRefreshOrderBook47

template <>
struct fmt::formatter<cme_mdp::MDIncrementalRefreshOrderBook47::NoMDEntries> {
  using value_type = cme_mdp::MDIncrementalRefreshOrderBook47::NoMDEntries;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(order_id={}, )"
        R"(md_order_priority={}, )"
        R"(md_entry_px={}, )"
        R"(md_display_qty={}, )"
        R"(security_id={}, )"
        R"(md_update_action={}, )"
        R"(md_entry_type={})"
        R"(}})"sv,
        roq::cme::mdp::get_int(value.orderID(), value.orderIDNullValue()),
        roq::cme::mdp::get_int(value.mDOrderPriority(), value.mDOrderPriorityNullValue()),
        const_cast<value_type &>(value).mDEntryPx(),
        roq::cme::mdp::get_int(value.mDDisplayQty(), value.mDDisplayQtyNullValue()),
        value.securityID(),
        value.mDUpdateAction(),
        value.mDEntryType());
  }
};

template <>
struct fmt::formatter<cme_mdp::MDIncrementalRefreshOrderBook47> {
  using value_type = cme_mdp::MDIncrementalRefreshOrderBook47;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type &value, format_context &context) const {
    using namespace std::literals;
    value.sbeRewind();  // note!
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(version={}, )"
        R"(transact_time={}, )"
        R"(match_event_indicator={}, )"
        R"(no_md_entries=[{}])"
        R"(}})"sv,
        value.actingVersion(),
        value.transactTime(),
        value.matchEventIndicator(),
        roq::cme::mdp::Group{value.noMDEntries()});
  }
};

// MDIncrementalRefreshTradeSummary48

template <>
struct fmt::formatter<cme_mdp::MDIncrementalRefreshTradeSummary48::NoMDEntries> {
  using value_type = cme_mdp::MDIncrementalRefreshTradeSummary48::NoMDEntries;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(md_entry_px={}, )"
        R"(md_entry_size={}, )"
        R"(security_id={}, )"
        R"(rpt_seq={}, )"
        R"(number_of_orders={}, )"
        R"(aggressor_side={}, )"
        R"(md_update_action={}, )"
        R"(md_entry_type={}, )"
        R"(md_trade_entry_id={})"
        R"(}})"sv,
        const_cast<value_type &>(value).mDEntryPx(),
        value.mDEntrySize(),
        value.securityID(),
        value.rptSeq(),
        value.numberOfOrders(),
        value.aggressorSide(),
        value.mDUpdateAction(),
        value.mDEntryType(),
        roq::cme::mdp::get_int(value.mDTradeEntryID(), value.mDTradeEntryIDNullValue()));
  }
};

template <>
struct fmt::formatter<cme_mdp::MDIncrementalRefreshTradeSummary48::NoOrderIDEntries> {
  using value_type = cme_mdp::MDIncrementalRefreshTradeSummary48::NoOrderIDEntries;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(order_id={}, )"
        R"(last_qty={})"
        R"(}})"sv,
        value.orderID(),
        value.lastQty());
  }
};

template <>
struct fmt::formatter<cme_mdp::MDIncrementalRefreshTradeSummary48> {
  using value_type = cme_mdp::MDIncrementalRefreshTradeSummary48;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type &value, format_context &context) const {
    using namespace std::literals;
    value.sbeRewind();  // note!
    fmt::format_to(
        context.out(),
        R"({{)"
        R"(version={}, )"
        R"(transact_time={}, )"
        R"(match_event_indicator={}, )"
        R"(no_md_entries=[{})"sv,
        value.actingVersion(),
        value.transactTime(),
        value.matchEventIndicator(),
        roq::cme::mdp::Group{value.noMDEntries()});
    return fmt::format_to(
        context.out(),
        R"(], )"
        R"(no_order_id_entries=[{}])"
        R"(}})"sv,
        roq::cme::mdp::Group{value.noOrderIDEntries()});
  }
};

// MDIncrementalRefreshDailyStatistics49

template <>
struct fmt::formatter<cme_mdp::MDIncrementalRefreshDailyStatistics49::NoMDEntries> {
  using value_type = cme_mdp::MDIncrementalRefreshDailyStatistics49::NoMDEntries;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(md_entry_px={}, )"
        R"(md_entry_size={}, )"
        R"(security_id={}, )"
        R"(rpt_seq={}, )"
        R"(trading_reference_date={}, )"
        R"(settl_price_type={}, )"
        R"(md_update_action={}, )"
        R"(md_entry_type={})"
        R"(}})"sv,
        const_cast<value_type &>(value).mDEntryPx(),
        roq::cme::mdp::get_int(value.mDEntrySize(), value.mDEntrySizeNullValue()),
        value.securityID(),
        value.rptSeq(),
        value.tradingReferenceDate(),
        const_cast<value_type &>(value).settlPriceType(),
        value.mDUpdateAction(),
        value.mDEntryType());
  }
};

template <>
struct fmt::formatter<cme_mdp::MDIncrementalRefreshDailyStatistics49> {
  using value_type = cme_mdp::MDIncrementalRefreshDailyStatistics49;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type &value, format_context &context) const {
    using namespace std::literals;
    value.sbeRewind();  // note!
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(version={}, )"
        R"(transact_time={}, )"
        R"(match_event_indicator={}, )"
        R"(no_md_entries=[{}])"
        R"(}})"sv,
        value.actingVersion(),
        value.transactTime(),
        value.matchEventIndicator(),
        roq::cme::mdp::Group{value.noMDEntries()});
  }
};

// MDIncrementalRefreshLimitsBanding50

template <>
struct fmt::formatter<cme_mdp::MDIncrementalRefreshLimitsBanding50::NoMDEntries> {
  using value_type = cme_mdp::MDIncrementalRefreshLimitsBanding50::NoMDEntries;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(high_limit_price={}, )"
        R"(low_limit_price={}, )"
        R"(max_price_variation={}, )"
        R"(security_id={}, )"
        R"(rpt_seq={}, )"
        R"(md_update_action={}, )"
        R"(md_entry_type={})"
        R"(}})"sv,
        const_cast<value_type &>(value).highLimitPrice(),
        const_cast<value_type &>(value).lowLimitPrice(),
        const_cast<value_type &>(value).maxPriceVariation(),
        value.securityID(),
        value.rptSeq(),
        value.mDUpdateAction(),
        value.mDEntryType());
  }
};

template <>
struct fmt::formatter<cme_mdp::MDIncrementalRefreshLimitsBanding50> {
  using value_type = cme_mdp::MDIncrementalRefreshLimitsBanding50;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type &value, format_context &context) const {
    using namespace std::literals;
    value.sbeRewind();  // note!
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(version={}, )"
        R"(transact_time={}, )"
        R"(match_event_indicator={}, )"
        R"(no_md_entries=[{}])"
        R"(}})"sv,
        value.actingVersion(),
        value.transactTime(),
        value.matchEventIndicator(),
        roq::cme::mdp::Group{value.noMDEntries()});
  }
};

// MDIncrementalRefreshSessionStatistics51

template <>
struct fmt::formatter<cme_mdp::MDIncrementalRefreshSessionStatistics51::NoMDEntries> {
  using value_type = cme_mdp::MDIncrementalRefreshSessionStatistics51::NoMDEntries;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(md_entry_px={}, )"
        R"(security_id={}, )"
        R"(rpt_seq={}, )"
        R"(open_close_settl_flag={}, )"
        R"(md_update_action={}, )"
        R"(md_entry_type={},)"
        R"(md_entry_size={})"
        R"(}})"sv,
        const_cast<value_type &>(value).mDEntryPx(),
        value.securityID(),
        value.rptSeq(),
        value.openCloseSettlFlag(),
        value.mDUpdateAction(),
        value.mDEntryType(),
        roq::cme::mdp::get_int(value.mDEntrySize(), value.mDEntrySizeNullValue()));
  }
};

template <>
struct fmt::formatter<cme_mdp::MDIncrementalRefreshSessionStatistics51> {
  using value_type = cme_mdp::MDIncrementalRefreshSessionStatistics51;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type &value, format_context &context) const {
    using namespace std::literals;
    value.sbeRewind();  // note!
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(version={}, )"
        R"(transact_time={}, )"
        R"(match_event_indicator={}, )"
        R"(no_md_entries=[{}])"
        R"(}})"sv,
        value.actingVersion(),
        value.transactTime(),
        value.matchEventIndicator(),
        roq::cme::mdp::Group{value.noMDEntries()});
  }
};

// MDIncrementalRefreshBookLongQty64

template <>
struct fmt::formatter<cme_mdp::MDIncrementalRefreshBookLongQty64::NoMDEntries> {
  using value_type = cme_mdp::MDIncrementalRefreshBookLongQty64::NoMDEntries;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(md_entry_px={}, )"
        R"(md_entry_size={}, )"
        R"(security_id={}, )"
        R"(rpt_seq={}, )"
        R"(number_of_orders={}, )"
        R"(md_price_level={}, )"
        R"(md_update_action={}, )"
        R"(md_entry_type={})"
        R"(}})"sv,
        const_cast<value_type &>(value).mDEntryPx(),
        roq::cme::mdp::get_int(value.mDEntrySize(), value.mDEntrySizeNullValue()),
        value.securityID(),
        value.rptSeq(),
        roq::cme::mdp::get_int(value.numberOfOrders(), value.numberOfOrdersNullValue()),
        value.mDPriceLevel(),
        value.mDUpdateAction(),
        value.mDEntryType());
  }
};

template <>
struct fmt::formatter<cme_mdp::MDIncrementalRefreshBookLongQty64::NoOrderIDEntries> {
  using value_type = cme_mdp::MDIncrementalRefreshBookLongQty64::NoOrderIDEntries;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(order_id={}, )"
        R"(md_order_priority={}, )"
        R"(md_display_qty={}, )"
        R"(reference_id={}, )"
        R"(order_update_action={})"
        R"(}})"sv,
        value.orderID(),
        roq::cme::mdp::get_int(value.mDOrderPriority(), value.mDOrderPriorityNullValue()),
        roq::cme::mdp::get_int(value.mDDisplayQty(), value.mDDisplayQtyNullValue()),
        roq::cme::mdp::get_int(value.referenceID(), value.referenceIDNullValue()),
        value.orderUpdateAction());
  }
};

template <>
struct fmt::formatter<cme_mdp::MDIncrementalRefreshBookLongQty64> {
  using value_type = cme_mdp::MDIncrementalRefreshBookLongQty64;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type &value, format_context &context) const {
    using namespace std::literals;
    value.sbeRewind();  // note!
    fmt::format_to(
        context.out(),
        R"({{)"
        R"(version={}, )"
        R"(transact_time={}, )"
        R"(match_event_indicator={}, )"
        R"(no_md_entries=[{})"sv,
        value.actingVersion(),
        value.transactTime(),
        value.matchEventIndicator(),
        roq::cme::mdp::Group{value.noMDEntries()});
    return fmt::format_to(
        context.out(),
        R"(], )"
        R"(no_order_id_entries=[{}])"
        R"(}})"sv,
        roq::cme::mdp::Group{value.noOrderIDEntries()});
  }
};

// MDIncrementalRefreshTradeSummaryLongQty65

template <>
struct fmt::formatter<cme_mdp::MDIncrementalRefreshTradeSummaryLongQty65::NoMDEntries> {
  using value_type = cme_mdp::MDIncrementalRefreshTradeSummaryLongQty65::NoMDEntries;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(md_entry_px={}, )"
        R"(md_entry_size={}, )"
        R"(security_id={}, )"
        R"(rpt_seq={}, )"
        R"(number_of_orders={}, )"
        R"(md_trade_entry_id={}, )"
        R"(aggressor_side={}, )"
        R"(md_update_action={}, )"
        R"(md_entry_type={})"
        R"(}})"sv,
        const_cast<value_type &>(value).mDEntryPx(),
        value.mDEntrySize(),
        value.securityID(),
        value.rptSeq(),
        value.numberOfOrders(),
        value.mDTradeEntryID(),
        value.aggressorSide(),
        value.mDUpdateAction(),
        value.mDEntryType());
  }
};

template <>
struct fmt::formatter<cme_mdp::MDIncrementalRefreshTradeSummaryLongQty65::NoOrderIDEntries> {
  using value_type = cme_mdp::MDIncrementalRefreshTradeSummaryLongQty65::NoOrderIDEntries;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(order_id={}, )"
        R"(last_qty={})"
        R"(}})"sv,
        value.orderID(),
        value.lastQty());
  }
};

template <>
struct fmt::formatter<cme_mdp::MDIncrementalRefreshTradeSummaryLongQty65> {
  using value_type = cme_mdp::MDIncrementalRefreshTradeSummaryLongQty65;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type &value, format_context &context) const {
    using namespace std::literals;
    value.sbeRewind();  // note!
    fmt::format_to(
        context.out(),
        R"({{)"
        R"(version={}, )"
        R"(transact_time={}, )"
        R"(match_event_indicator={}, )"
        R"(no_md_entries=[{})"sv,
        value.actingVersion(),
        value.transactTime(),
        value.matchEventIndicator(),
        roq::cme::mdp::Group{value.noMDEntries()});
    return fmt::format_to(
        context.out(),
        R"(], )"
        R"(no_order_id_entries=[{}])"
        R"(}})"sv,
        roq::cme::mdp::Group{value.noOrderIDEntries()});
  }
};

// MDIncrementalRefreshVolumeLongQty66

template <>
struct fmt::formatter<cme_mdp::MDIncrementalRefreshVolumeLongQty66::NoMDEntries> {
  using value_type = cme_mdp::MDIncrementalRefreshVolumeLongQty66::NoMDEntries;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(md_entry_size={}, )"
        R"(security_id={}, )"
        R"(rpt_seq={}, )"
        R"(md_update_action={}, )"
        R"(md_entry_type={})"
        R"(}})"sv,
        value.mDEntrySize(),
        value.securityID(),
        value.rptSeq(),
        value.mDUpdateAction(),
        value.mDEntryType());
  }
};

template <>
struct fmt::formatter<cme_mdp::MDIncrementalRefreshVolumeLongQty66> {
  using value_type = cme_mdp::MDIncrementalRefreshVolumeLongQty66;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type &value, format_context &context) const {
    using namespace std::literals;
    value.sbeRewind();  // note!
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(version={}, )"
        R"(transact_time={}, )"
        R"(match_event_indicator={}, )"
        R"(no_md_entries=[{}])"
        R"(}})"sv,
        value.actingVersion(),
        value.transactTime(),
        value.matchEventIndicator(),
        roq::cme::mdp::Group{value.noMDEntries()});
  }
};

// MDIncrementalRefreshSessionStatisticsLongQty67

template <>
struct fmt::formatter<cme_mdp::MDIncrementalRefreshSessionStatisticsLongQty67::NoMDEntries> {
  using value_type = cme_mdp::MDIncrementalRefreshSessionStatisticsLongQty67::NoMDEntries;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(md_entry_px={}, )"
        R"(md_entry_size={}, )"
        R"(security_id={}, )"
        R"(rpt_seq={}, )"
        R"(open_close_settl_flag={}, )"
        R"(md_update_action={}, )"
        R"(md_entry_type={})"
        R"(}})"sv,
        const_cast<value_type &>(value).mDEntryPx(),
        roq::cme::mdp::get_int(value.mDEntrySize(), value.mDEntrySizeNullValue()),
        value.securityID(),
        value.rptSeq(),
        value.openCloseSettlFlag(),
        value.mDUpdateAction(),
        value.mDEntryType());
  }
};

template <>
struct fmt::formatter<cme_mdp::MDIncrementalRefreshSessionStatisticsLongQty67> {
  using value_type = cme_mdp::MDIncrementalRefreshSessionStatisticsLongQty67;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type &value, format_context &context) const {
    using namespace std::literals;
    value.sbeRewind();  // note!
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(version={}, )"
        R"(transact_time={}, )"
        R"(match_event_indicator={}, )"
        R"(no_md_entries=[{}])"
        R"(}})"sv,
        value.actingVersion(),
        value.transactTime(),
        value.matchEventIndicator(),
        roq::cme::mdp::Group{value.noMDEntries()});
  }
};
