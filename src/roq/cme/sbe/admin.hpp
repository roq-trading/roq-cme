/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <fmt/format.h>

#include <cme_mdp/AdminHeartbeat12.h>

namespace roq {
namespace cme {
namespace sbe {}  // namespace sbe
}  // namespace cme
}  // namespace roq

// messages

// AdminHeartbeat12

template <>
struct fmt::formatter<cme_mdp::AdminHeartbeat12> {
  using value_type = cme_mdp::AdminHeartbeat12;
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type const &, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(}})"sv);
  }
};
