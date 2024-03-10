/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include <functional>
#include <memory>
#include <string>
#include <string_view>

#include <pcap/pcap.h>

namespace roq {
namespace cme {
namespace import {

struct PCAP final {
  using value_type = pcap_t;
  using callback_type = std::function<void(struct pcap_pkthdr const *header, u_char const *packet)>;

  explicit PCAP(std::string const &path);
  explicit PCAP(std::string_view const &path) : PCAP{std::string{path}} {}

  void dispatch(callback_type const &);

 private:
  char error_buffer_[PCAP_ERRBUF_SIZE] = {};
  std::unique_ptr<value_type, void (*)(value_type *)> handle_;
};

}  // namespace import
}  // namespace cme
}  // namespace roq
