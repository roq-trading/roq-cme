/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include "roq/flags/args.hpp"

#include "roq/cme/channel_tester/flags/flags.hpp"

namespace roq {
namespace cme {
namespace channel_tester {

struct Settings final : public flags::Flags {
  explicit Settings(args::Parser const &);
};

}  // namespace channel_tester
}  // namespace cme
}  // namespace roq
