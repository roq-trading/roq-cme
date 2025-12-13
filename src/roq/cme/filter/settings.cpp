/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/cme/filter/settings.hpp"

using namespace std::literals;

namespace roq {
namespace cme {
namespace filter {

// === IMPLEMENTATION ===

Settings::Settings(args::Parser const &) : flags::Flags{flags::Flags::create()}, cme{flags::CME::create()} {
}

}  // namespace filter
}  // namespace cme
}  // namespace roq
