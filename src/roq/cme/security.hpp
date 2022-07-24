/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <chrono>
#include <string>
#include <string_view>
#include <utility>

#include "roq/deribit/config.hpp"

#include "roq/deribit/tools/hasher.hpp"

namespace roq {
namespace deribit {

class Security final {
 public:
  Security(Config const &, std::string_view const &account);

  Security(Security &&) = delete;
  Security(Security const &) = delete;

  std::string_view get_account() const { return account_; }
  std::string_view get_access_key() const { return key_; }

  std::string create_nonce();

  std::pair<std::string, std::chrono::milliseconds> create_signature(
      std::chrono::milliseconds timestamp, std::string_view const &nonce) {
    return hasher_.create_signature(timestamp, nonce);
  }

  std::string create_raw_data(std::chrono::milliseconds timestamp) { return hasher_.create_raw_data(timestamp); }

  std::string create_password(std::string_view const &raw_data);

 private:
  const std::string account_;
  const std::string key_;
  tools::Hasher hasher_;
};

}  // namespace deribit
}  // namespace roq
