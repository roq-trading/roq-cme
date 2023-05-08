/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/cme/settings.hpp"

#include "roq/logging.hpp"

using namespace std::literals;

namespace roq {
namespace cme {

Settings::Settings(server::Type type)
    : server::Settings{server::create_settings(type, ROQ_PACKAGE_NAME, ROQ_BUILD_NUMBER)} {
  log::debug("settings={}"sv, *this);
}

}  // namespace cme
}  // namespace roq
