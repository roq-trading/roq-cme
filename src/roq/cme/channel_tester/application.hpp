/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include "roq/tool.hpp"

namespace roq {
namespace cme {
namespace channel_tester {

struct Application final : public roq::Tool {
  using roq::Tool::Tool;

 protected:
  int main(int argc, char **argv) override;
};

}  // namespace channel_tester
}  // namespace cme
}  // namespace roq
