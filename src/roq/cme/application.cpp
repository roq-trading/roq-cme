/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/cme/application.hpp"

#include "roq/cme/config.hpp"
#include "roq/cme/gateway.hpp"
#include "roq/cme/settings.hpp"

using namespace std::literals;

namespace roq {
namespace cme {

// === CONSTANTS ===

namespace {
auto const TYPE = server::Type::ORDER_MANAGEMENT;
}

// === IMPLEMENTATION ===

int Application::main(int, char **) {
  Settings settings{TYPE};
  Config config{settings};
  auto context = server::create_io_context();
  server::Trading<Gateway>{settings, config, *context}.dispatch();
  return EXIT_SUCCESS;
}

}  // namespace cme
}  // namespace roq
