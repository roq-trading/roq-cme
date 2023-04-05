/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <array>
#include <string>
#include <string_view>

#include "roq/core/mac/hmac.hpp"

#include "roq/cme/tools/canonical_message.hpp"

namespace roq {
namespace cme {
namespace tools {

struct Crypto final {
  explicit Crypto(std::string_view const &secret);

  Crypto(Crypto &&) = delete;
  Crypto(Crypto const &) = delete;

  std::span<std::byte const> create_signature(std::span<std::byte> const &buffer, CanonicalMessage const &);

 private:
  using MAC = core::mac::HMAC<core::hash::SHA256>;
  using Digest = std::array<std::byte, MAC::DIGEST_LENGTH>;

  std::string const key_;
  MAC mac_;
  Digest digest_;
};

}  // namespace tools
}  // namespace cme
}  // namespace roq
