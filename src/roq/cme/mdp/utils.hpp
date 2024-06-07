/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include <fmt/chrono.h>
#include <fmt/format.h>

#include <cme_mdp/MessageHeader.h>

#include <cme_mdp/AggressorSide.h>
#include <cme_mdp/Side.h>

#include <cme_mdp/MDInstrumentDefinitionFX63.h>

#include <magic_enum.hpp>

#include "roq/api.hpp"

#include "roq/core/sbe/iterator.hpp"

#include "roq/logging.hpp"

namespace roq {
namespace cme {
namespace mdp {

inline Side map(cme_mdp::MDEntryTypeBook::Value value) {
  switch (value) {
    using enum cme_mdp::MDEntryTypeBook::Value;
    case Bid:
      return Side::BUY;
    case Offer:
      return Side::SELL;
    case ImpliedBid:
      break;
    case ImpliedOffer:
      break;
    case BookReset:
      break;
    case MarketBestOffer:
      break;
    case MarketBestBid:
      break;
    case NULL_VALUE:
      break;
  }
  return {};
}

inline Side map_side(cme_mdp::AggressorSide::Value value) {
  switch (value) {
    using enum cme_mdp::AggressorSide::Value;
    case NoAggressor:
      break;
    case Buy:
      return Side::BUY;
    case Sell:
      return Side::SELL;
    case NULL_VALUE:
      break;
  }
  return {};
}

inline StatisticsType map(cme_mdp::MDEntryTypeDailyStatistics::Value value) {
  switch (value) {
    using enum cme_mdp::MDEntryTypeDailyStatistics::Value;
    case SettlementPrice:
      return StatisticsType::SETTLEMENT_PRICE;
      break;
    case ClearedVolume:
      break;
    case OpenInterest:
      return StatisticsType::OPEN_INTEREST;
      break;
    case FixingPrice:
      return StatisticsType::CLOSE_PRICE;
    case NULL_VALUE:
      break;
  }
  return {};
}

inline StatisticsType map(cme_mdp::MDEntryTypeStatistics::Value value) {
  switch (value) {
    using enum cme_mdp::MDEntryTypeStatistics::Value;
    case OpenPrice:
      return StatisticsType::OPEN_PRICE;
      break;
    case HighTrade:
      return StatisticsType::HIGHEST_TRADED_PRICE;
      break;
    case LowTrade:
      return StatisticsType::LOWEST_TRADED_PRICE;
      break;
    case VWAP:
      break;
    case HighestBid:
      break;
    case LowestOffer:
      break;
    case NULL_VALUE:
      break;
  }
  return {};
}

/*
START_OF_DAY,         //!< No matching, no order actions
PRE_OPEN,             //!< No matching, all order actions
PRE_OPEN_NO_CANCEL,   //!< No matching, only new orders
PRE_OPEN_FREEZE,      //!< Matching, no order actions
OPEN,                 //!< Matching, all order actions
FAST_MARKET,          //!< Same as Open, some settings could be relaxed by the exchange
HALT,                 //!< No matching, only order cancellation
CLOSE_NOT_FINAL,      //!< Same as Close, state required to support mid-session PreOpen
PRE_CLOSE,            //!< No matching, all order actions
PRE_CLOSE_NO_CANCEL,  //!< No matching, only new orders
PRE_CLOSE_FREEZE,     //!< Matching, no order actions
CLOSE,                //!< No matching, no order actions, good-for-day orders automatically canceled
POST_CLOSE,           //!< No matching, all order actions (only with next-trading-day validity)
END_OF_DAY,           //!< No matching, no order actions
*/
inline TradingStatus map_security_trading_status(cme_mdp::SecurityTradingStatus::Value value) {
  switch (value) {
    using enum cme_mdp::SecurityTradingStatus::Value;
    case TradingHalt:
      return TradingStatus::HALT;
    case Close:
      return TradingStatus::CLOSE;
    case NewPriceIndication:  // ???
      break;
    case ReadyToTrade:
      return TradingStatus::OPEN;
    case NotAvailableForTrading:
      return TradingStatus::CLOSE;  // ???
    case UnknownorInvalid:
      break;
    case PreOpen:
      return TradingStatus::PRE_OPEN;
    case PreCross:
      break;
    case Cross:
      break;
    case PostClose:
      return TradingStatus::POST_CLOSE;
    case NoChange:
      return TradingStatus::START_OF_DAY;  // ???
    case PrivateWorkup:
      break;
    case PublicWorkup:
      break;
    case NULL_VALUE:
      break;
  }
  return {};
}

inline UpdateAction map(cme_mdp::MDUpdateAction::Value value) {
  using namespace std::literals;
  switch (value) {
    using enum cme_mdp::MDUpdateAction::Value;
    case New:
      return UpdateAction::NEW;
    case Change:
      return UpdateAction::CHANGE;
    case Delete:
      return UpdateAction::DELETE;
    case DeleteThru:
      log::warn("+++ USING DELETE_THRU +++"sv);
      break;
    case DeleteFrom:
      log::warn("+++ USING DELETE_FROM +++"sv);
      break;
    case Overlay:
      log::warn("+++ USING OVERLAY +++"sv);
      break;
    case NULL_VALUE:
      break;
  }
  return {};
}

inline UpdateAction map(cme_mdp::OrderUpdateAction::Value value) {
  using namespace std::literals;
  switch (value) {
    using enum cme_mdp::OrderUpdateAction::Value;
    case New:
      return UpdateAction::NEW;
    case Update:
      return UpdateAction::CHANGE;
    case Delete:
      return UpdateAction::DELETE;
    case NULL_VALUE:
      break;
  }
  return {};
}

inline Side map_book_side(cme_mdp::Side::Value value) {
  switch (value) {
    using enum cme_mdp::Side::Value;
    case Buy:
      return Side::BUY;
    case Sell:
      return Side::SELL;
    case NULL_VALUE:
      return Side::UNDEFINED;
  }
  return {};
}

template <typename T>
size_t compute_length(T &);

template <>
inline size_t compute_length(cme_mdp::MessageHeader &value) {
  return value.encodedLength();
}

// types

template <typename T>
inline T get_int(T value, T null_value) {
  if (value != null_value)
    return value;
  return T{};
}

inline double get_double(cme_mdp::Decimal9 const &value) {
  auto mantissa = value.mantissa();
  return static_cast<double>(mantissa) * std::pow(10.0, value.exponent());
}

inline double get_double(cme_mdp::Decimal9NULL const &value) {
  auto mantissa = value.mantissa();
  if (mantissa != value.mantissaNullValue())
    return static_cast<double>(mantissa) * std::pow(10.0, value.exponent());
  return NaN;
}

inline double get_double(cme_mdp::DecimalQty const &value) {
  auto mantissa = value.mantissa();
  if (mantissa != value.mantissaNullValue())
    return static_cast<double>(mantissa) * std::pow(10.0, value.exponent());
  return NaN;
}

inline double get_double(cme_mdp::PRICE9 const &value) {
  auto mantissa = value.mantissa();
  return static_cast<double>(mantissa) * std::pow(10.0, value.exponent());
}

inline double get_double(cme_mdp::PRICENULL9 const &value) {
  auto mantissa = value.mantissa();
  if (mantissa != value.mantissaNullValue())
    return static_cast<double>(mantissa) * std::pow(10.0, value.exponent());
  return NaN;
}

inline std::string_view get_string_view(char const *buffer, size_t length) {
  for (auto iter = buffer; iter < (buffer + length); ++iter)
    if (!iter || (*iter) == '\0')
      return {buffer, static_cast<size_t>(iter - buffer)};
  return {buffer, length};
}

template <typename T>
struct Group final {
  explicit Group(T &value) : value_(value) {}
  auto format_to(auto &context) {
    using namespace std::literals;
    if (value_.count())
      fmt::format_to(context.out(), "{}"sv, fmt::join(roq::core::sbe::iterator{value_}, roq::core::sbe::sentinel{}, ", "sv));
    return context.out();
  }

 private:
  T &value_;
};
}  // namespace mdp
}  // namespace cme
}  // namespace roq

// header

template <>
struct fmt::formatter<cme_mdp::MessageHeader> {
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(cme_mdp::MessageHeader const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(blockLength={}, )"
        R"(templateId={}, )"
        R"(schemaId={}, )"
        R"(version={})"
        R"(}})"sv,
        value.blockLength(),
        value.templateId(),
        value.schemaId(),
        value.version());
  }
};

// types

template <>
struct fmt::formatter<cme_mdp::Decimal9> {
  using value_type = cme_mdp::Decimal9;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, roq::cme::mdp::get_double(value));
  }
};

template <>
struct fmt::formatter<cme_mdp::Decimal9NULL> {
  using value_type = cme_mdp::Decimal9NULL;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, roq::cme::mdp::get_double(value));
  }
};

template <>
struct fmt::formatter<cme_mdp::DecimalQty> {
  using value_type = cme_mdp::DecimalQty;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, roq::cme::mdp::get_double(value));
  }
};

template <>
struct fmt::formatter<cme_mdp::PRICE9> {
  using value_type = cme_mdp::PRICE9;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, roq::cme::mdp::get_double(value));
  }
};

template <>
struct fmt::formatter<cme_mdp::PRICENULL9> {
  using value_type = cme_mdp::PRICENULL9;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, roq::cme::mdp::get_double(value));
  }
};

// enum

template <>
struct fmt::formatter<cme_mdp::AggressorFlag::Value> {
  using value_type = cme_mdp::AggressorFlag::Value;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, magic_enum::enum_name(value));
  }
};

template <>
struct fmt::formatter<cme_mdp::AggressorSide::Value> {
  using value_type = cme_mdp::AggressorSide::Value;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, magic_enum::enum_name(value));
  }
};

template <>
struct fmt::formatter<cme_mdp::EventType::Value> {
  using value_type = cme_mdp::EventType::Value;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, magic_enum::enum_name(value));
  }
};

template <>
struct fmt::formatter<cme_mdp::HaltReason::Value> {
  using value_type = cme_mdp::HaltReason::Value;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, magic_enum::enum_name(value));
  }
};

template <>
struct fmt::formatter<cme_mdp::LegSide::Value> {
  using value_type = cme_mdp::LegSide::Value;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, magic_enum::enum_name(value));
  }
};

template <>
struct fmt::formatter<cme_mdp::MDEntryType::Value> {
  using value_type = cme_mdp::MDEntryType::Value;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, magic_enum::enum_name(value));
  }
};

template <>
struct fmt::formatter<cme_mdp::MDEntryTypeBook::Value> {
  using value_type = cme_mdp::MDEntryTypeBook::Value;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, magic_enum::enum_name(value));
  }
};

template <>
struct fmt::formatter<cme_mdp::MDEntryTypeDailyStatistics::Value> {
  using value_type = cme_mdp::MDEntryTypeDailyStatistics::Value;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, magic_enum::enum_name(value));
  }
};

template <>
struct fmt::formatter<cme_mdp::MDEntryTypeStatistics::Value> {
  using value_type = cme_mdp::MDEntryTypeStatistics::Value;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, magic_enum::enum_name(value));
  }
};

template <>
struct fmt::formatter<cme_mdp::MDUpdateAction::Value> {
  using value_type = cme_mdp::MDUpdateAction::Value;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, magic_enum::enum_name(value));
  }
};

template <>
struct fmt::formatter<cme_mdp::MoneyOrPar::Value> {
  using value_type = cme_mdp::MoneyOrPar::Value;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, magic_enum::enum_name(value));
  }
};

template <>
struct fmt::formatter<cme_mdp::OpenCloseSettlFlag::Value> {
  using value_type = cme_mdp::OpenCloseSettlFlag::Value;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, magic_enum::enum_name(value));
  }
};

template <>
struct fmt::formatter<cme_mdp::OrderUpdateAction::Value> {
  using value_type = cme_mdp::OrderUpdateAction::Value;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, magic_enum::enum_name(value));
  }
};

template <>
struct fmt::formatter<cme_mdp::PriceSource::Value> {
  using value_type = cme_mdp::PriceSource::Value;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, magic_enum::enum_name(value));
  }
};

template <>
struct fmt::formatter<cme_mdp::PutOrCall::Value> {
  using value_type = cme_mdp::PutOrCall::Value;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, magic_enum::enum_name(value));
  }
};

template <>
struct fmt::formatter<cme_mdp::RepoSubType::Value> {
  using value_type = cme_mdp::RepoSubType::Value;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, magic_enum::enum_name(value));
  }
};

template <>
struct fmt::formatter<cme_mdp::SecurityAltIDSource::Value> {
  using value_type = cme_mdp::SecurityAltIDSource::Value;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, magic_enum::enum_name(value));
  }
};

template <>
struct fmt::formatter<cme_mdp::SecurityTradingEvent::Value> {
  using value_type = cme_mdp::SecurityTradingEvent::Value;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, magic_enum::enum_name(value));
  }
};

template <>
struct fmt::formatter<cme_mdp::SecurityTradingStatus::Value> {
  using value_type = cme_mdp::SecurityTradingStatus::Value;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, magic_enum::enum_name(value));
  }
};

template <>
struct fmt::formatter<cme_mdp::SecurityUpdateAction::Value> {
  using value_type = cme_mdp::SecurityUpdateAction::Value;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, magic_enum::enum_name(value));
  }
};

template <>
struct fmt::formatter<cme_mdp::Side::Value> {
  using value_type = cme_mdp::Side::Value;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, magic_enum::enum_name(value));
  }
};

template <>
struct fmt::formatter<cme_mdp::WorkupTradingStatus::Value> {
  using value_type = cme_mdp::WorkupTradingStatus::Value;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, magic_enum::enum_name(value));
  }
};

// choice

template <>
struct fmt::formatter<cme_mdp::InstAttribValue> {
  using value_type = cme_mdp::InstAttribValue;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, value.rawValue());
  }
};

template <>
struct fmt::formatter<cme_mdp::MatchEventIndicator> {
  using value_type = cme_mdp::MatchEventIndicator;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, value.rawValue());
  }
};

template <>
struct fmt::formatter<cme_mdp::SettlPriceType> {
  using value_type = cme_mdp::SettlPriceType;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, value.rawValue());
  }
};

// complex

template <>
struct fmt::formatter<cme_mdp::MaturityMonthYear> {
  using value_type = cme_mdp::MaturityMonthYear;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(year={}, )"
        R"(month={}, )"
        R"(day={}, )"
        R"(week={})"
        R"(}}))"sv,
        value.year(),
        value.month(),
        value.day(),
        value.week());
  }
};

// group

template <typename T>
struct fmt::formatter<roq::cme::mdp::Group<T>> {
  using value_type = roq::cme::mdp::Group<T>;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type &value, format_context &context) const { return value.format_to(context); }
};
