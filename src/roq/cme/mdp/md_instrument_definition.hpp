/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <cme_mdp/MDInstrumentDefinitionFX63.h>
#include <cme_mdp/MDInstrumentDefinitionFixedIncome57.h>
#include <cme_mdp/MDInstrumentDefinitionFuture54.h>
#include <cme_mdp/MDInstrumentDefinitionOption55.h>
#include <cme_mdp/MDInstrumentDefinitionRepo58.h>
#include <cme_mdp/MDInstrumentDefinitionSpread56.h>

#include "roq/core/sbe/iterator.hpp"

#include "roq/cme/mdp/utils.hpp"

/*
 * MDInstrumentDefinitionFuture54
 * - NoEntries appears to be corrupted...?
 */

namespace roq {
namespace cme {
namespace mdp {

template <>
inline size_t compute_length(cme_mdp::MDInstrumentDefinitionFuture54 &value) {
  // NoEvents
  auto no_events_length = value.noEvents().count();
  value.sbeRewind();  // wtf!
  value.noEvents().forEach([](auto &e) { e.skip(); });
  // NoMDFeedTypes
  auto no_md_feed_types_length = value.noMDFeedTypes().count();
  value.noMDFeedTypes().forEach([](auto &e) { e.skip(); });
  // NoInstAttrib
  auto no_inst_attrib_length = value.noInstAttrib().count();
  value.noInstAttrib().forEach([](auto &e) { e.skip(); });
  // NoLotTypeRules
  auto no_lot_type_rules_length = value.noLotTypeRules().count();
  value.noLotTypeRules().forEach([](auto &e) { e.skip(); });
  return value.computeLength(
      no_events_length, no_md_feed_types_length, no_inst_attrib_length, no_lot_type_rules_length);
}

template <>
inline size_t compute_length(cme_mdp::MDInstrumentDefinitionOption55 &value) {
  // NoEvents
  auto no_events_length = value.noEvents().count();
  value.sbeRewind();  // wtf!
  value.noEvents().forEach([](auto &e) { e.skip(); });
  // NoMDFeedTypes
  auto no_md_feed_types_length = value.noMDFeedTypes().count();
  value.noMDFeedTypes().forEach([](auto &e) { e.skip(); });
  // NoInstAttrib
  auto no_inst_attrib_length = value.noInstAttrib().count();
  value.noInstAttrib().forEach([](auto &e) { e.skip(); });
  // NoLotTypeRules
  auto no_lot_type_rules_length = value.noLotTypeRules().count();
  value.noLotTypeRules().forEach([](auto &e) { e.skip(); });
  // NoUnderlyings
  auto no_underlyings_length = value.noUnderlyings().count();
  value.noUnderlyings().forEach([](auto &e) { e.skip(); });
  // NoRelatedInstruments
  auto no_related_instruments_length = value.noRelatedInstruments().count();
  value.noRelatedInstruments().forEach([](auto &e) { e.skip(); });
  return value.computeLength(
      no_events_length,
      no_md_feed_types_length,
      no_inst_attrib_length,
      no_lot_type_rules_length,
      no_underlyings_length,
      no_related_instruments_length);
}

template <>
inline size_t compute_length(cme_mdp::MDInstrumentDefinitionSpread56 &value) {
  // NoEvents
  auto no_events_length = value.noEvents().count();
  value.sbeRewind();  // wtf!
  value.noEvents().forEach([](auto &e) { e.skip(); });
  // NoMDFeedTypes
  auto no_md_feed_types_length = value.noMDFeedTypes().count();
  value.noMDFeedTypes().forEach([](auto &e) { e.skip(); });
  // NoInstAttrib
  auto no_inst_attrib_length = value.noInstAttrib().count();
  value.noInstAttrib().forEach([](auto &e) { e.skip(); });
  // NoLotTypeRules
  auto no_lot_type_rules_length = value.noLotTypeRules().count();
  value.noLotTypeRules().forEach([](auto &e) { e.skip(); });
  // NoLegs
  auto no_legs_length = value.noLegs().count();
  value.noLegs().forEach([](auto &e) { e.skip(); });
  return value.computeLength(
      no_events_length, no_md_feed_types_length, no_inst_attrib_length, no_lot_type_rules_length, no_legs_length);
}

template <>
inline size_t compute_length(cme_mdp::MDInstrumentDefinitionFixedIncome57 &value) {
  // NoEvents
  auto no_events_length = value.noEvents().count();
  value.sbeRewind();  // wtf!
  value.noEvents().forEach([](auto &e) { e.skip(); });
  // NoMDFeedTypes
  auto no_md_feed_types_length = value.noMDFeedTypes().count();
  value.noMDFeedTypes().forEach([](auto &e) { e.skip(); });
  // NoInstAttrib
  auto no_inst_attrib_length = value.noInstAttrib().count();
  value.noInstAttrib().forEach([](auto &e) { e.skip(); });
  // NoLotTypeRules
  auto no_lot_type_rules_length = value.noLotTypeRules().count();
  value.noLotTypeRules().forEach([](auto &e) { e.skip(); });
  return value.computeLength(
      no_events_length, no_md_feed_types_length, no_inst_attrib_length, no_lot_type_rules_length);
}

template <>
inline size_t compute_length(cme_mdp::MDInstrumentDefinitionRepo58 &value) {
  // NoEvents
  auto no_events_length = value.noEvents().count();
  value.sbeRewind();  // wtf!
  value.noEvents().forEach([](auto &e) { e.skip(); });
  // NoMDFeedTypes
  auto no_md_feed_types_length = value.noMDFeedTypes().count();
  value.noMDFeedTypes().forEach([](auto &e) { e.skip(); });
  // NoInstAttrib
  auto no_inst_attrib_length = value.noInstAttrib().count();
  value.noInstAttrib().forEach([](auto &e) { e.skip(); });
  // NoLotTypeRules
  auto no_lot_type_rules_length = value.noLotTypeRules().count();
  value.noLotTypeRules().forEach([](auto &e) { e.skip(); });
  // NoUnderlyings
  auto no_underlyings_length = value.noUnderlyings().count();
  value.noUnderlyings().forEach([](auto &e) { e.skip(); });
  // NoRelatedInstruments
  auto no_related_instruments_length = value.noRelatedInstruments().count();
  value.noRelatedInstruments().forEach([](auto &e) { e.skip(); });
  return value.computeLength(
      no_events_length,
      no_md_feed_types_length,
      no_inst_attrib_length,
      no_lot_type_rules_length,
      no_underlyings_length,
      no_related_instruments_length);
}

template <>
inline size_t compute_length(cme_mdp::MDInstrumentDefinitionFX63 &value) {
  // NoEvents
  auto no_events_length = value.noEvents().count();
  value.sbeRewind();  // wtf!
  value.noEvents().forEach([](auto &e) { e.skip(); });
  // NoMDFeedTypes
  auto no_md_feed_types_length = value.noMDFeedTypes().count();
  value.noMDFeedTypes().forEach([](auto &e) { e.skip(); });
  // NoInstAttrib
  auto no_inst_attrib_length = value.noInstAttrib().count();
  value.noInstAttrib().forEach([](auto &e) { e.skip(); });
  // NoLotTypeRules
  auto no_lot_type_rules_length = value.noLotTypeRules().count();
  value.noLotTypeRules().forEach([](auto &e) { e.skip(); });
  // NoTradingSessions
  auto no_trading_sessions_length = value.noTradingSessions().count();
  value.noTradingSessions().forEach([](auto &e) { e.skip(); });
  return value.computeLength(
      no_events_length,
      no_md_feed_types_length,
      no_inst_attrib_length,
      no_lot_type_rules_length,
      no_trading_sessions_length);
}

}  // namespace mdp
}  // namespace cme
}  // namespace roq

// messages

// MDInstrumentDefinitionFuture54

template <>
struct fmt::formatter<cme_mdp::MDInstrumentDefinitionFuture54::NoEvents> {
  using value_type = cme_mdp::MDInstrumentDefinitionFuture54::NoEvents;
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
        R"(event_type={}, )"
        R"(event_time={})"
        R"(}})"sv,
        value.eventType(),
        value.eventTime());
  }
};

template <>
struct fmt::formatter<cme_mdp::MDInstrumentDefinitionFuture54::NoMDFeedTypes> {
  using value_type = cme_mdp::MDInstrumentDefinitionFuture54::NoMDFeedTypes;
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
        R"(md_feed_type={}, )"
        R"(market_depth={})"
        R"(}})"sv,
        value.mDFeedType(),
        value.marketDepth());
  }
};

template <>
struct fmt::formatter<cme_mdp::MDInstrumentDefinitionFuture54::NoInstAttrib> {
  using value_type = cme_mdp::MDInstrumentDefinitionFuture54::NoInstAttrib;
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type const &value, Context &context) const {
    using namespace std::literals;
    /* XXX FIXME
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(inst_attrib_type={}, )"
        R"(inst_attrib_value={})"
        R"(}})"sv,
        value.instAttribType(),
        value.instAttribValue());
    */
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(inst_attrib_type={}, )"
        R"(inst_attrib_value=???)"
        R"(}})"sv,
        value.instAttribType());
  }
};

template <>
struct fmt::formatter<cme_mdp::MDInstrumentDefinitionFuture54::NoLotTypeRules> {
  using value_type = cme_mdp::MDInstrumentDefinitionFuture54::NoLotTypeRules;
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type const &value, Context &context) const {
    using namespace std::literals;
    /* XXX FIXME
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(lot_type={}, )"
        R"(min_lot_size={})"
        R"(}})"sv,
        value.lotType(),
        value.minLotSize());
    */
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(lot_type={}, )"
        R"(min_lot_size=???)"
        R"(}})"sv,
        value.lotType());
  }
};

template <>
struct fmt::formatter<cme_mdp::MDInstrumentDefinitionFuture54> {
  using value_type = cme_mdp::MDInstrumentDefinitionFuture54;
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
        R"(match_event_indicator={}, )"
        R"(tot_num_reports={}, )"
        R"(security_update_action={}, )"
        R"(last_update_time={}, )"
        R"(md_security_trading_status={}, )"
        R"(appl_id={}, )"
        R"(market_segment_id={}, )"
        R"(underlying_product={}, )"
        R"(security_exchange="{}", )"
        R"(security_group="{}", )"
        R"(asset="{}", )"
        R"(symbol="{}", )"
        R"(security_id={}, )"
        R"(security_id_source={}, )"
        R"(security_Type="{}", )"
        R"(cfi_code="{}", )"
        R"(maturity_month_year={}, )"
        R"(currency="{}", )"
        R"(settl_currency="{}", )"
        R"(match_algorithm={}, )"
        R"(min_trade_vol={}, )"
        R"(max_trade_vol={}, )"
        R"(min_price_increment={}, )"
        R"(display_factor={}, )"
        R"(main_fraction={}, )"
        R"(sub_fraction={}, )"
        R"(price_display_format={}, )"
        R"(unit_of_measure="{}", )"
        R"(unit_of_measure_qty={}, )"
        R"(trading_reference_price={}, )"
        R"(settl_price_type={}, )"
        R"(open_interest_qty={}, )"
        R"(cleared_volume={}, )"
        R"(high_limit_price={}, )"
        R"(low_limit_price={}, )"
        R"(max_price_variation={}, )"
        R"(decay_quantity={}, )"
        R"(decay_start_date={}, )"
        R"(original_contract_size={}, )"
        R"(contract_multiplier={}, )"
        R"(contract_multiplier_unit={}, )"
        R"(flow_schedule_type={}, )"
        R"(min_price_increment_amount={}, )"
        R"(user_defined_instrument={}, )"
        R"(trading_reference_date={}, )"
        R"(instrument_guid={}, )"
        R"(...)"
        R"(}})"sv,
        value.actingVersion(),
        value.matchEventIndicator(),
        value.totNumReports(),
        value.securityUpdateAction(),
        value.lastUpdateTime(),
        value.mDSecurityTradingStatus(),
        value.applID(),
        value.marketSegmentID(),
        value.underlyingProduct(),
        get_string_view(value.securityExchange(), value.securityExchangeLength()),
        get_string_view(value.securityGroup(), value.securityGroupLength()),
        get_string_view(value.asset(), value.assetLength()),
        get_string_view(value.symbol(), value.symbolLength()),
        value.securityID(),
        value.securityIDSource(),
        get_string_view(value.securityType(), value.securityTypeLength()),
        get_string_view(value.cFICode(), value.cFICodeLength()),
        value.maturityMonthYear(),
        get_string_view(value.currency(), value.currencyLength()),
        get_string_view(value.settlCurrency(), value.settlCurrencyLength()),
        value.matchAlgorithm(),
        value.minTradeVol(),
        value.maxTradeVol(),
        value.minPriceIncrement(),
        value.displayFactor(),
        value.mainFraction(),
        value.subFraction(),
        value.priceDisplayFormat(),
        get_string_view(value.unitOfMeasure(), value.unitOfMeasureLength()),
        value.unitOfMeasureQty(),
        value.tradingReferencePrice(),
        value.settlPriceType(),
        value.openInterestQty(),
        value.clearedVolume(),
        value.highLimitPrice(),
        value.lowLimitPrice(),
        value.maxPriceVariation(),
        value.decayQuantity(),
        value.decayStartDate(),
        value.originalContractSize(),
        value.contractMultiplier(),
        value.contractMultiplierUnit(),
        value.flowScheduleType(),
        value.minPriceIncrementAmount(),
        value.userDefinedInstrument(),
        value.tradingReferenceDate(),
        value.instrumentGUID());
    // XXX missing groups
  }
};

template <>
struct fmt::formatter<cme_mdp::MDInstrumentDefinitionOption55> {
  using value_type = cme_mdp::MDInstrumentDefinitionOption55;
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
        R"(match_event_indicator={}, )"
        R"(tot_num_reports={}, )"
        R"(security_update_action={}, )"
        R"(last_update_time={}, )"
        R"(md_security_trading_status={}, )"
        R"(appl_id={}, )"
        R"(market_segment_id={}, )"
        R"(underlying_product={}, )"
        R"(security_exchange="{}", )"
        R"(security_group="{}", )"
        R"(asset="{}", )"
        R"(symbol="{}", )"
        R"(security_id={}, )"
        R"(security_id_source={}, )"
        R"(security_Type="{}", )"
        R"(cfi_code="{}", )"
        R"(put_or_call={}, )"
        R"(maturity_month_year={}, )"
        R"(currency="{}", )"
        R"(strike_price={}, )"
        R"(strike_currency="{}", )"
        R"(settl_currency="{}", )"
        R"(min_cab_price={}, )"
        R"(match_algorithm={}, )"
        R"(min_trade_vol={}, )"
        R"(max_trade_vol={}, )"
        R"(min_price_increment={}, )"
        R"(min_price_increment_amount={}, )"
        R"(display_factor={}, )"
        R"(tick_rule={}, )"
        R"(main_fraction={}, )"
        R"(sub_fraction={}, )"
        R"(price_display_format={}, )"
        R"(unit_of_measure="{}", )"
        R"(unit_of_measure_qty={}, )"
        R"(trading_reference_price={}, )"
        R"(settl_price_type={}, )"
        R"(cleared_volume={}, )"
        R"(open_interest_qty={}, )"
        R"(low_limit_price={}, )"
        R"(high_limit_price={}, )"
        R"(user_defined_instrument={}, )"
        R"(trading_reference_date={}, )"
        R"(instrument_guid={}, )"
        R"(...)"
        R"(}})"sv,
        value.actingVersion(),
        value.matchEventIndicator(),
        value.totNumReports(),
        value.securityUpdateAction(),
        value.lastUpdateTime(),
        value.mDSecurityTradingStatus(),
        value.applID(),
        value.marketSegmentID(),
        value.underlyingProduct(),
        get_string_view(value.securityExchange(), value.securityExchangeLength()),
        get_string_view(value.securityGroup(), value.securityGroupLength()),
        get_string_view(value.asset(), value.assetLength()),
        get_string_view(value.symbol(), value.symbolLength()),
        value.securityID(),
        value.securityIDSource(),
        get_string_view(value.securityType(), value.securityTypeLength()),
        get_string_view(value.cFICode(), value.cFICodeLength()),
        value.putOrCall(),
        value.maturityMonthYear(),
        get_string_view(value.currency(), value.currencyLength()),
        value.strikePrice(),
        get_string_view(value.strikeCurrency(), value.strikeCurrencyLength()),
        get_string_view(value.settlCurrency(), value.settlCurrencyLength()),
        value.minCabPrice(),
        value.matchAlgorithm(),
        value.minTradeVol(),
        value.maxTradeVol(),
        value.minPriceIncrement(),
        value.minPriceIncrementAmount(),
        value.displayFactor(),
        value.tickRule(),
        value.mainFraction(),
        value.subFraction(),
        value.priceDisplayFormat(),
        get_string_view(value.unitOfMeasure(), value.unitOfMeasureLength()),
        value.unitOfMeasureQty(),
        value.tradingReferencePrice(),
        value.settlPriceType(),
        value.clearedVolume(),
        value.openInterestQty(),
        value.lowLimitPrice(),
        value.highLimitPrice(),
        value.userDefinedInstrument(),
        value.tradingReferenceDate(),
        value.instrumentGUID());
    // XXX missing groups
  }
};

template <>
struct fmt::formatter<cme_mdp::MDInstrumentDefinitionSpread56> {
  using value_type = cme_mdp::MDInstrumentDefinitionSpread56;
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
        R"(match_event_indicator={}, )"
        R"(tot_num_reports={}, )"
        R"(security_update_action={}, )"
        R"(last_update_time={}, )"
        R"(md_security_trading_status={}, )"
        R"(appl_id={}, )"
        R"(market_segment_id={}, )"
        R"(underlying_product={}, )"
        R"(security_exchange="{}", )"
        R"(security_group="{}", )"
        R"(asset="{}", )"
        R"(symbol="{}", )"
        R"(security_id={}, )"
        R"(security_id_source={}, )"
        R"(security_Type="{}", )"
        R"(cfi_code="{}", )"
        R"(maturity_month_year={}, )"
        R"(currency="{}", )"
        R"(security_sub_type="{}", )"
        R"(user_defined_instrument={}, )"
        R"(match_algorithm={}, )"
        R"(min_trade_vol={}, )"
        R"(max_trade_vol={}, )"
        R"(min_price_increment={}, )"
        R"(display_factor={}, )"
        R"(price_display_format={}, )"
        R"(price_ratio={}, )"
        R"(tick_rule={}, )"
        R"(unit_of_measure="{}", )"
        R"(trading_reference_price={}, )"
        R"(settle_price_type={}, )"
        R"(open_interest_qty={}, )"
        R"(cleared_volume={}, )"
        R"(high_limit_price={}, )"
        R"(low_limit_price={}, )"
        R"(max_price_variation={}, )"
        R"(main_fraction={}, )"
        R"(sub_fraction={}, )"
        R"(trading_reference_date={}, )"
        R"(price_quote_method="{}", )"
        R"(risk_set="{}", )"
        R"(market_set="{}", )"
        R"(instrument_guid={}, )"
        R"(financial_instrument_full_name="{}", )"
        R"(...)"
        R"(}})"sv,
        value.actingVersion(),
        value.matchEventIndicator(),
        value.totNumReports(),
        value.securityUpdateAction(),
        value.lastUpdateTime(),
        value.mDSecurityTradingStatus(),
        value.applID(),
        value.marketSegmentID(),
        value.underlyingProduct(),
        get_string_view(value.securityExchange(), value.securityExchangeLength()),
        get_string_view(value.securityGroup(), value.securityGroupLength()),
        get_string_view(value.asset(), value.assetLength()),
        get_string_view(value.symbol(), value.symbolLength()),
        value.securityID(),
        value.securityIDSource(),
        get_string_view(value.securityType(), value.securityTypeLength()),
        get_string_view(value.cFICode(), value.cFICodeLength()),
        value.maturityMonthYear(),
        get_string_view(value.currency(), value.currencyLength()),
        get_string_view(value.securitySubType(), value.securitySubTypeLength()),
        value.userDefinedInstrument(),
        value.matchAlgorithm(),
        value.minTradeVol(),
        value.maxTradeVol(),
        value.minPriceIncrement(),
        value.displayFactor(),
        value.priceDisplayFormat(),
        value.priceRatio(),
        value.tickRule(),
        get_string_view(value.unitOfMeasure(), value.unitOfMeasureLength()),
        value.tradingReferencePrice(),
        value.settlPriceType(),
        value.openInterestQty(),
        value.clearedVolume(),
        value.highLimitPrice(),
        value.lowLimitPrice(),
        value.maxPriceVariation(),
        value.mainFraction(),
        value.subFraction(),
        value.tradingReferenceDate(),
        get_string_view(value.priceQuoteMethod(), value.priceQuoteMethodLength()),
        get_string_view(value.riskSet(), value.riskSetLength()),
        get_string_view(value.marketSet(), value.marketSetLength()),
        value.instrumentGUID(),
        get_string_view(value.financialInstrumentFullName(), value.financialInstrumentFullNameLength()));
    // XXX missing groups
  }
};

template <>
struct fmt::formatter<cme_mdp::MDInstrumentDefinitionFixedIncome57> {
  using value_type = cme_mdp::MDInstrumentDefinitionFixedIncome57;
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
        R"(match_event_indicator={}, )"
        R"(tot_num_reports={}, )"
        R"(security_update_action={}, )"
        R"(last_update_time={}, )"
        R"(md_security_trading_status={}, )"
        R"(appl_id={}, )"
        R"(market_segment_id={}, )"
        R"(underlying_product={}, )"
        R"(security_exchange="{}", )"
        R"(security_group="{}", )"
        R"(asset="{}", )"
        R"(symbol="{}", )"
        R"(security_id={}, )"
        R"(security_id_source={}, )"
        R"(security_Type="{}", )"
        R"(cfi_code="{}", )"
        R"(currency="{}", )"
        R"(settl_currency="{}", )"
        R"(match_algorithm={}, )"
        R"(min_trade_vol={}, )"
        R"(max_trade_vol={}, )"
        R"(min_price_increment={}, )"
        R"(display_factor={}, )"
        R"(main_fraction={}, )"
        R"(sub_fraction={}, )"
        R"(price_display_format={}, )"
        R"(unit_of_measure="{}", )"
        R"(unit_of_measure_qty={}, )"
        R"(trading_reference_price={}, )"
        R"(trading_reference_date={}, )"
        R"(high_limit_price={}, )"
        R"(low_limit_price={}, )"
        R"(max_price_variation={}, )"
        R"(min_price_increment_amount={}, )"
        R"(issue_date={}, )"
        R"(dated_date={}, )"
        R"(maturity_date={}, )"
        R"(coupon_rate={}, )"
        R"(par_value={}, )"
        R"(coupon_frequency_unit="{}", )"
        R"(coupon_frequency_period={}, )"
        R"(coupon_day_count="{}", )"
        R"(country_of_issue="{}", )"
        R"(issuer="{}", )"
        R"(financial_instrument_full_name="{}", )"
        R"(security_alt_id="{}", )"
        R"(security_alt_id_source={}, )"
        R"(price_quote_method="{}", )"
        R"(party_role_clearing_org="{}", )"
        R"(user_defined_instrument={}, )"
        R"(risk_set="{}", )"
        R"(market_set="{}", )"
        R"(instrument_guid={}, )"
        R"(...)"
        R"(}})"sv,
        value.actingVersion(),
        value.matchEventIndicator(),
        value.totNumReports(),
        value.securityUpdateAction(),
        value.lastUpdateTime(),
        value.mDSecurityTradingStatus(),
        value.applID(),
        value.marketSegmentID(),
        value.underlyingProduct(),
        get_string_view(value.securityExchange(), value.securityExchangeLength()),
        get_string_view(value.securityGroup(), value.securityGroupLength()),
        get_string_view(value.asset(), value.assetLength()),
        get_string_view(value.symbol(), value.symbolLength()),
        value.securityID(),
        value.securityIDSource(),
        get_string_view(value.securityType(), value.securityTypeLength()),
        get_string_view(value.cFICode(), value.cFICodeLength()),
        get_string_view(value.currency(), value.currencyLength()),
        get_string_view(value.settlCurrency(), value.settlCurrencyLength()),
        value.matchAlgorithm(),
        value.minTradeVol(),
        value.maxTradeVol(),
        value.minPriceIncrement(),
        value.displayFactor(),
        value.mainFraction(),
        value.subFraction(),
        value.priceDisplayFormat(),
        get_string_view(value.unitOfMeasure(), value.unitOfMeasureLength()),
        value.unitOfMeasureQty(),
        value.tradingReferencePrice(),
        value.tradingReferenceDate(),
        value.highLimitPrice(),
        value.lowLimitPrice(),
        value.maxPriceVariation(),
        value.minPriceIncrementAmount(),
        value.issueDate(),
        value.datedDate(),
        value.maturityDate(),
        value.couponRate(),
        value.parValue(),
        get_string_view(value.couponFrequencyUnit(), value.couponFrequencyUnitLength()),
        value.couponFrequencyPeriod(),
        get_string_view(value.couponDayCount(), value.couponDayCountLength()),
        get_string_view(value.countryOfIssue(), value.countryOfIssueLength()),
        get_string_view(value.issuer(), value.issuerLength()),
        get_string_view(value.financialInstrumentFullName(), value.financialInstrumentFullNameLength()),
        get_string_view(value.securityAltID(), value.securityAltIDLength()),
        value.securityAltIDSource(),
        get_string_view(value.priceQuoteMethod(), value.priceQuoteMethodLength()),
        get_string_view(value.partyRoleClearingOrg(), value.partyRoleClearingOrgLength()),
        value.userDefinedInstrument(),
        get_string_view(value.riskSet(), value.riskSetLength()),
        get_string_view(value.marketSet(), value.marketSetLength()),
        value.instrumentGUID());
    // XXX missing groups
  }
};

template <>
struct fmt::formatter<cme_mdp::MDInstrumentDefinitionRepo58> {
  using value_type = cme_mdp::MDInstrumentDefinitionRepo58;
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
        R"(match_event_indicator={}, )"
        R"(tot_num_reports={}, )"
        R"(security_update_action={}, )"
        R"(last_update_time={}, )"
        R"(md_security_trading_status={}, )"
        R"(appl_id={}, )"
        R"(market_segment_id={}, )"
        R"(underlying_product={}, )"
        R"(security_exchange="{}", )"
        R"(security_group="{}", )"
        R"(asset="{}", )"
        R"(symbol="{}", )"
        R"(security_id={}, )"
        R"(security_id_source={}, )"
        R"(security_type="{}", )"
        R"(cfi_code="{}", )"
        R"(currency="{}", )"
        R"(settl_currency="{}", )"
        R"(match_algorithm={}, )"
        R"(min_trade_vol={}, )"
        R"(max_trade_vol={}, )"
        R"(min_price_increment={}, )"
        R"(display_factor={}, )"
        R"(unit_of_measure="{}", )"
        R"(unit_of_measure_qty={}, )"
        R"(trading_reference_price={}, )"
        R"(trading_reference_date={}, )"
        R"(high_limit_price={}, )"
        R"(low_limit_price={}, )"
        R"(max_price_variation={}, )"
        R"(financial_instrument_full_name="{}", )"
        R"(party_role_clearing_org="{}", )"
        R"(start_date={}, )"
        R"(end_date={}, )"
        R"(termination_type="{}", )"
        R"(security_sub_type={}, )"
        R"(money_or_par={}, )"
        R"(max_no_of_substitutions={}, )"
        R"(price_quote_method="{}", )"
        R"(user_defined_instrument={}, )"
        R"(risk_set="{}", )"
        R"(market_set="{}", )"
        R"(instrument_guid={}, )"
        R"(term_code="{}", )"
        R"(...)"
        R"(}})"sv,
        value.actingVersion(),
        value.matchEventIndicator(),
        value.totNumReports(),
        value.securityUpdateAction(),
        value.lastUpdateTime(),
        value.mDSecurityTradingStatus(),
        value.applID(),
        value.marketSegmentID(),
        value.underlyingProduct(),
        get_string_view(value.securityExchange(), value.securityExchangeLength()),
        get_string_view(value.securityGroup(), value.securityGroupLength()),
        get_string_view(value.asset(), value.assetLength()),
        get_string_view(value.symbol(), value.symbolLength()),
        value.securityID(),
        value.securityIDSource(),
        get_string_view(value.securityType(), value.securityTypeLength()),
        get_string_view(value.cFICode(), value.cFICodeLength()),
        get_string_view(value.currency(), value.currencyLength()),
        get_string_view(value.settlCurrency(), value.settlCurrencyLength()),
        value.matchAlgorithm(),
        value.minTradeVol(),
        value.maxTradeVol(),
        value.minPriceIncrement(),
        value.displayFactor(),
        get_string_view(value.unitOfMeasure(), value.unitOfMeasureLength()),
        value.unitOfMeasureQty(),
        value.tradingReferencePrice(),
        value.tradingReferenceDate(),
        value.highLimitPrice(),
        value.lowLimitPrice(),
        value.maxPriceVariation(),
        get_string_view(value.financialInstrumentFullName(), value.financialInstrumentFullNameLength()),
        get_string_view(value.partyRoleClearingOrg(), value.partyRoleClearingOrgLength()),
        value.startDate(),
        value.endDate(),
        get_string_view(value.terminationType(), value.terminationTypeLength()),
        value.securitySubType(),
        value.moneyOrPar(),
        value.maxNoOfSubstitutions(),
        get_string_view(value.priceQuoteMethod(), value.priceQuoteMethodLength()),
        value.userDefinedInstrument(),
        get_string_view(value.riskSet(), value.riskSetLength()),
        get_string_view(value.marketSet(), value.marketSetLength()),
        value.instrumentGUID(),
        get_string_view(value.termCode(), value.termCodeLength()));
    // XXX missing groups
  }
};

template <>
struct fmt::formatter<cme_mdp::MDInstrumentDefinitionFX63> {
  using value_type = cme_mdp::MDInstrumentDefinitionFX63;
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
        R"(match_event_indicator={}, )"
        R"(tot_num_reports={}, )"
        R"(security_update_action={}, )"
        R"(last_update_time={}, )"
        R"(md_security_trading_status={}, )"
        R"(appl_id={}, )"
        R"(market_segment_id={}, )"
        R"(underlying_product={}, )"
        R"(security_exchange="{}", )"
        R"(security_group="{}", )"
        R"(asset="{}", )"
        R"(symbol="{}", )"
        R"(security_id={}, )"
        R"(security_id_source={}, )"
        R"(security_Type="{}", )"
        R"(cfi_code="{}", )"
        R"(currency="{}", )"
        R"(settl_currency="{}", )"
        R"(price_quote_currency="{}", )"
        R"(match_algorithm={}, )"
        R"(min_trade_vol={}, )"
        R"(max_trade_vol={}, )"
        R"(min_price_increment={}, )"
        R"(display_factor={}, )"
        R"(price_precision={}, )"
        R"(unit_of_measure="{}", )"
        R"(unit_of_measure_qty={}, )"
        R"(high_limit_price={}, )"
        R"(low_limit_price={}, )"
        R"(max_price_variation={}, )"
        R"(user_defined_instrument={}, )"
        R"(financial_instrument_full_name="{}", )"
        R"(fx_currency_symbol="{}", )"
        R"(settl_type="{}", )"
        R"(intervening_days={}, )"
        R"(fx_benchmark_rate_fix="{}", )"
        R"(rate_source="{}", )"
        R"(fix_rate_local_time="{}", )"
        R"(fix_rate_local_time_zone="{}", )"
        R"(min_quote_life={}, )"
        R"(max_price_discretion_offset={}, )"
        R"(instrument_guid={}, )"
        R"(maturity_month_year={}, )"
        R"(settlement_locale="{}", ")"
        R"(...)"
        R"(}})"sv,
        value.actingVersion(),
        value.matchEventIndicator(),
        value.totNumReports(),
        value.securityUpdateAction(),
        value.lastUpdateTime(),
        value.mDSecurityTradingStatus(),
        value.applID(),
        value.marketSegmentID(),
        value.underlyingProduct(),
        get_string_view(value.securityExchange(), value.securityExchangeLength()),
        get_string_view(value.securityGroup(), value.securityGroupLength()),
        get_string_view(value.asset(), value.assetLength()),
        get_string_view(value.symbol(), value.symbolLength()),
        value.securityID(),
        value.securityIDSource(),
        get_string_view(value.securityType(), value.securityTypeLength()),
        get_string_view(value.cFICode(), value.cFICodeLength()),
        get_string_view(value.currency(), value.currencyLength()),
        get_string_view(value.settlCurrency(), value.settlCurrencyLength()),
        get_string_view(value.priceQuoteCurrency(), value.priceQuoteCurrencyLength()),
        value.matchAlgorithm(),
        value.minTradeVol(),
        value.maxTradeVol(),
        value.minPriceIncrement(),
        value.displayFactor(),
        value.pricePrecision(),
        get_string_view(value.unitOfMeasure(), value.unitOfMeasureLength()),
        value.unitOfMeasureQty(),
        value.highLimitPrice(),
        value.lowLimitPrice(),
        value.maxPriceVariation(),
        value.userDefinedInstrument(),
        get_string_view(value.financialInstrumentFullName(), value.financialInstrumentFullNameLength()),
        get_string_view(value.fXCurrencySymbol(), value.fXCurrencySymbolLength()),
        get_string_view(value.settlType(), value.settlTypeLength()),
        value.interveningDays(),
        get_string_view(value.fXBenchmarkRateFix(), value.fXBenchmarkRateFixLength()),
        get_string_view(value.rateSource(), value.rateSourceLength()),
        get_string_view(value.fixRateLocalTime(), value.fixRateLocalTimeLength()),
        get_string_view(value.fixRateLocalTimeZone(), value.fixRateLocalTimeZoneLength()),
        value.minQuoteLife(),
        value.maxPriceDiscretionOffset(),
        value.instrumentGUID(),
        value.maturityMonthYear(),
        get_string_view(value.settlementLocale(), value.settlementLocaleLength()));
    // XXX missing groups
  }
};
