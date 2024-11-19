/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include <fmt/format.h>

#include <cmath>
#include <span>

#include <cme_ilink/MessageHeader.h>
#include <cme_ilink/Negotiate500.h>

#include "roq/utils/debug/hex/message.hpp"

#include "roq/cme/ilink/negotiate.hpp"

using namespace std::literals;

using namespace Catch::literals;

using namespace cme_ilink;

using namespace roq;

TEST_CASE("simple", "[negotiate]") {
  std::vector<std::byte> buffer(4096);
  auto hmac_signature =
      "\x55\x51\x3a\xfc\xd2\x4b\xd0\x4c\xbe\x2f\x69\x44\xe0\x26\xf1\xf2"
      "\xb3\xb9\x26\x5c\x02\x60\xac\xba\x7e\xa6\x69\x2d\x29\x51\x55\xf6"sv;
  auto negotiate = cme::ilink::Negotiate{
      .hmac_signature = {reinterpret_cast<std::byte const *>(std::data(hmac_signature)), std::size(hmac_signature)},
      .access_key_id = "uaxIRXP9SuJZ02z7cVN1"sv,
      .uuid = 1685510153123456789,
      .request_timestamp = 1685510153123456789ns,
      .session = "M9H"sv,
      .firm = "ROQ"sv,
  };
  auto message = negotiate.encode(buffer);
  auto message_2 = fmt::format("{}"sv, utils::debug::hex::Message{message});
  auto expected =
      "\x4c\x00"  // block length
      "\xf4\x01"  // template id
      "\x08\x00"  // schema id
      "\x08\x00"  // version
      "\x55\x51\x3a\xfc\xd2\x4b\xd0\x4c\xbe\x2f\x69\x44\xe0\x26\xf1\xf2"
      "\xb3\xb9\x26\x5c\x02\x60\xac\xba\x7e\xa6\x69\x2d\x29\x51\x55\xf6"  // hmac signature
      "\x75\x61\x78\x49\x52\x58\x50\x39\x53\x75"
      "\x4a\x5a\x30\x32\x7a\x37\x63\x56\x4e\x31"  // access key
      "\x15\xe7\x3c\xd6\x8d\x22\x64\x17"          // uuid
      "\x15\xe7\x3c\xd6\x8d\x22\x64\x17"          // request timestamp
      "\x4d\x39\x48"                              // session
      "\x52\x4f\x51\x00\x00"                      // firm
      "\x00\x00"sv;                               // credentials
  auto expected_2 = fmt::format("{}"sv, utils::debug::hex::Message{expected});
  CHECK(message_2 == expected_2);
}

TEST_CASE("decode", "[negotiate]") {
  [[maybe_unused]] auto message =
      "\x58\x00"  // length (88)
      "\xfe\xca"
      "\x4c\x00"  // block length (76)
      "\xf4\x01"  // template id  (500)
      "\x08\x00"  // schema id (8)
      "\x07\x00"  // version (7)
      "\x7a\xb3\xcd\x48\x56\xbf\xb6\x59\xaa\xf0\xd7\xab\xe2\xed\x28\xcf"
      "\xf2\x3c\x76\xec\x7a\xd3\x18\xba\x26\x0a\xc3\x21\x9f\xd9\x0b\xc2"  // hmac signature
      "\x75\x61\x78\x49\x52\x58\x50\x39\x53\x75"
      "\x4a\x5a\x30\x32\x7a\x37\x63\x56\x4e\x31"  // access key (uaxIRXP9SuJZ02z7cVN1)
      "\xec\x08\xfa\x17\xb8\x9a\x63\x17"          // uuid
      "\xec\x08\xfa\x17\xb8\x9a\x63\x17"          // request timestamp
      "\x4d\x39\x48"                              // session (M9H)
      "\x52\x4f\x51\x00\x00"sv;                   // firm (ROQ)
}
