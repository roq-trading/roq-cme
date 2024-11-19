/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/cme/dump/application.hpp"

#include "roq/flags/args.hpp"
#include "roq/logging/flags/settings.hpp"

using namespace std::literals;

// === CONSTANTS ===

namespace {
roq::Tool::Info const INFO{
    .description = "Roq CME Dump Tool"sv,
    .package_name = ROQ_PACKAGE_NAME,
    .host = ROQ_HOST,
    .build_version = ROQ_BUILD_VERSION,
    .build_number = ROQ_BUILD_NUMBER,
    .build_type = ROQ_BUILD_TYPE,
    .git_hash = ROQ_GIT_DESCRIBE_HASH,
};
}  // namespace

// === IMPLEMENTATION ===

int main(int argc, char **argv) {
  roq::flags::Args args{argc, argv, INFO.description, INFO.build_version};
  roq::logging::flags::Settings settings{args};
  return roq::cme::dump::Application{args, settings, INFO}.run();
}
