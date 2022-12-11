/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include "roq/utils/compare.hpp"

namespace roq {
namespace cme {

inline double compute_contracts_multiplier(double contract_size) {
  return utils::is_zero(contract_size) ? 1.0 : (1.0 / contract_size);
}

}  // namespace cme
}  // namespace roq
