/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include "roq/flags/args.hpp"

#include "roq/cme/pcap_filter/flags/flags.hpp"

namespace roq {
namespace cme {
namespace pcap_filter {

struct Settings final : public flags::Flags {
  explicit Settings(args::Parser const &);
};

}  // namespace pcap_filter
}  // namespace cme
}  // namespace roq
