/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include "roq/cme/pcap/settings.hpp"

namespace roq {
namespace cme {
namespace pcap {

struct Controller final {
  explicit Controller(Settings const &);

  Controller(Controller const &) = delete;
  Controller(Controller &&) = delete;

  void dispatch();

 private:
  Settings const &settings_;
};

}  // namespace pcap
}  // namespace cme
}  // namespace roq
