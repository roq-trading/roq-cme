/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/cme/dump/settings.hpp"

using namespace std::literals;

namespace roq {
namespace cme {
namespace dump {

// === IMPLEMENTATION ===

Settings::Settings(args::Parser const &) : flags::Flags{flags::Flags::create()} {
}

}  // namespace dump
}  // namespace cme
}  // namespace roq
