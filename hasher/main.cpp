/* Copyright (c) 2017-2022, Hans Erik Thrane */

#define __STDC_FORMAT_MACROS 1

#include <inttypes.h>
#include <stdint.h>

#include <charconv>
#include <string_view>

#include "roq/deribit/tools/hasher.hpp"

using namespace std::literals;

using namespace roq;           // NOLINT
using namespace roq::deribit;  // NOLINT

int main(int argc, char **argv) {
  // validate
  if (argc < 4) {
    fprintf(stderr, "ARGS: {fix|ws} secret timestamp [nonce]\n");
    return EXIT_FAILURE;
  }

  // flags
  std::string_view type{argv[1]};
  std::string_view secret{argv[2]};
  std::string_view timestamp{argv[3]};
  auto nonce = argc >= 5 ? std::string_view{argv[4]} : std::string_view{};

  // convert
  int64_t value = {};
  std::from_chars(std::data(timestamp), std::data(timestamp) + std::size(timestamp), value);
  std::chrono::milliseconds real_timestamp{value};

  // summary
  printf("   secret : \"%.*s\"\n", static_cast<int>(std::size(secret)), std::data(secret));
  printf("timestamp : %" PRId64 "\n", value);

  // compute
  tools::Hasher hasher(secret);

  // fix
  if (type.compare("fix"sv) == 0) {
    // compute
    auto raw_data = hasher.create_raw_data(real_timestamp, nonce);
    auto password = hasher.create_password(raw_data);
    // summary
    printf(" raw_data : \"%.*s\"\n", static_cast<int>(std::size(raw_data)), std::data(raw_data));
    printf(" password : \"%.*s\"\n", static_cast<int>(std::size(password)), std::data(password));
  } else if (type.compare("ws"sv) == 0) {
    // compute
    auto real_nonce = std::empty(nonce) ? hasher.create_nonce() : std::string{nonce};
    auto [signature, used_timestamp] = hasher.create_signature(real_timestamp, real_nonce);
    // summary
    printf("    nonce : \"%.*s\"\n", static_cast<int>(std::size(real_nonce)), std::data(real_nonce));
    printf("signature : \"%.*s\"\n", static_cast<int>(std::size(signature)), std::data(signature));
  } else {
    fprintf(stderr, "Unknown type=%s\n", argv[1]);
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
