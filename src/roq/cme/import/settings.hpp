/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include "roq/flags/args.hpp"

#include "roq/cme/import/flags/cme.hpp"
#include "roq/cme/import/flags/event_log.hpp"
#include "roq/cme/import/flags/flags.hpp"
#include "roq/cme/import/flags/misc.hpp"
#include "roq/cme/import/flags/test.hpp"

namespace roq {
namespace cme {
namespace import {

struct Settings final : public flags::Flags {
  explicit Settings(args::Parser const &);

  flags::CME cme;
  flags::EventLog event_log;
  flags::Misc misc;
  flags::Test test;
};

}  // namespace import
}  // namespace cme
}  // namespace roq
