/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <string>

#include "roq/api.hpp"

namespace roq {
namespace deribit {

struct Instrument final {
  Symbol symbol;
  double contract_size = NaN;
  double multiplier = 1.0;
};

}  // namespace deribit
}  // namespace roq
