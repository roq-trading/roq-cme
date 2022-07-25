/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <fmt/chrono.h>
#include <fmt/format.h>

#include <cme_mdp/MessageHeader.h>

#include <cme_mdp/Side.h>

#include <cme_mdp/MDInstrumentDefinitionFX63.h>

#include "roq/api.hpp"

#include "roq/core/sbe/iterator.hpp"

#include "roq/logging.hpp"

namespace roq {
namespace cme {
namespace sbe {

inline Side map_book_side(cme_mdp::Side::Value value) {
  switch (value) {
    using enum cme_mdp::Side::Value;
    case Buy:
      return Side::BUY;
    case Sell:
      return Side::SELL;
    case NULL_VALUE:
      return Side::UNDEFINED;
  }
  return Side::UNDEFINED;
}

template <typename T>
size_t compute_length(T &);

template <>
inline size_t compute_length(cme_mdp::MessageHeader &value) {
  return value.encodedLength();
}

template <>
inline size_t compute_length(cme_mdp::MDInstrumentDefinitionFX63 &value) {
  return value.computeLength();
}

}  // namespace sbe
}  // namespace cme
}  // namespace roq

// header

template <>
struct fmt::formatter<cme_mdp::MessageHeader> {
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(cme_mdp::MessageHeader const &value, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(blockLength={}, )"
        R"(templateId={}, )"
        R"(schemaId={}, )"
        R"(version={})"
        R"(}})"sv,
        value.blockLength(),
        value.templateId(),
        value.schemaId(),
        value.version());
  }
};

// helper

// messages

template <>
struct fmt::formatter<cme_mdp::MDInstrumentDefinitionFX63> {
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(cme_mdp::MDInstrumentDefinitionFX63 &value, Context &context) const {
    using namespace std::literals;
    value.sbeRewind();
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(symbol="{}")"
        R"(}})"sv,
        value.symbol());
  }
};
