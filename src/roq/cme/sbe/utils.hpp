/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <fmt/chrono.h>
#include <fmt/format.h>

#include <cme_mdp/MessageHeader.h>

#include <cme_mdp/Side.h>

#include <cme_mdp/MDInstrumentDefinitionFX63.h>

#include <magic_enum.hpp>

#include "roq/api.hpp"

#include "roq/core/sbe/iterator.hpp"

#include "roq/logging.hpp"

namespace roq {
namespace cme {
namespace sbe {

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
  return Side::UNDEFINED;
}

template <typename T>
size_t compute_length(T &);

template <>
inline size_t compute_length(cme_mdp::MessageHeader &value) {
  return value.encodedLength();
}

// types

inline double map(cme_mdp::Decimal9 const &value) {
  auto mantissa = value.mantissa();
  auto exponent = value.exponent();
  return static_cast<double>(mantissa) * std::pow(10.0, exponent);
}

inline double map(cme_mdp::Decimal9NULL const &value) {
  auto mantissa = value.mantissa();
  if (mantissa == value.mantissaNullValue())
    return roq::NaN;
  auto exponent = value.exponent();
  return static_cast<double>(mantissa) * std::pow(10.0, exponent);
}

inline double map(cme_mdp::DecimalQty const &value) {
  auto mantissa = value.mantissa();
  if (mantissa == value.mantissaNullValue())
    return roq::NaN;
  auto exponent = value.exponent();
  return static_cast<double>(mantissa) * std::pow(10.0, exponent);
}

inline double map(cme_mdp::PRICE9 const &value) {
  auto mantissa = value.mantissa();
  auto exponent = value.exponent();
  return static_cast<double>(mantissa) * std::pow(10.0, exponent);
}

inline double map(cme_mdp::PRICENULL9 const &value) {
  auto mantissa = value.mantissa();
  if (mantissa == value.mantissaNullValue())
    return roq::NaN;
  auto exponent = value.exponent();
  return static_cast<double>(mantissa) * std::pow(10.0, exponent);
}

}  // namespace sbe
}  // namespace cme
}  // namespace roq

// header

template <>
struct fmt::formatter<cme_mdp::MessageHeader> {
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(cme_mdp::MessageHeader const &value, Context &context) const {
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
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type const &value, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, roq::cme::sbe::map(value));
  }
};

template <>
struct fmt::formatter<cme_mdp::Decimal9NULL> {
  using value_type = cme_mdp::Decimal9NULL;
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type const &value, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, roq::cme::sbe::map(value));
  }
};

template <>
struct fmt::formatter<cme_mdp::DecimalQty> {
  using value_type = cme_mdp::DecimalQty;
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type const &value, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, roq::cme::sbe::map(value));
  }
};

template <>
struct fmt::formatter<cme_mdp::PRICE9> {
  using value_type = cme_mdp::PRICE9;
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type const &value, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, roq::cme::sbe::map(value));
  }
};

template <>
struct fmt::formatter<cme_mdp::PRICENULL9> {
  using value_type = cme_mdp::PRICENULL9;
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type const &value, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, roq::cme::sbe::map(value));
  }
};

// enum

template <>
struct fmt::formatter<cme_mdp::AggressorFlag::Value> {
  using value_type = cme_mdp::AggressorFlag::Value;
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type const &value, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, magic_enum::enum_name(value));
  }
};

template <>
struct fmt::formatter<cme_mdp::AggressorSide::Value> {
  using value_type = cme_mdp::AggressorSide::Value;
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type const &value, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, magic_enum::enum_name(value));
  }
};

template <>
struct fmt::formatter<cme_mdp::EventType::Value> {
  using value_type = cme_mdp::EventType::Value;
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type const &value, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, magic_enum::enum_name(value));
  }
};

template <>
struct fmt::formatter<cme_mdp::HaltReason::Value> {
  using value_type = cme_mdp::HaltReason::Value;
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type const &value, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, magic_enum::enum_name(value));
  }
};

template <>
struct fmt::formatter<cme_mdp::LegSide::Value> {
  using value_type = cme_mdp::LegSide::Value;
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type const &value, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, magic_enum::enum_name(value));
  }
};

template <>
struct fmt::formatter<cme_mdp::MDEntryType::Value> {
  using value_type = cme_mdp::MDEntryType::Value;
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type const &value, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, magic_enum::enum_name(value));
  }
};

template <>
struct fmt::formatter<cme_mdp::MDEntryTypeBook::Value> {
  using value_type = cme_mdp::MDEntryTypeBook::Value;
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type const &value, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, magic_enum::enum_name(value));
  }
};

template <>
struct fmt::formatter<cme_mdp::MDEntryTypeDailyStatistics::Value> {
  using value_type = cme_mdp::MDEntryTypeDailyStatistics::Value;
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type const &value, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, magic_enum::enum_name(value));
  }
};

template <>
struct fmt::formatter<cme_mdp::MDEntryTypeStatistics::Value> {
  using value_type = cme_mdp::MDEntryTypeStatistics::Value;
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type const &value, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, magic_enum::enum_name(value));
  }
};

template <>
struct fmt::formatter<cme_mdp::MDUpdateAction::Value> {
  using value_type = cme_mdp::MDUpdateAction::Value;
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type const &value, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, magic_enum::enum_name(value));
  }
};

template <>
struct fmt::formatter<cme_mdp::MoneyOrPar::Value> {
  using value_type = cme_mdp::MoneyOrPar::Value;
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type const &value, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, magic_enum::enum_name(value));
  }
};

template <>
struct fmt::formatter<cme_mdp::OpenCloseSettlFlag::Value> {
  using value_type = cme_mdp::OpenCloseSettlFlag::Value;
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type const &value, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, magic_enum::enum_name(value));
  }
};

template <>
struct fmt::formatter<cme_mdp::OrderUpdateAction::Value> {
  using value_type = cme_mdp::OrderUpdateAction::Value;
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type const &value, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, magic_enum::enum_name(value));
  }
};

template <>
struct fmt::formatter<cme_mdp::PriceSource::Value> {
  using value_type = cme_mdp::PriceSource::Value;
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type const &value, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, magic_enum::enum_name(value));
  }
};

template <>
struct fmt::formatter<cme_mdp::PutOrCall::Value> {
  using value_type = cme_mdp::PutOrCall::Value;
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type const &value, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, magic_enum::enum_name(value));
  }
};

template <>
struct fmt::formatter<cme_mdp::RepoSubType::Value> {
  using value_type = cme_mdp::RepoSubType::Value;
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type const &value, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, magic_enum::enum_name(value));
  }
};

template <>
struct fmt::formatter<cme_mdp::SecurityAltIDSource::Value> {
  using value_type = cme_mdp::SecurityAltIDSource::Value;
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type const &value, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, magic_enum::enum_name(value));
  }
};

template <>
struct fmt::formatter<cme_mdp::SecurityTradingEvent::Value> {
  using value_type = cme_mdp::SecurityTradingEvent::Value;
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type const &value, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, magic_enum::enum_name(value));
  }
};

template <>
struct fmt::formatter<cme_mdp::SecurityTradingStatus::Value> {
  using value_type = cme_mdp::SecurityTradingStatus::Value;
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type const &value, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, magic_enum::enum_name(value));
  }
};

template <>
struct fmt::formatter<cme_mdp::SecurityUpdateAction::Value> {
  using value_type = cme_mdp::SecurityUpdateAction::Value;
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type const &value, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, magic_enum::enum_name(value));
  }
};

template <>
struct fmt::formatter<cme_mdp::Side::Value> {
  using value_type = cme_mdp::Side::Value;
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type const &value, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, magic_enum::enum_name(value));
  }
};

template <>
struct fmt::formatter<cme_mdp::WorkupTradingStatus::Value> {
  using value_type = cme_mdp::WorkupTradingStatus::Value;
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type const &value, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, magic_enum::enum_name(value));
  }
};

// choice

template <>
struct fmt::formatter<cme_mdp::InstAttribValue> {
  using value_type = cme_mdp::InstAttribValue;
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type const &value, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, value.rawValue());
  }
};

template <>
struct fmt::formatter<cme_mdp::MatchEventIndicator> {
  using value_type = cme_mdp::MatchEventIndicator;
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type const &value, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, value.rawValue());
  }
};

template <>
struct fmt::formatter<cme_mdp::SettlPriceType> {
  using value_type = cme_mdp::SettlPriceType;
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type const &value, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, value.rawValue());
  }
};

// complex

template <>
struct fmt::formatter<cme_mdp::MaturityMonthYear> {
  using value_type = cme_mdp::MaturityMonthYear;
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type const &value, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({}-{}-{})"sv, value.year(), value.month(), value.year());
  }
};
