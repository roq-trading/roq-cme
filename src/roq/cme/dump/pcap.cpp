/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/cme/dump/pcap.hpp"

#include "roq/exceptions.hpp"

using namespace std::literals;

namespace roq {
namespace cme {
namespace dump {

// === HELPERS ===

namespace {
struct UserData final {
  PCAP::callback_type const &callback;
  pcap_t *handle;
};

static void deleter(PCAP::value_type *handle) {
  if (handle)
    pcap_close(handle);
}

void helper(u_char *opaque, struct pcap_pkthdr const *header, u_char const *packet) {
  auto &user_data = *reinterpret_cast<UserData *>(opaque);
  if (user_data.callback(header, packet))
    pcap_breakloop(user_data.handle);
}
}  // namespace

// === IMPLEMENTATION ===

PCAP::PCAP(std::string const &path) : handle_{pcap_open_offline(path.c_str(), error_buffer_), deleter} {
  if (handle_ == nullptr)
    throw RuntimeError{"pcap_open_offline: {}"sv, error_buffer_};
}

void PCAP::dispatch(callback_type const &callback) {
  auto user_data = UserData{
      .callback = callback,
      .handle = handle_.get(),
  };
  auto res = pcap_dispatch(user_data.handle, -1, helper, reinterpret_cast<u_char *>(&user_data));
  if (res < 0)
    throw RuntimeError{"pcap_dispatch: {}"sv, pcap_geterr(handle_.get())};
}

}  // namespace dump
}  // namespace cme
}  // namespace roq
