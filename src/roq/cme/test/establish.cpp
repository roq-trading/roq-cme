/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include <fmt/format.h>

#include <cmath>
#include <span>

#include <cme_ilink/Establish503.h>
#include <cme_ilink/MessageHeader.h>

#include "roq/utils/debug/hex/message.hpp"

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
      .blockLength(Establish503::sbeBlockLength())
      .templateId(Establish503::sbeTemplateId())
      .schemaId(Establish503::sbeSchemaId())
      .version(Establish503::sbeSchemaVersion());
  value.wrapForEncode(reinterpret_cast<char *>(std::data(buffer)), message_header.encodedLength(), std::size(buffer));
  value.sbeRewind();  // note!
  auto length = value.computeLength();
  return std::span{std::data(buffer), length};
}
}  // namespace
*/

TEST_CASE("simple", "[establish]") {
  std::vector<std::byte> buffer(4096);
  // MessageHeader message_header;
  Establish503 establish;
  auto &est = establish.wrapAndApplyHeader(reinterpret_cast<char *>(std::data(buffer)), 0, std::size(buffer));
  est  //
      .putHMACSignature("abc"sv)
      .putAccessKeyID("abc"sv)
      .putTradingSystemName("abc"sv)
      .putTradingSystemVersion("abc"sv)
      .putTradingSystemVendor("abc"sv)
      .uUID(123)
      .requestTimestamp(123)
      .nextSeqNo(123)
      .putSession("abc"sv)
      .putFirm("abc"sv)
      .keepAliveInterval(123);
  std::span message{std::data(buffer), Establish503::sbeBlockAndHeaderLength()};
  fmt::print(stderr, "{}\n"sv, utils::debug::hex::Message{message});
  fmt::print(
      stderr,
      "{} {} {}\n"sv,
      std::size(message),
      Establish503::sbeBlockAndHeaderLength(),
      Establish503::sbeBlockLength());
  [[maybe_unused]] auto expected =
      "\x4c\x00"  // block length
      "\xf4\x01"  // template id (500)
      "\x08\x00"  // schema id (8)
      "\x07\x00"  // version (7)
      "\x61\x62\x63\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x7b\x00\x00\x00"
      "\x00\x00\x00\x00\x7b\x00\x00\x00\x00\x00\x00\x00\x61\x62\x63\x61\x62\x63\x00\x0"sv;
  CHECK(true);
}

TEST_CASE("decode", "[establish]") {
  [[maybe_unused]] auto message =
      "\x92\x00"  // length
      "\xfe\xca"
      "\x84\x00"  // block length
      "\xf7\x01"  // template id (503)
      "\x08\x00"  // schema id
      "\x08\x00"  // version
      "\x00\x5a\x9b\x87\xc4\x1e\x1e\xbf\xc4\x84\x1e\x94\x71\xf9\xd1\xcd"
      "\xb9\x37\x61\x2e\xf5\xdf\x5e\x2e\x59\xf5\x07\x86\x13\xe0\x0f\xd8"  // hmac signature
      "\x75\x61\x78\x49\x52\x58\x50\x39\x53\x75"
      "\x4a\x5a\x30\x32\x7a\x37\x63\x56\x4e\x31"  // access key
      "\x72\x6f\x71\x2d\x63\x6d\x65\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"  // trading system name
      "\x30\x2e\x39\x2e\x34\x00\x00\x00\x00\x00"  // trading system version
      "\x52\x4f\x51\x00\x00\x00\x00\x00\x00\x00"  // trading system vendor
      "\xce\x07\x71\xc7\x07\xfd\x05\x00"          // uuid
      "\x4f\x80\x86\x11\x63\x66\x64\x17"          // request timestamp
      "\x01\x00\x00\x00"                          // next seqno
      "\x4d\x39\x48"                              // session
      "\x52\x4f\x51\x00\x00"                      // firm
      "\x30\x75"                                  // keepalive
      "\x00\x00"sv;                               // credentials
}
