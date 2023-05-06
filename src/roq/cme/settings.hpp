/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include "roq/server.hpp"

namespace roq {
namespace cme {

struct Settings final : public server::Settings {
  static Settings create(server::Type);
};

}  // namespace cme
}  // namespace roq
