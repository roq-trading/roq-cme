/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <cstdint>
#include <span>

#include <cme_mdp/MDInstrumentDefinitionFX63.h>

#include "roq/trace.hpp"

#include "roq/cme/sbe/frame.hpp"

namespace roq {
namespace cme {
namespace sbe {

struct Parser final {
  struct Handler {
    virtual void operator()(Trace<cme_mdp::MDInstrumentDefinitionFX63> const &, Frame const &) = 0;
  };

  static bool dispatch(Handler &, std::span<std::byte const> const &buffer, TraceInfo const &);
};

}  // namespace sbe
}  // namespace cme
}  // namespace roq
