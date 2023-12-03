/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/cme/pcap_filter/application.hpp"

#include "roq/logging.hpp"

#include "roq/cme/pcap_filter/controller.hpp"
#include "roq/cme/pcap_filter/settings.hpp"

using namespace std::literals;

namespace roq {
namespace cme {
namespace pcap_filter {

// === IMPLEMENTATION ===

int Application::main(args::Parser const &args) {
  Settings settings{args};
  Controller{settings}.dispatch();
  return EXIT_SUCCESS;
}

}  // namespace pcap_filter
}  // namespace cme
}  // namespace roq
