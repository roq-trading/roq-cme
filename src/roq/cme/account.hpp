/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <chrono>
#include <string>
#include <string_view>
#include <utility>

#include "roq/cme/config.hpp"

#include "roq/cme/tools/crypto.hpp"

namespace roq {
namespace cme {

struct Account final {
  Account(Config const &, std::string_view const &name);

  Account(Account &&) = delete;
  Account(Account const &) = delete;

  std::string_view get_name() const { return name_; }
  std::string_view get_login() const { return login_; }
  std::string_view get_password() const { return password_; }

  inline std::span<std::byte const> create_signature(tools::CanonicalMessage const &message) {
    return crypto_.create_signature(encode_buffer_, message);
  }

 private:
  std::string const name_;
  std::string const login_;
  std::string const password_;
  tools::Crypto crypto_;
  std::vector<std::byte> encode_buffer_;
};

}  // namespace cme
}  // namespace roq
