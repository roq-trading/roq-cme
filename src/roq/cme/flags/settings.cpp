/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/cme/flags/settings.hpp"

#include "roq/logging.hpp"

#include "roq/cme/flags/flags.hpp"

using namespace std::literals;

namespace roq {
namespace cme {
namespace flags {

Settings::Settings(args::Parser const &args)
    : server::flags::Settings{args, ROQ_PACKAGE_NAME, ROQ_BUILD_NUMBER, ROQ_GIT_DESCRIBE_HASH, {}}, flags::Flags{flags::Flags::create()},
      misc{flags::Misc::create()}, multicast{flags::Multicast::create()}, ilink{flags::iLink::create()}, test{flags::Test::create()} {
  log::info("settings={}"sv, *this);
}

}  // namespace flags
}  // namespace cme
}  // namespace roq
