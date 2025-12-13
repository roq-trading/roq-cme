/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include "roq/flags/args.hpp"

#include "roq/cme/filter/flags/cme.hpp"
#include "roq/cme/filter/flags/flags.hpp"

namespace roq {
namespace cme {
namespace filter {

struct Settings final : public flags::Flags {
  explicit Settings(args::Parser const &);

  flags::CME cme;
};

}  // namespace filter
}  // namespace cme
}  // namespace roq
