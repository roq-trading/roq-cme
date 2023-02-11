/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/cme/tester/application.hpp"

#include <string>
#include <vector>

#include "roq/logging.hpp"

#include "roq/cme/tester/controller.hpp"

using namespace std::literals;

namespace roq {
namespace cme {
namespace tester {

// === IMPLEMENTATION ===

int Application::main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
  std::vector<std::string> arguments(argv + 1, argv + argc);
  Controller{arguments}.dispatch();
  return EXIT_SUCCESS;
}

}  // namespace tester
}  // namespace cme
}  // namespace roq
