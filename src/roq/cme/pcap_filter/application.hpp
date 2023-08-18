/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include "roq/tool.hpp"

namespace roq {
namespace cme {
namespace pcap_filter {

struct Application final : public roq::Tool {
  using roq::Tool::Tool;

 protected:
  int main(args::Parser const &) override;
};

}  // namespace pcap_filter
}  // namespace cme
}  // namespace roq
