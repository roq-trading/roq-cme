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

struct Authenticator final {
  Authenticator(Config const &, std::string_view const &account);

  Authenticator(Authenticator &&) = delete;
  Authenticator(Authenticator const &) = delete;

  inline std::span<std::byte const> create_signature(tools::Canonical const &message) {
    return crypto_.create_signature(encode_buffer_, message);
  }

 private:
  std::string const account_;
  tools::Crypto crypto_;
  std::vector<std::byte> encode_buffer_;
};

}  // namespace cme
}  // namespace roq
