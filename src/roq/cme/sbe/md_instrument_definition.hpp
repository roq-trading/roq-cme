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
        R"(security_exchange={}, )"
        R"(security_group={}, )"
        R"(asset="{}", )"
        R"(symbol="{}", )"
        R"(security_id="{}", )"
        R"(security_id_source="{}", )"
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
        R"(no_events=[{}], )"
        R"(no_md_feed_types=[{}], )"
        R"(no_inst_attrib=[{}], )"
        R"(no_lot_type_rules=[{}])"
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
        value.instrumentGUID(),
        fmt::join(roq::core::sbe::iterator{value.noEvents()}, roq::core::sbe::sentinel{}, ", "sv),
        fmt::join(roq::core::sbe::iterator{value.noMDFeedTypes()}, roq::core::sbe::sentinel{}, ", "sv),
        fmt::join(roq::core::sbe::iterator{value.noInstAttrib()}, roq::core::sbe::sentinel{}, ", "sv),
        fmt::join(roq::core::sbe::iterator{value.noLotTypeRules()}, roq::core::sbe::sentinel{}, ", "sv));
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
struct fmt::formatter<cme_mdp::MDInstrumentDefinitionFixedIncome57> {
  using value_type = cme_mdp::MDInstrumentDefinitionFixedIncome57;
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
struct fmt::formatter<cme_mdp::MDInstrumentDefinitionRepo58> {
  using value_type = cme_mdp::MDInstrumentDefinitionRepo58;
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
struct fmt::formatter<cme_mdp::MDInstrumentDefinitionFX63> {
  using value_type = cme_mdp::MDInstrumentDefinitionFX63;
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
