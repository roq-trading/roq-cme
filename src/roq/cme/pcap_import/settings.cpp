/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/cme/pcap_import/settings.hpp"

using namespace std::literals;

namespace roq {
namespace cme {
namespace pcap_import {

// === IMPLEMENTATION ===

Settings::Settings(args::Parser const &) : flags::Flags{flags::Flags::create()} {
}

}  // namespace pcap_import
}  // namespace cme
}  // namespace roq
