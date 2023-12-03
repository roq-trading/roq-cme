/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/cme/account.hpp"

namespace roq {
namespace cme {

// === CONSTANTS ===

namespace {
auto const ENCODE_BUFFER_LENGTH = size_t{512};
}  // namespace

// === IMPLEMENTATION ===

Account::Account(Config const &config, std::string_view const &name)
    : name_{name}, login_{config.get_login(name_)}, password_{config.get_password(name_)},
      crypto_{config.get_secret(name_)}, encode_buffer_(ENCODE_BUFFER_LENGTH) {
}

}  // namespace cme
}  // namespace roq
