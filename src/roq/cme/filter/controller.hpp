/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include "roq/cme/filter/settings.hpp"

namespace roq {
namespace cme {
namespace filter {

struct Controller final {
  explicit Controller(Settings const &);

  Controller(Controller const &) = delete;
  Controller(Controller &&) = delete;

  void dispatch();

 private:
  Settings const &settings_;
};

}  // namespace filter
}  // namespace cme
}  // namespace roq
