/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include "roq/flags/args.hpp"

#include "roq/cme/pcap_tester/flags/flags.hpp"

namespace roq {
namespace cme {
namespace pcap_tester {

struct Settings final : public flags::Flags {
  explicit Settings(args::Parser const &);
};

}  // namespace pcap_tester
}  // namespace cme
}  // namespace roq
