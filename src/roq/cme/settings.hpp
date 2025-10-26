/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include <fmt/format.h>

#include "roq/server/flags/settings.hpp"

#include "roq/cme/flags/flags.hpp"
#include "roq/cme/flags/ilink.hpp"
#include "roq/cme/flags/misc.hpp"
#include "roq/cme/flags/multicast.hpp"
#include "roq/cme/flags/test.hpp"

namespace roq {
namespace cme {

struct Settings final : public server::flags::Settings, public flags::Flags {
  explicit Settings(args::Parser const &);

  flags::Misc misc;
  flags::Multicast multicast;
  flags::iLink ilink;
  flags::Test test;
};

}  // namespace cme
}  // namespace roq

template <>
struct fmt::formatter<roq::cme::Settings> {
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(roq::cme::Settings const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(misc={}, )"
        R"(multicast={}, )"
        R"(ilink={}, )"
        R"(test={}, )"
        R"(server={})"
        R"(}})"sv,
        value.misc,
        value.multicast,
        value.ilink,
        value.test,
        static_cast<roq::server::Settings const &>(value));
  }
};
