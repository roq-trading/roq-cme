/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/cme/authenticator.hpp"

namespace roq {
namespace cme {

// === CONSTANTS ===

namespace {
auto const ENCODE_BUFFER_LENGTH = size_t{512};
}  // namespace

// === IMPLEMENTATION ===

Authenticator::Authenticator(Config const &config, std::string_view const &account)
    : account_{account}, login_{config.get_login(account_)}, password_{config.get_password(account_)},
      crypto_{config.get_secret(account_)}, encode_buffer_(ENCODE_BUFFER_LENGTH) {
}

}  // namespace cme
}  // namespace roq
