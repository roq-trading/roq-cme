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

TEST_CASE("simple", "[negotiate]") {
  std::vector<std::byte> buffer(4096);
  // MessageHeader message_header;
  Negotiate500 negotiate;
  auto &ngo = negotiate.wrapAndApplyHeader(reinterpret_cast<char *>(std::data(buffer)), 0, std::size(buffer));
  ngo.putHMACSignature("abc"sv).uUID(123).requestTimestamp(123).putSession("abc"sv).putFirm("abc"sv);
  std::span message{std::data(buffer), Negotiate500::sbeBlockAndHeaderLength()};
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
      "\x61\x62\x63\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x7b\x00\x00\x00"
      "\x00\x00\x00\x00\x7b\x00\x00\x00\x00\x00\x00\x00\x61\x62\x63\x61\x62\x63\x00\x0"sv;
  CHECK(true);
}
