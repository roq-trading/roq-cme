/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/cme/import/settings.hpp"

using namespace std::literals;

namespace roq {
namespace cme {
namespace import {

// === IMPLEMENTATION ===

Settings::Settings(args::Parser const &)
    : flags::Flags{flags::Flags::create()}, cme{flags::CME::create()}, event_log{flags::EventLog::create()},
      misc{flags::Misc::create()}, test{flags::Test::create()} {
}

}  // namespace import
}  // namespace cme
}  // namespace roq
