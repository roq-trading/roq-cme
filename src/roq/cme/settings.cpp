/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/cme/settings.hpp"

#include "roq/logging.hpp"

#include "roq/cme/flags/flags.hpp"

using namespace std::literals;

namespace roq {
namespace cme {

Settings::Settings(args::Parser const &args)
    : server::flags::Settings{args, ROQ_PACKAGE_NAME, ROQ_BUILD_NUMBER}, exchange{flags::Flags::exchange()} {
  log::debug("settings={}"sv, *this);
}

}  // namespace cme
}  // namespace roq
