/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/cme/channel_tester/application.hpp"

#include "roq/logging.hpp"

#include "roq/cme/channel_tester/controller.hpp"
#include "roq/cme/channel_tester/settings.hpp"

using namespace std::literals;

namespace roq {
namespace cme {
namespace channel_tester {

// === IMPLEMENTATION ===

int Application::main(args::Parser const &) {
  Settings settings;
  Controller{settings}.dispatch();
  return EXIT_SUCCESS;
}

}  // namespace channel_tester
}  // namespace cme
}  // namespace roq
