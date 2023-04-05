/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <roq/core/hash/sha256.hpp>

#include "roq/cme/tools/crypto.hpp"

using namespace roq;
using namespace roq::cme;

using namespace std::literals;

// === CONSTANTS ===

namespace {
auto const KEY = "sSzUA6j8tGDfmLoFrOPhWHY3VeXbC3NrApp94Ci4H4XvcjuCuvOXp8gH89XzMPDe"sv;
auto const SECRET = "tHurnNFWLFkm97xVRqoESdujAiq1ilNjnY52tDej5RilUbTVZXT2YB5eo7txFLHk"sv;
}  // namespace

// === IMPLEMENTATION ===

TEST_CASE("simple", "[tools_crypto]") {
  tools::Crypto crypto{SECRET};
  std::vector<std::byte> buffer(4096);
  auto message = tools::CanonicalMessage{
      .request_timestamp = {},
  };

  auto signature = crypto.create_signature(buffer, message);
  /*
  auto expected =
      "?timestamp=1674303865000&signature=fa3ec135cd0ca6fd1267e30feb51147885209b0ee6c997ace0a6c7694b29736f"sv;
  CHECK(query == expected);
  */
}
