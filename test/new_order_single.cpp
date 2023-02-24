/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include <fmt/format.h>

#include <cmath>
#include <span>

#include <cme_ilink3/MessageHeader.h>
#include <cme_ilink3/NewOrderSingle514.h>

#include "roq/debug/hex/message.hpp"

using namespace std::literals;

using namespace Catch::literals;

using namespace cme_ilink3;

using namespace roq;

namespace {
template <typename T>
auto encode(auto &buffer, auto &message_header, T &value) {
  value.sbeRewind();  // note!
  message_header.wrap(reinterpret_cast<char *>(std::data(buffer)), 0, 0, std::size(buffer))
      .blockLength(NewOrderSingle514::sbeBlockLength())
      .templateId(NewOrderSingle514::sbeTemplateId())
      .schemaId(NewOrderSingle514::sbeSchemaId())
      .version(NewOrderSingle514::sbeSchemaVersion());
  value.wrapForEncode(reinterpret_cast<char *>(std::data(buffer)), message_header.encodedLength(), std::size(buffer));
  value.sbeRewind();  // note!
  auto length = value.computeLength();
  return std::span{std::data(buffer), length};
}
}  // namespace

TEST_CASE("simple", "[new_order_single]") {
  std::vector<std::byte> buffer(4096);
  // MessageHeader message_header;
  NewOrderSingle514 new_order_single;
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
  std::span message{std::data(buffer), NewOrderSingle514::sbeBlockAndHeaderLength()};
  fmt::print(stderr, "{}\n"sv, debug::hex::Message{message});
  fmt::print(
      stderr,
      "{} {} {}\n"sv,
      std::size(message),
      NewOrderSingle514::sbeBlockAndHeaderLength(),
      NewOrderSingle514::sbeBlockLength());
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
