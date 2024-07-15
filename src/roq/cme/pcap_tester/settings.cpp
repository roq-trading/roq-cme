/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/cme/pcap_tester/settings.hpp"

using namespace std::literals;

namespace roq {
namespace cme {
namespace pcap_tester {

// === IMPLEMENTATION ===

Settings::Settings(args::Parser const &) : flags::Flags{flags::Flags::create()} {
}

}  // namespace pcap_tester
}  // namespace cme
}  // namespace roq
