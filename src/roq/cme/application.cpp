/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/cme/application.hpp"

#include "roq/cme/config.hpp"
#include "roq/cme/gateway.hpp"
#include "roq/cme/settings.hpp"

using namespace std::literals;

namespace roq {
namespace cme {

// === IMPLEMENTATION ===

int Application::main(args::Parser const &args) {
  Settings settings{args};
  Config config{settings};
  auto context = server::create_io_context(settings);
  server::Trading<Gateway>{settings, config, *context}.dispatch();
  return EXIT_SUCCESS;
}

}  // namespace cme
}  // namespace roq
