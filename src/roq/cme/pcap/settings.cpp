/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/cme/pcap/settings.hpp"

using namespace std::literals;

namespace roq {
namespace cme {
namespace pcap {

// === IMPLEMENTATION ===

Settings::Settings(args::Parser const &) : flags::Flags{flags::Flags::create()} {
}

}  // namespace pcap
}  // namespace cme
}  // namespace roq
