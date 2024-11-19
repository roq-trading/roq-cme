/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/cme/filter/application.hpp"

#include "roq/logging.hpp"

#include "roq/cme/filter/controller.hpp"
#include "roq/cme/filter/settings.hpp"

using namespace std::literals;

namespace roq {
namespace cme {
namespace filter {

// === IMPLEMENTATION ===

int Application::main(args::Parser const &args) {
  Settings settings{args};
  Controller{settings}.dispatch();
  return EXIT_SUCCESS;
}

}  // namespace filter
}  // namespace cme
}  // namespace roq
