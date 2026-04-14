/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <fmt/chrono.h>
#include <fmt/format.h>

#include <magic_enum/magic_enum_format.hpp>

#include <cme/sbe/ilink/ExecutionReportNew522.h>
#include <cme/sbe/ilink/MessageHeader.h>

#include <cme/sbe/ilink/BooleanFlag.h>
#include <cme/sbe/ilink/BooleanNULL.h>
#include <cme/sbe/ilink/ExecMode.h>
#include <cme/sbe/ilink/ExecReason.h>
#include <cme/sbe/ilink/ExpCycle.h>
#include <cme/sbe/ilink/FTI.h>
#include <cme/sbe/ilink/KeepAliveLapsed.h>
#include <cme/sbe/ilink/ManualOrdInd.h>
#include <cme/sbe/ilink/ManualOrdIndReq.h>
#include <cme/sbe/ilink/OrdStatusTrd.h>
#include <cme/sbe/ilink/OrderStatus.h>
#include <cme/sbe/ilink/OrderType.h>
#include <cme/sbe/ilink/SecRspTyp.h>
#include <cme/sbe/ilink/ShortSaleType.h>
#include <cme/sbe/ilink/SideReq.h>
#include <cme/sbe/ilink/SplitMsg.h>
#include <cme/sbe/ilink/TimeInForce.h>

#include <magic_enum/magic_enum.hpp>

#include "roq/api.hpp"

#include "roq/core/sbe/iterator.hpp"

namespace roq {
namespace cme {
namespace ilink {

// types

template <typename T>
inline T get_int(T value, T null_value) {
  if (value != null_value) {
    return value;
  }
  return T{};
}

inline double get_double(::cme::sbe::ilink::PRICE9 const &value) {
  auto mantissa = value.mantissa();
  return static_cast<double>(mantissa) * std::pow(10.0, value.exponent());
}

inline double get_double(::cme::sbe::ilink::PRICENULL9 const &value) {
  auto mantissa = value.mantissa();
  if (mantissa != value.mantissaNullValue()) {
    return static_cast<double>(mantissa) * std::pow(10.0, value.exponent());
  }
  return NaN;
}

inline double get_double(::cme::sbe::ilink::Decimal32NULL const &value) {
  auto mantissa = value.mantissa();
  if (mantissa != value.mantissaNullValue()) {
    return static_cast<double>(mantissa) * std::pow(10.0, value.exponent());
  }
  return NaN;
}

inline double get_double(::cme::sbe::ilink::Decimal64NULL const &value) {
  auto mantissa = value.mantissa();
  if (mantissa != value.mantissaNullValue()) {
    return static_cast<double>(mantissa) * std::pow(10.0, value.exponent());
  }
  return NaN;
}

inline std::string_view get_string_view(char const *buffer, size_t length) {
  for (auto iter = buffer; iter < (buffer + length); ++iter) {
    if (!iter || (*iter) == '\0') {
      return {buffer, static_cast<size_t>(iter - buffer)};
    }
  }
  return {buffer, length};
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
}  // namespace ilink
}  // namespace cme
}  // namespace roq

// header

/*
template <>
struct fmt::formatter<::cme::sbe::ilink::MessageHeader> {
  constexpr auto parse(format_parse_context &context) {
    return std::begin(context);
  }
  auto format(::cme::sbe::ilink::MessageHeader const &value, format_context &context) const {
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
*/

// types

template <>
struct fmt::formatter<::cme::sbe::ilink::PRICE9> {
  using value_type = ::cme::sbe::ilink::PRICE9;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, roq::cme::ilink::get_double(value));
  }
};

template <>
struct fmt::formatter<::cme::sbe::ilink::PRICENULL9> {
  using value_type = ::cme::sbe::ilink::PRICENULL9;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, roq::cme::ilink::get_double(value));
  }
};

template <>
struct fmt::formatter<::cme::sbe::ilink::Decimal32NULL> {
  using value_type = ::cme::sbe::ilink::Decimal32NULL;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, roq::cme::ilink::get_double(value));
  }
};

template <>
struct fmt::formatter<::cme::sbe::ilink::Decimal64NULL> {
  using value_type = ::cme::sbe::ilink::Decimal64NULL;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, roq::cme::ilink::get_double(value));
  }
};

// complex

template <>
struct fmt::formatter<::cme::sbe::ilink::MaturityMonthYear> {
  using value_type = ::cme::sbe::ilink::MaturityMonthYear;
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

// note! set
template <>
struct fmt::formatter<::cme::sbe::ilink::ExecInst> {
  using value_type = ::cme::sbe::ilink::ExecInst;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, value.rawValue());
  }
};
