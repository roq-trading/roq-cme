/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/cme/pcap/application.hpp"

#include "roq/logging.hpp"

#include "roq/cme/pcap/controller.hpp"
#include "roq/cme/pcap/settings.hpp"

using namespace std::literals;

namespace roq {
namespace cme {
namespace pcap {

// === IMPLEMENTATION ===

int Application::main(args::Parser const &args) {
  Settings settings{args};
  Controller{settings}.dispatch();
  return EXIT_SUCCESS;
}

}  // namespace pcap
}  // namespace cme
}  // namespace roq
