/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "roq/debug/hex/message.hpp"

#include "roq/core/hash/sha256.hpp"

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
      .request_timestamp = 1563720650008ms,
      .uuid = 1563720660068,
      .session = "ABC"sv,
      .firm_id = "007"sv,
      .trading_system_name = {},
      .trading_version_id = {},
      .trading_system_vendor_id = {},
      .next_seq_no = 1,
      .keep_alive_interval = 30s,
  };
  auto signature = crypto.create_signature(buffer, message);
  auto tmp = fmt::format("{}"sv, debug::hex::Message{signature});
  auto const expected = R"(\x42\x99\x33\x00\x7b\xf5\xdd\xd8\xc4\xff\xc3\x5f\x94\x23\x43\x8c)"
                        R"(\x25\x2a\xa9\xb4\x36\xd2\x2b\xfa\xf5\xf4\xe7\xea\xcb\xbb\x0b\x64)"sv;
  CHECK(tmp == expected);
}
