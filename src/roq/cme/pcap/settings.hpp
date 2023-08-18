/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include "roq/flags/args.hpp"

#include "roq/cme/pcap/flags/flags.hpp"

namespace roq {
namespace cme {
namespace pcap {

struct Settings final : public flags::Flags {
  explicit Settings(args::Parser const &);
};

}  // namespace pcap
}  // namespace cme
}  // namespace roq
