/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <fmt/chrono.h>
#include <fmt/format.h>

#include <cme/sbe/mdp/MessageHeader.h>

#include <cme/sbe/mdp/AggressorSide.h>
#include <cme/sbe/mdp/Side.h>

#include <cme/sbe/mdp/MDInstrumentDefinitionFX63.h>

#include <magic_enum/magic_enum.hpp>

#include "roq/api.hpp"

#include "roq/core/sbe/iterator.hpp"

#include "roq/cme/mdp/map.hpp"

namespace roq {
namespace cme {
namespace mdp {

template <typename T>
size_t compute_length(T &);

template <>
inline size_t compute_length(::cme::sbe::mdp::MessageHeader &value) {
  return value.encodedLength();
}

template <typename T>
struct Group final {
  explicit Group(T &value) : value_(value) {}
  auto format_to(auto &context) {
    using namespace std::literals;
    if (value_.count()) {
      fmt::format_to(context.out(), "{}"sv, fmt::join(roq::core::sbe::iterator{value_}, roq::core::sbe::sentinel{}, ", "sv));
    }
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
struct fmt::formatter<::cme::sbe::mdp::MessageHeader> {
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(::cme::sbe::mdp::MessageHeader const &value, format_context &context) const {
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
struct fmt::formatter<::cme::sbe::mdp::Decimal9> {
  using value_type = ::cme::sbe::mdp::Decimal9;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, roq::map(value).template get<double>());
  }
};

template <>
struct fmt::formatter<::cme::sbe::mdp::Decimal9NULL> {
  using value_type = ::cme::sbe::mdp::Decimal9NULL;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, roq::map(value).template get<double>());
  }
};

template <>
struct fmt::formatter<::cme::sbe::mdp::DecimalQty> {
  using value_type = ::cme::sbe::mdp::DecimalQty;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, roq::map(value).template get<double>());
  }
};

template <>
struct fmt::formatter<::cme::sbe::mdp::PRICE9> {
  using value_type = ::cme::sbe::mdp::PRICE9;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, roq::map(value).template get<double>());
  }
};

template <>
struct fmt::formatter<::cme::sbe::mdp::PRICENULL9> {
  using value_type = ::cme::sbe::mdp::PRICENULL9;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, roq::map(value).template get<double>());
  }
};

// choice

template <>
struct fmt::formatter<::cme::sbe::mdp::InstAttribValue> {
  using value_type = ::cme::sbe::mdp::InstAttribValue;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, value.rawValue());
  }
};

template <>
struct fmt::formatter<::cme::sbe::mdp::MatchEventIndicator> {
  using value_type = ::cme::sbe::mdp::MatchEventIndicator;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, value.rawValue());
  }
};

template <>
struct fmt::formatter<::cme::sbe::mdp::SettlPriceType> {
  using value_type = ::cme::sbe::mdp::SettlPriceType;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, value.rawValue());
  }
};

// complex

template <>
struct fmt::formatter<::cme::sbe::mdp::MaturityMonthYear> {
  using value_type = ::cme::sbe::mdp::MaturityMonthYear;
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
