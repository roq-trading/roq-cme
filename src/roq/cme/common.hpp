/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/core/fix/common.hpp"

namespace roq {
namespace deribit {

constexpr auto FIX_VERSION = core::fix::Version::FIX_44;

constexpr std::string_view SENDER_COMP_ID = "ROQ_TRADING";
constexpr std::string_view TARGET_COMP_ID = "DERIBITSERVER";

}  // namespace deribit
}  // namespace roq
