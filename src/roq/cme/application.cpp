/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/cme/application.hpp"

#include "roq/cme/config.hpp"
#include "roq/cme/gateway.hpp"

#include "roq/cme/flags/config.hpp"

using namespace std::literals;

namespace roq {
namespace cme {

int Application::main(int, char **) {
  log::info(R"(Parse config_file="{}")"sv, flags::Config::config_file());
  Config config(flags::Config::config_file(), flags::Config::secrets_file());
  log::info<1>("config={}"sv, config);
  log::info("Starting the gateway..."sv);
  roq::server::Trading<Gateway>(ROQ_PACKAGE_NAME, ROQ_BUILD_NUMBER, {}, config).dispatch();
  return EXIT_SUCCESS;
}

}  // namespace cme
}  // namespace roq
