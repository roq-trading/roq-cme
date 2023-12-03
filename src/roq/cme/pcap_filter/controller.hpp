/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include "roq/cme/pcap_filter/settings.hpp"

namespace roq {
namespace cme {
namespace pcap_filter {

struct Controller final {
  explicit Controller(Settings const &);

  Controller(Controller const &) = delete;
  Controller(Controller &&) = delete;

  void dispatch();

 private:
  Settings const &settings_;
};

}  // namespace pcap_filter
}  // namespace cme
}  // namespace roq
