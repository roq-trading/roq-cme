/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include "roq/utils/pcap/reader.hpp"

#include "roq/cme/dump/settings.hpp"

namespace roq {
namespace cme {
namespace dump {

struct Controller final {
  Controller(Settings const &, std::string_view const &pcap_path);

  Controller(Controller const &) = delete;
  Controller(Controller &&) = delete;

  void dispatch();

 private:
  Settings const &settings_;
  std::string const pcap_path_;
};

}  // namespace dump
}  // namespace cme
}  // namespace roq
