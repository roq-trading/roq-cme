/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <fmt/compile.h>
#include <fmt/format.h>

#include "roq/server.hpp"

#include "roq/cme/flags/common.hpp"
#include "roq/cme/flags/ilink.hpp"
#include "roq/cme/flags/multicast.hpp"
#include "roq/cme/flags/test.hpp"

namespace roq {
namespace cme {

struct Settings final : public server::Settings {
  explicit Settings(server::Type);

  std::span<std::string const> exchange;

  flags::Common__flags common;
  flags::Multicast__flags multicast;
  flags::iLink__flags ilink;
  flags::Test__flags test;
};

}  // namespace cme
}  // namespace roq

template <>
struct fmt::formatter<roq::cme::Settings> {
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(roq::cme::Settings const &value, Context &context) const {
    using namespace std::literals;
    using namespace fmt::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(exchange=[{}], )"
        R"(common={}, )"
        R"(multicast={}, )"
        R"(ilink={}, )"
        R"(test={}, )"
        R"(server={})"
        R"(}})"_cf,
        fmt::join(value.exchange, ", "sv),
        value.common,
        value.multicast,
        value.ilink,
        value.test,
        static_cast<roq::server::Settings const &>(value));
  }
};
