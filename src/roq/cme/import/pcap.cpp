/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/cme/import/pcap.hpp"

#include "roq/exceptions.hpp"

using namespace std::literals;

namespace roq {
namespace cme {
namespace import {

// === HELPERS ===

namespace {
static void deleter(PCAP::value_type *handle) {
  if (handle)
    pcap_close(handle);
}

void helper(u_char *user_data, struct pcap_pkthdr const *header, u_char const *packet) {
  auto callback = reinterpret_cast<PCAP::callback_type const *>(user_data);
  (*callback)(header, packet);
}
}  // namespace

// === IMPLEMENTATION ===

PCAP::PCAP(std::string const &path) : handle_{pcap_open_offline(path.c_str(), error_buffer_), deleter} {
  if (handle_ == nullptr)
    throw RuntimeError{"pcap_open_offline: {}"sv, error_buffer_};
}

void PCAP::dispatch(callback_type const &callback) {
  auto res =
      pcap_dispatch(handle_.get(), -1, helper, reinterpret_cast<u_char *>(&const_cast<callback_type &>(callback)));
  if (res < 0)
    throw RuntimeError{"pcap_dispatch: {}"sv, pcap_geterr(handle_.get())};
}

}  // namespace import
}  // namespace cme
}  // namespace roq
