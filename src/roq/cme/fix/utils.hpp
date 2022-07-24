/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/api.hpp"

#include "roq/deribit/fix/adv_order_type.hpp"

namespace roq {
namespace deribit {
namespace fix {

inline void update(AdvOrderType &result, std::string_view const &value) {
  result = parse_adv_order_type(value);
}

SecurityType map_security_type(std::string_view const &value);

Error map_error(std::string_view const &value);

std::string_view map(Mask<ExecutionInstruction> const &);

extern Error reject_to_error(std::string_view const &reason, std::string_view const &text);

}  // namespace fix
}  // namespace deribit
}  // namespace roq
