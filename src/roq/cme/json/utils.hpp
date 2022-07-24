/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <chrono>

#include "roq/core/utility.hpp"

#include "roq/core/json/parser.hpp"

#include "roq/core/charconv/datetime.hpp"

#include "roq/deribit/json/direction.hpp"
#include "roq/deribit/json/state.hpp"

namespace roq {
namespace deribit {
namespace json {

template <typename T>
inline void update(T &result, core::json::Value const &value) {
  result = core::json::get<T>(value);
}

template <>
inline void update(double &result, core::json::Value const &value) {
  if (std::holds_alternative<std::string_view>(value)) {
    if (std::get<std::string_view>(value).compare("undefined") == 0) {
      result = NaN;
      return;
    }
  }
  result = core::json::get<double>(value);
}

template <>
inline void update(std::chrono::milliseconds &result, core::json::Value const &value) {
  result = std::chrono::milliseconds{core::json::get<uint64_t>(value)};
}

inline Side map(Direction direction) {
  switch (direction) {
    using enum Direction::type_t;
    case UNDEFINED:
      break;
    case UNKNOWN:
      break;
    case BUY:
      return Side::BUY;
    case SELL:
      return Side::SELL;
    case ZERO:
      return Side::UNDEFINED;
  }
  return Side::UNDEFINED;
}

inline TradingStatus map(State state) {
  switch (state) {
    using enum State::type_t;
    case UNDEFINED:
      break;
    case UNKNOWN:
      break;
    case CLOSED:
      return TradingStatus::OPEN;
    case OPEN:
      return TradingStatus::OPEN;
    case CREATED:     // XXX don't know how to map
    case SETTLED:     // XXX don't know how to map
    case TERMINATED:  // XXX don't know how to map
      break;
  }
  return TradingStatus::UNDEFINED;
}

}  // namespace json
}  // namespace deribit
}  // namespace roq
