/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include <fmt/format.h>

#include <cmath>
#include <span>

#include <cme_ilink/MessageHeader.h>
#include <cme_ilink/NewOrderSingle514.h>

#include "roq/utils/debug/hex/message.hpp"

using namespace std::literals;

using namespace Catch::literals;

using namespace cme_ilink;

using namespace roq;

using value_type = NewOrderSingle514;

namespace {
template <typename T>
auto encode(auto &buffer, auto &message_header, T &value) {
  value.sbeRewind();  // note!
  message_header.wrap(reinterpret_cast<char *>(std::data(buffer)), 0, 0, std::size(buffer))
      .blockLength(value_type::sbeBlockLength())
      .templateId(value_type::sbeTemplateId())
      .schemaId(value_type::sbeSchemaId())
      .version(value_type::sbeSchemaVersion());
  value.wrapForEncode(reinterpret_cast<char *>(std::data(buffer)), message_header.encodedLength(), std::size(buffer));
  value.sbeRewind();  // note!
  auto length = value.computeLength();
  return std::span{std::data(buffer), length};
}
}  // namespace

TEST_CASE("simple", "[new_order_single]") {
  std::vector<std::byte> buffer(4096);
  // MessageHeader message_header;
  value_type new_order_single;
  auto &nso = new_order_single.wrapAndApplyHeader(reinterpret_cast<char *>(std::data(buffer)), 0, std::size(buffer));
  nso.price().mantissa(77);
  nso.orderQty(123)
      .securityID(123)
      .side(SideReq::Buy)
      .seqNum(123)
      .putSenderID("abc"sv)
      .putClOrdID("abc123"sv)
      .partyDetailsListReqID(0)
      .orderRequestID(123)
      .sendingTimeEpoch(123);
  nso.stopPx().mantissa(77);
  nso.putLocation("abc"sv)
      .minQty(123)
      .displayQty(123)
      .expireDate(123)
      .ordType(OrderTypeReq::Limit)
      .timeInForce(TimeInForce::GoodTillCancel)
      .manualOrderIndicator(ManualOrdIndReq::Automated);
  nso.execInst().aON(true);
  nso.executionMode(ExecMode::Aggressive).liquidityFlag(BooleanNULL::True).managedOrder(BooleanNULL::True).shortSaleType(ShortSaleType::LongSell);
  nso.discretionPrice().mantissa(PRICENULL9::mantissaNullValue());
  /*
  auto &price = new_order_single.price();
  auto mantissa = static_cast<uint64_t>(std::pow(10, price.exponent()) * 123.4);  // round
  price.mantissa(mantissa);
  auto message = encode(buffer, message_header, new_order_single);
  */
  std::span message{std::data(buffer), value_type::sbeBlockAndHeaderLength()};
  fmt::print(stderr, "{}\n"sv, utils::debug::hex::Message{message});
  fmt::print(stderr, "{} {} {}\n"sv, std::size(message), value_type::sbeBlockAndHeaderLength(), value_type::sbeBlockLength());
  [[maybe_unused]] auto expected =
      "\x7c\x00"  // block length
      "\x02\x02"  // template id (514)
      "\x08\x00"  // schema id (8)
      "\x07\x00"  // version (7)
      "\x4d\x00\x00\x00\x00\x00\x00\x00\x7b\x00\x00\x00\x7b\x00\x00\x00\x01\x7b\x00\x00\x00\x61\x62\x63\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x61\x62\x63\x31\x32\x33\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x7b\x00\x00\x00\x00\x00\x00\x00\x7b\x00\x00\x00\x00\x00\x00"
      "\x00\x4d\x00\x00\x00\x00\x00\x00\x00\x61\x62\x63\x00\x00\x7b\x00\x00\x00\x7b\x00\x00\x00\x7b\x00\x32\x01\x00\x01"
      "\x41\x01\x01\x00\xff\xff\xff\xff\xff\xff\xff\x7f"sv;
  CHECK(true);
}

/*
"\x90\x00"
"\xfe\xca"
"\x84\x00"
"\x02\x02"
"\x08\x00"
"\x08\x00"
"\x00\xe8\x76\x48\x17\x00\x00\x00" // price
"\x01\x00\x00\x00" // order qty
"\x8e\x2a\x05\x00" // security id
"\x01" // side (1=buy)
"\x02\x00\x00\x00" // seq num
"\x41\x31\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // sender id
"\x74\x65\x73\x74\x00\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // cl_ord_id
"\x00\x00\x00\x00\x00\x00\x00\x00" // party details list req id
"\x01\x00\x00\x00\x00\x00\x00\x00" // order request id
"\xc3\xfb\xc0\x8b\x3e\x4a\x66\x17" // sending time epoch
"\xff\xff\xff\xff\xff\xff\xff\x7f" // stop px (null)
"\x55\x4b\x00\x00\x00" // location
"\xff\xff\xff\xff" // min qty (null)
"\xff\xff\xff\xff" // display qty (null)
"\xff\xff" // expire date (null)
"\x32" // ord type (2=limit)
"\x01" // time in force (1=GTC)
"\x00" // manual order ind (0=Automated)
"\x00" // exec inst (none)
"\x30" // execution mode (null)
"\x00" // liquidity flag (0=false)
"\x00" // managed order (0=false)
"\xff" // short sale type (null)
"\xff\xff\xff\xff\xff\xff\xff\x7f" // discretion price (null)
"\xff\xff\xff\xff\xff\xff\xff\x7f"sv;  // reservation price (null)
*/

/*
"\x90\x00"
"\xFE\xCA"
"\x84\x00"
"\x02\x02"
"\x08\x00"
"\x08\x00"
"\x00\x1C\x97\x12\x99\xDC\x00\x00"          // OK price
"\x01\x00\x00\x00"                          // OK order qty
"\x63\x1F\x00\x00"                          // OK security id
"\x02"                                      // OK side
"\x84\x05\x02\x00"                          // OK seq num
"\x48\x52\x49\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"  // OK sender id
"\x30\x38\x35\x32\x32\x38\x32\x33\x38\x33"
"\x31\x30\x30\x30\x31\x33\x4E\x4B\x00\x00"  // -- cl_ord_id
"\x00\x00\x00\x00\x00\x00\x00\x00"          // OK party details list req id
"\x8E\xCB\x4C\x95\xA1\xCA\x2E\x01"          // -- order request_id
"\x6E\xDB\xEE\x89\x88\x01\x00\x00"          // OK sending time
"\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x7F"          // OK stop px
"\x55\x53\x2C\x49\x4C"                      // -- location
"\xFF\xFF\xFF\xFF"                          // OK min qty
"\xFF\xFF\xFF\xFF"                          // OK display qty
"\xFF\xFF"                                  // OK expire date
"\x32"                                      // OK ord type
"\x00"                                      // OK time in force
"\x00"                                      // OK manual order ind
"\x00"                                      // OK exec inst
"\x00"                                      // -- execution mode
"\x00"                                      // OK liquidity flag
"\x00"                                      // OK managed order
"\x00"                                      // -- short sale
"\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x7F"          // OK discretion price
"\x00\x00\x00\x00\x00\x00\x00\x00"sv;       // -- reservation price
*/
