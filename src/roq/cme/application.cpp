/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/cme/application.hpp"

#include "roq/cme/config.hpp"
#include "roq/cme/gateway.hpp"

using namespace std::literals;

namespace roq {
namespace cme {

// === CONSTANTS ===

namespace {
auto const SETTINGS = server::Settings{
    .package_name = ROQ_PACKAGE_NAME,
    .build_number = ROQ_BUILD_NUMBER,
    .api = {},
    .type = server::Type::ORDER_MANAGEMENT,
};
}

// === IMPLEMENTATION ===

int Application::main(int, char **) {
  Config config;
  auto context = server::create_io_context();
  server::Trading<Gateway>{SETTINGS, config, *context}.dispatch();
  return EXIT_SUCCESS;
}

}  // namespace cme
}  // namespace roq
