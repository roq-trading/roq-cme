/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include "roq/tool.hpp"

namespace roq {
namespace cme {
namespace dump {

struct Application final : public roq::Tool {
  using roq::Tool::Tool;

 protected:
  int main(args::Parser const &) override;
};

}  // namespace dump
}  // namespace cme
}  // namespace roq
