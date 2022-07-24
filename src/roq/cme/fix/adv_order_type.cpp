/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/deribit/fix/adv_order_type.hpp"

using namespace std::literals;

namespace roq {
namespace deribit {
namespace fix {

std::string_view EnumNameAdvOrderType(AdvOrderType const &value) {
  switch (value) {
    using enum AdvOrderType;
    case IMPLIED_VOLATILITY_ORDER:
      return "IMPLIED_VOLATILITY_ORDER"sv;
    case USD_ORDER:
      return "USD_ORDER"sv;
    default:
      return "UNKNOWN"sv;
  }
}

AdvOrderType parse_adv_order_type(std::string_view const &value) {
  if (std::empty(value))
    return AdvOrderType::UNKNOWN;
  switch (std::data(value)[0]) {
    case '0':
      return AdvOrderType::IMPLIED_VOLATILITY_ORDER;
    case '1':
      return AdvOrderType::USD_ORDER;
    default:
      return AdvOrderType::UNKNOWN;
  }
}

}  // namespace fix
}  // namespace deribit
}  // namespace roq
