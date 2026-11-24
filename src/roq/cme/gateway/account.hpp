/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <chrono>
#include <string>
#include <string_view>
#include <utility>

#include "roq/cme/gateway/config.hpp"

#include "roq/cme/tools/crypto.hpp"

namespace roq {
namespace cme {
namespace gateway {

struct Account final {
  Account(Config const &, std::string_view const &name);

  Account(Account const &) = delete;

  inline std::span<std::byte const> create_signature(tools::CanonicalMessage const &message) { return crypto_.create_signature(encode_buffer_, message); }

  std::string const name;
  std::string const login;
  std::string const password;

 private:
  tools::Crypto crypto_;
  std::vector<std::byte> encode_buffer_;
};

}  // namespace gateway
}  // namespace cme
}  // namespace roq
