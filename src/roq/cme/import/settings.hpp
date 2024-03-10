/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include "roq/flags/args.hpp"

#include "roq/cme/import/flags/flags.hpp"

namespace roq {
namespace cme {
namespace import {

struct Settings final : public flags::Flags {
  explicit Settings(args::Parser const &);
};

}  // namespace import
}  // namespace cme
}  // namespace roq
