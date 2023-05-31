/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "roq/debug/hex/message.hpp"

#include "roq/core/hash/sha256.hpp"

#include "roq/cme/tools/canonical_message.hpp"
#include "roq/cme/tools/crypto.hpp"

using namespace roq;
using namespace roq::cme;

using namespace std::literals;

// === CONSTANTS ===

namespace {
auto const SECRET = "6YPQyMHZH8_bVh_rJ6N250cNXy901j_zYnuVsKrFz00"sv;
}  // namespace

// === IMPLEMENTATION ===

TEST_CASE("negotiate", "[tools_crypto]") {
  tools::Crypto crypto{SECRET};
  std::vector<std::byte> buffer(4096);
  auto now = 1685417517123456789ns;
  auto message = tools::CanonicalMessage{
      .request_timestamp = now,
      .uuid = std::chrono::duration_cast<std::chrono::microseconds>(now).count(),
      .session = "ABC"sv,
      .firm_id = "007"sv,
      .trading_system_name = {},
      .trading_system_version = {},
      .trading_system_vendor = {},
      .next_seq_no = {},
      .keep_alive_interval = {},
  };
  auto signature = crypto.create_signature(buffer, message);
  auto tmp = fmt::format("{}"sv, debug::hex::Message{signature});
  /*
  auto const expected = R"(\x55\x51\x3a\xfc\xd2\x4b\xd0\x4c\xbe\x2f\x69\x44\xe0\x26\xf1\xf2)"
                        R"(\xb3\xb9\x26\x5c\x02\x60\xac\xba\x7e\xa6\x69\x2d\x29\x51\x55\xf6)"sv;
  auto const expected = R"(\x4a\x1e\x7f\xa2\xdd\x96\x3f\x5a\x01\x89\x5f\x82\x7a\x99\xdf\x1a)"
                        R"(\x5a\xe0\x31\x56\xeb\xc5\xb0\x64\xf4\xb6\x70\x4b\x11\x88\x5c\xf0)"sv;
  CHECK(tmp == expected);
  */
}

TEST_CASE("establish", "[tools_crypto]") {
  tools::Crypto crypto{SECRET};
  std::vector<std::byte> buffer(4096);
  auto message = tools::CanonicalMessage{
      .request_timestamp = 1563720650008ms,
      .uuid = 1563720660068,
      .session = "ABC"sv,
      .firm_id = "007"sv,
      .trading_system_name = "foo"sv,
      .trading_system_version = "bar"sv,
      .trading_system_vendor = "baz"sv,
      .next_seq_no = 1,
      .keep_alive_interval = 30s,
  };
  auto signature = crypto.create_signature(buffer, message);
  auto tmp = fmt::format("{}"sv, debug::hex::Message{signature});
  /*
  auto const expected = R"(\xcc\xc8\x97\x27\x75\x36\x62\x69\x47\xfd\xaf\xcb\xbc\xb8\x23\x3e)"
                        R"(\x48\x9a\x18\x02\x5e\x67\xef\x8d\x30\x19\x67\x44\x97\xd9\xf0\x07)"sv;
  auto const expected = R"(\xbe\x38\xdb\x1a\xbb\x6a\x24\x3b\x8a\x71\xa0\x4c\xe4\xb5\x3b\x3f)"
                        R"(\xfc\x76\x78\x6e\x34\x74\x16\x32\xfd\x9e\x88\x94\xff\xfa\x6a\x32)"sv;
  CHECK(tmp == expected);
  */
}
