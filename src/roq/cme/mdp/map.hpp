/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <cme_mdp/AggressorSide.h>
#include <cme_mdp/MDEntryTypeBook.h>
#include <cme_mdp/MDEntryTypeDailyStatistics.h>
#include <cme_mdp/MDEntryTypeStatistics.h>
#include <cme_mdp/MDUpdateAction.h>
#include <cme_mdp/OrderUpdateAction.h>
#include <cme_mdp/SecurityTradingStatus.h>
#include <cme_mdp/Side.h>

#include <cme_mdp/Decimal9.h>
#include <cme_mdp/Decimal9NULL.h>
#include <cme_mdp/DecimalQty.h>
#include <cme_mdp/PRICE9.h>
#include <cme_mdp/PRICENULL9.h>

#include <cmath>

#include "roq/side.hpp"
#include "roq/statistics_type.hpp"
#include "roq/trading_status.hpp"
#include "roq/update_action.hpp"

#include "roq/map.hpp"

namespace roq {

template <>
template <>
std::optional<Side> Map<cme_mdp::AggressorSide::Value>::helper() const;

template <>
template <>
std::optional<Side> Map<cme_mdp::MDEntryTypeBook::Value>::helper() const;

template <>
template <>
std::optional<StatisticsType> Map<cme_mdp::MDEntryTypeDailyStatistics::Value>::helper() const;

template <>
template <>
std::optional<StatisticsType> Map<cme_mdp::MDEntryTypeStatistics::Value>::helper() const;

template <>
template <>
std::optional<UpdateAction> Map<cme_mdp::MDUpdateAction::Value>::helper() const;

template <>
template <>
std::optional<UpdateAction> Map<cme_mdp::OrderUpdateAction::Value>::helper() const;

template <>
template <>
std::optional<TradingStatus> Map<cme_mdp::SecurityTradingStatus::Value>::helper() const;

template <>
template <>
std::optional<Side> Map<cme_mdp::Side::Value>::helper() const;

// int

// XXX FIXME TODO use Map
namespace cme {
namespace mdp {
template <typename T>
inline T get_int(T value, T null_value) {
  if (value != null_value) {
    return value;
  }
  return T{};
}
}  // namespace mdp
}  // namespace cme

// double

template <>
template <>
inline std::optional<double> Map<cme_mdp::Decimal9>::helper() const {
  auto &value = std::get<0>(args_);
  auto mantissa = value.mantissa();
  return static_cast<double>(mantissa) * std::pow(10.0, value.exponent());
}

template <>
template <>
inline std::optional<double> Map<cme_mdp::Decimal9NULL>::helper() const {
  auto &value = std::get<0>(args_);
  auto mantissa = value.mantissa();
  if (mantissa != value.mantissaNullValue()) {
    return static_cast<double>(mantissa) * std::pow(10.0, value.exponent());
  }
  return std::numeric_limits<double>::quiet_NaN();
}

template <>
template <>
inline std::optional<double> Map<cme_mdp::DecimalQty>::helper() const {
  auto &value = std::get<0>(args_);
  auto mantissa = value.mantissa();
  if (mantissa != value.mantissaNullValue()) {
    return static_cast<double>(mantissa) * std::pow(10.0, value.exponent());
  }
  return std::numeric_limits<double>::quiet_NaN();
}

template <>
template <>
inline std::optional<double> Map<cme_mdp::PRICE9>::helper() const {
  auto &value = std::get<0>(args_);
  auto mantissa = value.mantissa();
  return static_cast<double>(mantissa) * std::pow(10.0, value.exponent());
}

template <>
template <>
inline std::optional<double> Map<cme_mdp::PRICENULL9>::helper() const {
  auto &value = std::get<0>(args_);
  auto mantissa = value.mantissa();
  if (mantissa != value.mantissaNullValue()) {
    return static_cast<double>(mantissa) * std::pow(10.0, value.exponent());
  }
  return std::numeric_limits<double>::quiet_NaN();
}

// string

// XXX FIXME TODO use Map
namespace cme {
namespace mdp {
inline std::string_view get_string_view(char const *buffer, size_t length) {
  for (auto iter = buffer; iter < (buffer + length); ++iter) {
    if (!iter || (*iter) == '\0') {
      return {buffer, static_cast<size_t>(iter - buffer)};
    }
  }
  return {buffer, length};
}
}  // namespace mdp
}  // namespace cme

}  // namespace roq
