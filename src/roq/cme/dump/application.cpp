/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/cme/dump/application.hpp"

#include "roq/logging.hpp"

#include "roq/cme/dump/controller.hpp"
#include "roq/cme/dump/settings.hpp"

using namespace std::literals;

namespace roq {
namespace cme {
namespace dump {

// === IMPLEMENTATION ===

int Application::main(args::Parser const &args) {
  Settings settings{args};
  auto params = args.params();
  if (std::size(params) != 1)
    log::fatal("Expected exactly one argument"sv);
  Controller{settings, params[0]}.dispatch();
  return EXIT_SUCCESS;
}

}  // namespace dump
}  // namespace cme
}  // namespace roq
