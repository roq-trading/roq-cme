/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/deribit/security.hpp"

namespace roq {
namespace deribit {

Security::Security(Config const &config, std::string_view const &account)
    : account_(account), key_(config.get_access_key(account)), hasher_(config.get_access_secret(account)) {
}

std::string Security::create_nonce() {
  return hasher_.create_nonce();
}

std::string Security::create_password(std::string_view const &raw_data) {
  return hasher_.create_password(raw_data);
}

}  // namespace deribit
}  // namespace roq
