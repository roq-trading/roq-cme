/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/cme/pcap_tester/application.hpp"

#include "roq/logging.hpp"

#include "roq/cme/pcap_tester/controller.hpp"
#include "roq/cme/pcap_tester/settings.hpp"

using namespace std::literals;

namespace roq {
namespace cme {
namespace pcap_tester {

// === IMPLEMENTATION ===

int Application::main(args::Parser const &args) {
  Settings settings{args};
  auto params = args.params();
  if (std::size(params) != 1)
    log::fatal("Expected exactly one argument"sv);
  Controller{settings, params[0]}.dispatch();
  return EXIT_SUCCESS;
}

}  // namespace pcap_tester
}  // namespace cme
}  // namespace roq
