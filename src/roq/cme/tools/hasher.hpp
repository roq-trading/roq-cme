/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <chrono>
#include <string>
#include <string_view>
#include <utility>

#include "roq/core/crypto/hmac_sha256.hpp"
#include "roq/core/crypto/sha256.hpp"

namespace roq {
namespace deribit {
namespace tools {

class Hasher final {
 public:
  explicit Hasher(std::string_view const &secret);

  Hasher(Hasher &&) = delete;
  Hasher(Hasher const &) = delete;

  std::string create_nonce();

  std::pair<std::string, std::chrono::milliseconds> create_signature(
      std::chrono::milliseconds timestamp, std::string_view const &nonce);

  std::string create_raw_data(std::chrono::milliseconds timestamp);
  std::string create_raw_data(std::chrono::milliseconds timestamp, std::string_view const &nonce);

  std::string create_password(std::string_view const &raw_data);

 protected:
  int64_t get_sequence(std::chrono::milliseconds timestamp);

 private:
  const std::string secret_;
  core::crypto::SHA256 sha_;
  core::crypto::HMAC_SHA256 hmac_;
  std::chrono::milliseconds timestamp_ = {};
};

}  // namespace tools
}  // namespace deribit
}  // namespace roq
