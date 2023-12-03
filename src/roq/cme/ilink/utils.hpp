/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include <fmt/chrono.h>
#include <fmt/format.h>

#include <cme_ilink/ExecutionReportNew522.h>
#include <cme_ilink/MessageHeader.h>

#include <cme_ilink/BooleanFlag.h>
#include <cme_ilink/BooleanNULL.h>
#include <cme_ilink/ExecMode.h>
#include <cme_ilink/ExecReason.h>
#include <cme_ilink/ExpCycle.h>
#include <cme_ilink/FTI.h>
#include <cme_ilink/KeepAliveLapsed.h>
#include <cme_ilink/ManualOrdInd.h>
#include <cme_ilink/ManualOrdIndReq.h>
#include <cme_ilink/OrdStatusTrd.h>
#include <cme_ilink/OrderStatus.h>
#include <cme_ilink/OrderType.h>
#include <cme_ilink/SecRspTyp.h>
#include <cme_ilink/ShortSaleType.h>
#include <cme_ilink/SideReq.h>
#include <cme_ilink/SplitMsg.h>
#include <cme_ilink/TimeInForce.h>

#include <magic_enum.hpp>

#include "roq/api.hpp"

#include "roq/core/sbe/iterator.hpp"

namespace roq {
namespace cme {
namespace ilink {

// types

template <typename T>
inline T get_int(T value, T null_value) {
  if (value != null_value)
    return value;
  return T{};
}

inline double get_double(cme_ilink::PRICE9 const &value) {
  auto mantissa = value.mantissa();
  return static_cast<double>(mantissa) * std::pow(10.0, value.exponent());
}

inline double get_double(cme_ilink::PRICENULL9 const &value) {
  auto mantissa = value.mantissa();
  if (mantissa != value.mantissaNullValue())
    return static_cast<double>(mantissa) * std::pow(10.0, value.exponent());
  return NaN;
}

inline double get_double(cme_ilink::Decimal32NULL const &value) {
  auto mantissa = value.mantissa();
  if (mantissa != value.mantissaNullValue())
    return static_cast<double>(mantissa) * std::pow(10.0, value.exponent());
  return NaN;
}

inline double get_double(cme_ilink::Decimal64NULL const &value) {
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
      fmt::format_to(
          context.out(), "{}"sv, fmt::join(roq::core::sbe::iterator{value_}, roq::core::sbe::sentinel{}, ", "sv));
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
struct fmt::formatter<cme_ilink::MessageHeader> {
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(cme_ilink::MessageHeader const &value, Context &context) const {
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
struct fmt::formatter<cme_ilink::PRICE9> {
  using value_type = cme_ilink::PRICE9;
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type const &value, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, roq::cme::ilink::get_double(value));
  }
};

template <>
struct fmt::formatter<cme_ilink::PRICENULL9> {
  using value_type = cme_ilink::PRICENULL9;
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type const &value, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, roq::cme::ilink::get_double(value));
  }
};

template <>
struct fmt::formatter<cme_ilink::Decimal32NULL> {
  using value_type = cme_ilink::Decimal32NULL;
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type const &value, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, roq::cme::ilink::get_double(value));
  }
};

template <>
struct fmt::formatter<cme_ilink::Decimal64NULL> {
  using value_type = cme_ilink::Decimal64NULL;
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type const &value, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), R"({})"sv, roq::cme::ilink::get_double(value));
  }
};

// complex

template <>
struct fmt::formatter<cme_ilink::MaturityMonthYear> {
  using value_type = cme_ilink::MaturityMonthYear;
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
struct fmt::formatter<cme_ilink::ExecInst> {
  using value_type = cme_ilink::ExecInst;
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

// BooleanFlag

template <>
struct fmt::formatter<cme_ilink::BooleanFlag::Value> {
  using value_type = cme_ilink::BooleanFlag::Value;
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type const &value, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), "{}"sv, magic_enum::enum_name(value));
  }
};

// BooleanNULL

template <>
struct fmt::formatter<cme_ilink::BooleanNULL::Value> {
  using value_type = cme_ilink::BooleanNULL::Value;
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type const &value, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), "{}"sv, magic_enum::enum_name(value));
  }
};

// ExecMode

template <>
struct fmt::formatter<cme_ilink::ExecMode::Value> {
  using value_type = cme_ilink::ExecMode::Value;
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type const &value, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), "{}"sv, magic_enum::enum_name(value));
  }
};

// ExecReason

template <>
struct fmt::formatter<cme_ilink::ExecReason::Value> {
  using value_type = cme_ilink::ExecReason::Value;
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type const &value, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), "{}"sv, magic_enum::enum_name(value));
  }
};

// ExpCycle

template <>
struct fmt::formatter<cme_ilink::ExpCycle::Value> {
  using value_type = cme_ilink::ExpCycle::Value;
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type const &value, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), "{}"sv, magic_enum::enum_name(value));
  }
};

// FTI

template <>
struct fmt::formatter<cme_ilink::FTI::Value> {
  using value_type = cme_ilink::FTI::Value;
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type const &value, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), "{}"sv, magic_enum::enum_name(value));
  }
};

// KeepAliveLapsed

template <>
struct fmt::formatter<cme_ilink::KeepAliveLapsed::Value> {
  using value_type = cme_ilink::KeepAliveLapsed::Value;
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type const &value, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), "{}"sv, magic_enum::enum_name(value));
  }
};

// ManualOrdInd

template <>
struct fmt::formatter<cme_ilink::ManualOrdInd::Value> {
  using value_type = cme_ilink::ManualOrdInd::Value;
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type const &value, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), "{}"sv, magic_enum::enum_name(value));
  }
};

// ManualOrdIndReq

template <>
struct fmt::formatter<cme_ilink::ManualOrdIndReq::Value> {
  using value_type = cme_ilink::ManualOrdIndReq::Value;
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type const &value, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), "{}"sv, magic_enum::enum_name(value));
  }
};

// OrdStatusTrd

template <>
struct fmt::formatter<cme_ilink::OrdStatusTrd::Value> {
  using value_type = cme_ilink::OrdStatusTrd::Value;
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type const &value, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), "{}"sv, magic_enum::enum_name(value));
  }
};

// OrderStatus

template <>
struct fmt::formatter<cme_ilink::OrderStatus::Value> {
  using value_type = cme_ilink::OrderStatus::Value;
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type const &value, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), "{}"sv, magic_enum::enum_name(value));
  }
};

// OrderType

template <>
struct fmt::formatter<cme_ilink::OrderType::Value> {
  using value_type = cme_ilink::OrderType::Value;
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type const &value, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), "{}"sv, magic_enum::enum_name(value));
  }
};

// SecRspTyp

template <>
struct fmt::formatter<cme_ilink::SecRspTyp::Value> {
  using value_type = cme_ilink::SecRspTyp::Value;
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type const &value, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), "{}"sv, magic_enum::enum_name(value));
  }
};

// ShortSaleType

template <>
struct fmt::formatter<cme_ilink::ShortSaleType::Value> {
  using value_type = cme_ilink::ShortSaleType::Value;
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type const &value, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), "{}"sv, magic_enum::enum_name(value));
  }
};

// SideReq

template <>
struct fmt::formatter<cme_ilink::SideReq::Value> {
  using value_type = cme_ilink::SideReq::Value;
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type const &value, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), "{}"sv, magic_enum::enum_name(value));
  }
};

// SplitMsg

template <>
struct fmt::formatter<cme_ilink::SplitMsg::Value> {
  using value_type = cme_ilink::SplitMsg::Value;
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type const &value, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), "{}"sv, magic_enum::enum_name(value));
  }
};

// TimeInForce

template <>
struct fmt::formatter<cme_ilink::TimeInForce::Value> {
  using value_type = cme_ilink::TimeInForce::Value;
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type const &value, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(context.out(), "{}"sv, magic_enum::enum_name(value));
  }
};
