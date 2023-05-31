/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include <fmt/format.h>

#include <cmath>
#include <span>

#include <cme_ilink/MessageHeader.h>
#include <cme_ilink/Negotiate500.h>

#include "roq/debug/hex/message.hpp"

using namespace std::literals;

using namespace Catch::literals;

using namespace cme_ilink;

using namespace roq;

/*
namespace {
template <typename T>
auto encode(auto &buffer, auto &message_header, T &value) {
  value.sbeRewind();  // note!
  message_header.wrap(reinterpret_cast<char *>(std::data(buffer)), 0, 0, std::size(buffer))
      .blockLength(Negotiate500::sbeBlockLength())
      .templateId(Negotiate500::sbeTemplateId())
      .schemaId(Negotiate500::sbeSchemaId())
      .version(Negotiate500::sbeSchemaVersion());
  value.wrapForEncode(reinterpret_cast<char *>(std::data(buffer)), message_header.encodedLength(), std::size(buffer));
  value.sbeRewind();  // note!
  auto length = value.computeLength();
  return std::span{std::data(buffer), length};
}
}  // namespace
*/

TEST_CASE("simple", "[negotiate]") {
  std::vector<std::byte> buffer(4096);
  // MessageHeader message_header;
  Negotiate500 negotiate;
  auto &ngo = negotiate.wrapAndApplyHeader(reinterpret_cast<char *>(std::data(buffer)), 0, std::size(buffer));
  ngo  //
      .putHMACSignature("abc"sv)
      .putAccessKeyID("abc"sv)
      .uUID(123)
      .requestTimestamp(123)
      .putSession("abc"sv)
      .putFirm("abc"sv)
      .skipCredentials();
  auto length = MessageHeader::encodedLength() + Negotiate500::computeLength(0);
  std::span message{std::data(buffer), length};
  fmt::print(stderr, "{}\n"sv, debug::hex::Message{message});
  fmt::print(
      stderr,
      "{} {} {}\n"sv,
      std::size(message),
      Negotiate500::sbeBlockAndHeaderLength(),
      Negotiate500::sbeBlockLength());
  auto expected =
      "\x4c\x00"  // block length
      "\xf4\x01"  // template id (500)
      "\x08\x00"  // schema id (8)
      "\x07\x00"  // version (7)
      "\x61\x62\x63\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x7b\x00\x00"
      "\x00"
      "\x00\x00\x00\x00\x7b\x00\x00\x00\x00\x00\x00\x00\x61\x62\x63\x61\x62\x63\x00\x0"sv;
  CHECK(true);
}

TEST_CASE("decode", "[negotiate]") {
  auto message =
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
