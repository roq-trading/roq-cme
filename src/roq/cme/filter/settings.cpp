/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/cme/filter/settings.hpp"

using namespace std::literals;

namespace roq {
namespace cme {
namespace filter {

// === IMPLEMENTATION ===

Settings::Settings(args::Parser const &) : flags::Flags{flags::Flags::create()} {
}

}  // namespace filter
}  // namespace cme
}  // namespace roq
