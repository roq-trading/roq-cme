/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <cstdint>

namespace roq {
namespace cme {
namespace gateway {

enum class OrderEntryState : uint8_t {
  UNDEFINED = 0,
  ORDERS,
  DONE,
};

}  // namespace gateway
}  // namespace cme
}  // namespace roq
