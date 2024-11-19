/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include "roq/tool.hpp"

namespace roq {
namespace cme {
namespace import {

struct Application final : public roq::Tool {
  using roq::Tool::Tool;

 protected:
  int main(args::Parser const &) override;
};

}  // namespace import
}  // namespace cme
}  // namespace roq
