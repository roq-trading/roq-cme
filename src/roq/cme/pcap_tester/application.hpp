/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include "roq/tool.hpp"

namespace roq {
namespace cme {
namespace pcap_tester {

struct Application final : public roq::Tool {
  using roq::Tool::Tool;

 protected:
  int main(args::Parser const &) override;
};

}  // namespace pcap_tester
}  // namespace cme
}  // namespace roq
