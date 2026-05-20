/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/cme/gateway/account.hpp"

namespace roq {
namespace cme {
namespace gateway {

// === CONSTANTS ===

namespace {
size_t const ENCODE_BUFFER_LENGTH = 512;
}  // namespace

// === IMPLEMENTATION ===

Account::Account(Config const &config, std::string_view const &name)
    : name_{name}, login_{config.get_login(name_)}, password_{config.get_password(name_)}, crypto_{config.get_secret(name_)},
      encode_buffer_(ENCODE_BUFFER_LENGTH) {
}

}  // namespace gateway
}  // namespace cme
}  // namespace roq
