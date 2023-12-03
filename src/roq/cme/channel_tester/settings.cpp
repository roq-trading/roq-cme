/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/cme/channel_tester/settings.hpp"

using namespace std::literals;

namespace roq {
namespace cme {
namespace channel_tester {

// === IMPLEMENTATION ===

Settings::Settings(args::Parser const &) : flags::Flags{flags::Flags::create()} {
}

}  // namespace channel_tester
}  // namespace cme
}  // namespace roq
