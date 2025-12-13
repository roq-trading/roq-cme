/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/cme/import/application.hpp"

#include "roq/logging.hpp"

#include "roq/cme/import/controller.hpp"
#include "roq/cme/import/settings.hpp"

using namespace std::literals;

namespace roq {
namespace cme {
namespace import {

// === IMPLEMENTATION ===

int Application::main(args::Parser const &args) {
  Settings settings{args};
  auto params = args.params();
  if (std::size(params) != 1) {
    log::fatal("Expected exactly one argument"sv);
  }
  Controller{settings, params[0]}.dispatch();
  return EXIT_SUCCESS;
}

}  // namespace import
}  // namespace cme
}  // namespace roq
