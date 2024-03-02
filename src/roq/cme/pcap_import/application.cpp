/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/cme/pcap_import/application.hpp"

#include "roq/logging.hpp"

#include "roq/cme/pcap_import/controller.hpp"
#include "roq/cme/pcap_import/settings.hpp"

using namespace std::literals;

namespace roq {
namespace cme {
namespace pcap_import {

// === IMPLEMENTATION ===

int Application::main(args::Parser const &args) {
  Settings settings{args};
  auto params = args.params();
  if (std::size(params) != 1)
    log::fatal("Expected exactly one argument"sv);
  Controller{settings}.dispatch(params[0]);
  return EXIT_SUCCESS;
}

}  // namespace pcap_import
}  // namespace cme
}  // namespace roq
