/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/deribit/tools/hasher.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <random>
#include <span>

#include <fmt/format.h>

#include "roq/core/binascii/base64.hpp"
#include "roq/core/binascii/hex.hpp"

using namespace std::literals;

namespace roq {
namespace deribit {
namespace tools {

namespace {
const constexpr auto CHARSET_DATA = "abcdefghijklmnopqrstuvwxyz0123456789"sv;
const constexpr auto RANDOM_BYTES = 32;

static_assert(std::size(CHARSET_DATA) == 36);

std::random_device GENERATOR;
std::uniform_int_distribution<int> CHARSET_DISTRIBUTION(
    0, std::size(CHARSET_DATA) - 1);  // note! max value is inclusive
std::uniform_int_distribution<uint32_t> DISTRIBUTION;
}  // namespace

Hasher::Hasher(std::string_view const &access_secret) : secret_(access_secret), hmac_(secret_) {
}

std::string Hasher::create_nonce() {
  std::string result(RANDOM_BYTES, '-');
  std::generate(std::begin(result), std::end(result), []() { return CHARSET_DATA[CHARSET_DISTRIBUTION(GENERATOR)]; });
  return result;
}

std::pair<std::string, std::chrono::milliseconds> Hasher::create_signature(
    std::chrono::milliseconds timestamp, std::string_view const &nonce) {
  auto sequence = get_sequence(timestamp);
  auto message = fmt::format("{}\n{}\n"sv, sequence, nonce);
  hmac_.clear();
  hmac_.update(message);
  std::array<char, 32> buffer;
  auto length = hmac_.digest(buffer);
  assert(length == std::size(buffer));
  return {core::binascii::Hex::encode(buffer), std::chrono::milliseconds{sequence}};
}

std::string Hasher::create_raw_data(std::chrono::milliseconds timestamp) {
  using value_type = decltype(DISTRIBUTION)::result_type;
  constexpr auto n = RANDOM_BYTES / sizeof(value_type);
  std::array<value_type, n> buffer;
  for (size_t i = 0; i < n; ++i)
    buffer[i] = DISTRIBUTION(GENERATOR);
  std::span tmp{reinterpret_cast<std::byte *>(std::data(buffer)), std::size(buffer) * sizeof(value_type)};
  auto nonce = core::binascii::Base64::encode(tmp, false);
  return create_raw_data(timestamp, nonce);
}

std::string Hasher::create_raw_data(std::chrono::milliseconds timestamp, std::string_view const &nonce) {
  auto sequence = get_sequence(timestamp);
  auto raw_data = fmt::format("{:013}.{}"sv, sequence, nonce);
  return raw_data;
}

std::string Hasher::create_password(std::string_view const &raw_data) {
  sha_.clear();
  sha_.update(raw_data);
  sha_.update(secret_);
  std::array<char, 32> buffer;
  auto length = sha_.digest(buffer);
  assert(length == std::size(buffer));
  return core::binascii::Base64::encode(buffer, false);
}

int64_t Hasher::get_sequence(std::chrono::milliseconds timestamp) {
  if (timestamp_ < timestamp) {
    timestamp_ = timestamp;
  } else {
    ++timestamp_;
  }
  return timestamp_.count();
}

}  // namespace tools
}  // namespace deribit
}  // namespace roq
