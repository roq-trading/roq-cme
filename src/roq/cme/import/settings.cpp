/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/cme/import/settings.hpp"

using namespace std::literals;

namespace roq {
namespace cme {
namespace import {

// === IMPLEMENTATION ===

Settings::Settings(args::Parser const &) : flags::Flags{flags::Flags::create()} {
}

}  // namespace import
}  // namespace cme
}  // namespace roq
