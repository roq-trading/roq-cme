/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/cme/application.hpp"

#include "roq/cme/flags/settings.hpp"

#include "roq/cme/gateway/config.hpp"
#include "roq/cme/gateway/controller.hpp"

using namespace std::literals;

namespace roq {
namespace cme {

// === CONSTANTS ===

namespace {
uint8_t const API = {};
}

// === IMPLEMENTATION ===

int Application::main(args::Parser const &args) {
  flags::Settings settings{args};
  gateway::Config config{settings};
  auto context = server::create_io_context(settings);
  server::Trading2<gateway::Controller>{settings, config, *context, API}.dispatch();
  return EXIT_SUCCESS;
}

}  // namespace cme
}  // namespace roq
