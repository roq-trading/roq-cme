/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include "roq/tool.hpp"

namespace roq {
namespace cme {
namespace tester {

struct Application final : public roq::Tool {
  using roq::Tool::Tool;

 protected:
  int main(int argc, char **argv) override;
};

}  // namespace tester
}  // namespace cme
}  // namespace roq
