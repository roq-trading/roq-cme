/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include <fmt/format.h>

#include <cmath>
#include <span>

#include <cme_ilink/MessageHeader.h>
#include <cme_ilink/NewOrderSingle514.h>

#include "roq/debug/hex/message.hpp"

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
  nso.executionMode(ExecMode::Aggressive)
      .liquidityFlag(BooleanNULL::True)
      .managedOrder(BooleanNULL::True)
      .shortSaleType(ShortSaleType::LongSell);
  nso.discretionPrice().mantissa(PRICENULL9::mantissaNullValue());
  /*
  auto &price = new_order_single.price();
  auto mantissa = static_cast<uint64_t>(std::pow(10, price.exponent()) * 123.4);  // round
  price.mantissa(mantissa);
  auto message = encode(buffer, message_header, new_order_single);
  */
  std::span message{std::data(buffer), value_type::sbeBlockAndHeaderLength()};
  fmt::print(stderr, "{}\n"sv, debug::hex::Message{message});
  fmt::print(
      stderr, "{} {} {}\n"sv, std::size(message), value_type::sbeBlockAndHeaderLength(), value_type::sbeBlockLength());
  auto expected =
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
