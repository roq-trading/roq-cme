/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <cme_mdp/MDInstrumentDefinitionFX63.h>
#include <cme_mdp/MDInstrumentDefinitionFixedIncome57.h>
#include <cme_mdp/MDInstrumentDefinitionFuture54.h>
#include <cme_mdp/MDInstrumentDefinitionOption55.h>
#include <cme_mdp/MDInstrumentDefinitionRepo58.h>
#include <cme_mdp/MDInstrumentDefinitionSpread56.h>

#include "roq/core/sbe/iterator.hpp"

#include "roq/cme/sbe/utils.hpp"

/*
 * MDInstrumentDefinitionFuture54
 * - NoEntries appears to be corrupted...?
 */

namespace roq {
namespace cme {
namespace sbe {

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

}  // namespace sbe
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
    value.sbeRewind();  // note!
    return fmt::format_to(
        context.out(),
        R"({{)"
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
        R"(maturity_month_year="{}", )"
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
        R"(unit_of_measure={}, )"
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
        value.matchEventIndicator(),
        value.totNumReports(),
        value.securityUpdateAction(),
        value.lastUpdateTime(),
        value.mDSecurityTradingStatus(),
        value.applID(),
        value.marketSegmentID(),
        value.underlyingProduct(),
        value.securityExchange(),
        value.securityGroup(),
        value.asset(),
        value.symbol(),
        value.securityID(),
        value.securityIDSource(),
        value.securityType(),
        value.cFICode(),
        value.maturityMonthYear(),
        value.currency(),
        value.settlCurrency(),
        value.matchAlgorithm(),
        value.minTradeVol(),
        value.maxTradeVol(),
        value.minPriceIncrement(),
        value.displayFactor(),
        value.mainFraction(),
        value.subFraction(),
        value.priceDisplayFormat(),
        value.unitOfMeasure(),
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
    // XXX binary message looks weird...
    // fmt::join(roq::core::sbe::iterator{value.noEvents()}, roq::core::sbe::sentinel{}, ", "sv),
    // fmt::join(roq::core::sbe::iterator{value.noMDFeedTypes()}, roq::core::sbe::sentinel{}, ", "sv),
    // fmt::join(roq::core::sbe::iterator{value.noInstAttrib()}, roq::core::sbe::sentinel{}, ", "sv),
    // fmt::join(roq::core::sbe::iterator{value.noLotTypeRules()}, roq::core::sbe::sentinel{}, ", "sv),
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
    value.sbeRewind();
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(symbol="{}")"
        R"(}})"sv,
        value.symbol());
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
    value.sbeRewind();  // note!
    return fmt::format_to(
        context.out(),
        R"({{)"
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
        R"(maturity_month_year="{}", )"
        R"(currency="{}", )"
        R"(security_sub_type="{}", )"
        R"(user_defined_instrument="{}", )"
        R"(match_algorithm="{}", )"
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
        value.matchEventIndicator(),
        value.totNumReports(),
        value.securityUpdateAction(),
        value.lastUpdateTime(),
        value.mDSecurityTradingStatus(),
        value.applID(),
        value.marketSegmentID(),
        value.underlyingProduct(),
        value.securityExchange(),
        value.securityGroup(),
        value.asset(),
        value.symbol(),
        value.securityID(),
        value.securityIDSource(),
        value.securityType(),
        value.cFICode(),
        value.maturityMonthYear(),
        value.currency(),
        value.securitySubType(),
        value.userDefinedInstrument(),
        value.matchAlgorithm(),
        value.minTradeVol(),
        value.maxTradeVol(),
        value.minPriceIncrement(),
        value.displayFactor(),
        value.priceDisplayFormat(),
        value.priceRatio(),
        value.tickRule(),
        value.unitOfMeasure(),
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
        value.priceQuoteMethod(),
        value.riskSet(),
        value.marketSet(),
        value.instrumentGUID(),
        value.financialInstrumentFullName());
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
    value.sbeRewind();  // note!
    return fmt::format_to(
        context.out(),
        R"({{)"
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
        R"(unit_of_measure={}, )"
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
        R"(coupon_frequency_unit={}, )"
        R"(coupon_frequency_period={}, )"
        R"(coupon_day_count={}, )"
        R"(country_of_issue={}, )"
        R"(issuer={}, )"
        R"(financial_instrument_full_name="{}", )"
        R"(security_alt_id={}, )"
        R"(security_alt_id_source={}, )"
        R"(price_quote_method={}, )"
        R"(party_role_clearing_org={}, )"
        R"(user_defined_instrument={}, )"
        R"(risk_set={}, )"
        R"(market_set={}, )"
        R"(instrument_guid={}, )"
        R"(...)"
        R"(}})"sv,
        value.matchEventIndicator(),
        value.totNumReports(),
        value.securityUpdateAction(),
        value.lastUpdateTime(),
        value.mDSecurityTradingStatus(),
        value.applID(),
        value.marketSegmentID(),
        value.underlyingProduct(),
        value.securityExchange(),
        value.securityGroup(),
        value.asset(),
        value.symbol(),
        value.securityID(),
        value.securityIDSource(),
        value.securityType(),
        value.cFICode(),
        value.currency(),
        value.settlCurrency(),
        value.matchAlgorithm(),
        value.minTradeVol(),
        value.maxTradeVol(),
        value.minPriceIncrement(),
        value.displayFactor(),
        value.mainFraction(),
        value.subFraction(),
        value.priceDisplayFormat(),
        value.unitOfMeasure(),
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
        value.couponFrequencyUnit(),
        value.couponFrequencyPeriod(),
        value.couponDayCount(),
        value.countryOfIssue(),
        value.issuer(),
        value.financialInstrumentFullName(),
        value.securityAltID(),
        value.securityAltIDSource(),
        value.priceQuoteMethod(),
        value.partyRoleClearingOrg(),
        value.userDefinedInstrument(),
        value.riskSet(),
        value.marketSet(),
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
    value.sbeRewind();  // note!
    return fmt::format_to(
        context.out(),
        R"({{)"
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
        R"(unit_of_measure={}, )"
        R"(unit_of_measure_qty={}, )"
        R"(trading_reference_price={}, )"
        R"(trading_reference_date={}, )"
        R"(high_limit_price={}, )"
        R"(low_limit_price={}, )"
        R"(max_price_variation={}, )"
        R"(financial_instrument_full_name={}, )"
        R"(party_role_clearing_org={}, )"
        R"(start_date={}, )"
        R"(end_date={}, )"
        R"(termination_type={}, )"
        R"(security_sub_type={}, )"
        R"(money_or_par={}, )"
        R"(max_no_of_substitutions={}, )"
        R"(price_quote_method={}, )"
        R"(user_defined_instrument={}, )"
        R"(risk_set={}, )"
        R"(market_set={}, )"
        R"(instrument_guid={}, )"
        R"(term_code={}, )"
        R"(...)"
        R"(}})"sv,
        value.matchEventIndicator(),
        value.totNumReports(),
        value.securityUpdateAction(),
        value.lastUpdateTime(),
        value.mDSecurityTradingStatus(),
        value.applID(),
        value.marketSegmentID(),
        value.underlyingProduct(),
        value.securityExchange(),
        value.securityGroup(),
        value.asset(),
        value.symbol(),
        value.securityID(),
        value.securityIDSource(),
        value.securityType(),
        value.cFICode(),
        value.currency(),
        value.settlCurrency(),
        value.matchAlgorithm(),
        value.minTradeVol(),
        value.maxTradeVol(),
        value.minPriceIncrement(),
        value.displayFactor(),
        value.unitOfMeasure(),
        value.unitOfMeasureQty(),
        value.tradingReferencePrice(),
        value.tradingReferenceDate(),
        value.highLimitPrice(),
        value.lowLimitPrice(),
        value.maxPriceVariation(),
        value.financialInstrumentFullName(),
        value.partyRoleClearingOrg(),
        value.startDate(),
        value.endDate(),
        value.terminationType(),
        value.securitySubType(),
        value.moneyOrPar(),
        value.maxNoOfSubstitutions(),
        value.priceQuoteMethod(),
        value.userDefinedInstrument(),
        value.riskSet(),
        value.marketSet(),
        value.instrumentGUID(),
        value.termCode());
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
    value.sbeRewind();  // note!
    return fmt::format_to(
        context.out(),
        R"({{)"
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
        R"(settl_type={}, )"
        R"(intervening_days={}, )"
        R"(fx_benchmark_rate_fix={}, )"
        R"(rate_source="{}", )"
        R"(fix_rate_local_time={}, )"
        R"(fix_rate_local_time_zone={}, )"
        R"(min_quote_life={}, )"
        R"(max_price_discretion_offset={}, )"
        R"(instrument_guid={}, )"
        R"(maturity_month_year={}, )"
        R"(settlement_locale={}, ")"
        R"(...)"
        R"(}})"sv,
        value.matchEventIndicator(),
        value.totNumReports(),
        value.securityUpdateAction(),
        value.lastUpdateTime(),
        value.mDSecurityTradingStatus(),
        value.applID(),
        value.marketSegmentID(),
        value.underlyingProduct(),
        value.securityExchange(),
        value.securityGroup(),
        value.asset(),
        value.symbol(),
        value.securityID(),
        value.securityIDSource(),
        value.securityType(),
        value.cFICode(),
        value.currency(),
        value.settlCurrency(),
        value.priceQuoteCurrency(),
        value.matchAlgorithm(),
        value.minTradeVol(),
        value.maxTradeVol(),
        value.minPriceIncrement(),
        value.displayFactor(),
        value.pricePrecision(),
        value.unitOfMeasure(),
        value.unitOfMeasureQty(),
        value.highLimitPrice(),
        value.lowLimitPrice(),
        value.maxPriceVariation(),
        value.userDefinedInstrument(),
        value.financialInstrumentFullName(),
        value.fXCurrencySymbol(),
        value.settlType(),
        value.interveningDays(),
        value.fXBenchmarkRateFix(),
        value.rateSource(),
        value.fixRateLocalTime(),
        value.fixRateLocalTimeZone(),
        value.minQuoteLife(),
        value.maxPriceDiscretionOffset(),
        value.instrumentGUID(),
        value.maturityMonthYear(),
        value.settlementLocale());
    // XXX missing groups
  }
};
