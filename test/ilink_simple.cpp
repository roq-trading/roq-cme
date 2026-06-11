/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include <fmt/format.h>

#include <cmath>
#include <span>

#include "roq/cme/protocol/ilink/parser.hpp"

using namespace std::literals;

using namespace Catch::literals;

using namespace roq;
using namespace roq::cme;

TEST_CASE("simple", "[ilink_simple]") {
  constexpr auto const message =
      "\x4f\x00"  // message size
      "\xfe\xca"  // encoding type
      "\x42\x00"  // block length
      "\xfb\x01"  // template id (Terminate507)
      "\x08\x00"  // schema id
      "\x08\x00"  // version
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"  // reason
      "\x00\x00\x00\x00\x00\x00\x00\x00"                                                  // uuid
      "\xbc\x85\xa8\x00\x40\x42\x5f\x17"                                                  // request timestamp
      "\x12\x00"                                                                          // error codes
      "\xff"sv;                                                                           // split msg
  static_assert(std::size(message) == 79);
  struct MyHandler final : public protocol::ilink::Parser::Handler {
    // session
    void operator()(Trace<::cme::sbe::ilink::NegotiationResponse501> const &) { FAIL(); }
    void operator()(Trace<::cme::sbe::ilink::NegotiationReject502> const &) { FAIL(); }
    void operator()(Trace<::cme::sbe::ilink::EstablishmentAck504> const &) { FAIL(); }
    void operator()(Trace<::cme::sbe::ilink::EstablishmentReject505> const &) { FAIL(); }
    void operator()(Trace<::cme::sbe::ilink::Sequence506> const &) { FAIL(); }
    void operator()(Trace<::cme::sbe::ilink::Terminate507> const &) { ++counter; }
    void operator()(Trace<::cme::sbe::ilink::Retransmission509> const &) { FAIL(); }
    void operator()(Trace<::cme::sbe::ilink::RetransmitReject510> const &) { FAIL(); }
    void operator()(Trace<::cme::sbe::ilink::NotApplied513> const &) { FAIL(); }
    // business
    void operator()(Trace<::cme::sbe::ilink::PartyDetailsDefinitionRequestAck519> const &) { FAIL(); }
    void operator()(Trace<::cme::sbe::ilink::BusinessReject521> const &) { FAIL(); }
    // execution report
    void operator()(Trace<::cme::sbe::ilink::ExecutionReportNew522> const &) { FAIL(); }
    void operator()(Trace<::cme::sbe::ilink::ExecutionReportReject523> const &) { FAIL(); }
    void operator()(Trace<::cme::sbe::ilink::ExecutionReportTradeOutright525> const &) { FAIL(); }
    void operator()(Trace<::cme::sbe::ilink::ExecutionReportTradeSpread526> const &) { FAIL(); }
    void operator()(Trace<::cme::sbe::ilink::ExecutionReportTradeSpreadLeg527> const &) { FAIL(); }
    void operator()(Trace<::cme::sbe::ilink::ExecutionReportModify531> const &) { FAIL(); }
    void operator()(Trace<::cme::sbe::ilink::ExecutionReportStatus532> const &) { FAIL(); }
    void operator()(Trace<::cme::sbe::ilink::ExecutionReportCancel534> const &) { FAIL(); }
    void operator()(Trace<::cme::sbe::ilink::ExecutionReportPendingCancel564> const &) { FAIL(); }
    void operator()(Trace<::cme::sbe::ilink::ExecutionReportPendingReplace565> const &) { FAIL(); }
    // order
    void operator()(Trace<::cme::sbe::ilink::OrderCancelReject535> const &) { FAIL(); }
    void operator()(Trace<::cme::sbe::ilink::OrderCancelReplaceReject536> const &) { FAIL(); }
    // order mass action
    void operator()(Trace<::cme::sbe::ilink::OrderMassActionReport562> const &) { FAIL(); }
    // security definition
    void operator()(Trace<::cme::sbe::ilink::SecurityDefinitionResponse561> const &) { FAIL(); }
    int counter = 0;
  } handler;
  std::span buffer{reinterpret_cast<std::byte const *>(std::data(message)), std::size(message)};
  TraceInfo trace_info;
  auto res = protocol::ilink::Parser::dispatch(handler, buffer, trace_info);
  CHECK(res);
  CHECK(handler.counter == 1);
}

TEST_CASE("multiple", "[ilink_simple]") {
  constexpr auto const message =
      "\x56\x01\xfe\xca\x4a\x01\x09\x02\x08\x00\x08\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff"
      "\x50\xd1\xa8\x00\x40\x42\x5f\x17\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\x6d"
      "\x00\x00\x00\x00\xff\xff\x4f\x00\xfe\xca\x43\x00\xfb\x01\x08\x00\x08\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xbc\x85\xa8\x00\x40\x42\x5f\x17\x12\x00"
      "\xff"sv;
  static_assert(std::size(message) == 421);
  struct MyHandler final : public protocol::ilink::Parser::Handler {
    // session
    void operator()(Trace<::cme::sbe::ilink::NegotiationResponse501> const &) { FAIL(); }
    void operator()(Trace<::cme::sbe::ilink::NegotiationReject502> const &) { FAIL(); }
    void operator()(Trace<::cme::sbe::ilink::EstablishmentAck504> const &) { FAIL(); }
    void operator()(Trace<::cme::sbe::ilink::EstablishmentReject505> const &) { FAIL(); }
    void operator()(Trace<::cme::sbe::ilink::Sequence506> const &) { FAIL(); }
    void operator()(Trace<::cme::sbe::ilink::Terminate507> const &) { ++counter; }
    void operator()(Trace<::cme::sbe::ilink::Retransmission509> const &) { FAIL(); }
    void operator()(Trace<::cme::sbe::ilink::RetransmitReject510> const &) { FAIL(); }
    void operator()(Trace<::cme::sbe::ilink::NotApplied513> const &) { FAIL(); }
    // business
    void operator()(Trace<::cme::sbe::ilink::PartyDetailsDefinitionRequestAck519> const &) { ++counter; }
    void operator()(Trace<::cme::sbe::ilink::BusinessReject521> const &) { ++counter; }
    // execution report
    void operator()(Trace<::cme::sbe::ilink::ExecutionReportNew522> const &) { FAIL(); }
    void operator()(Trace<::cme::sbe::ilink::ExecutionReportReject523> const &) { FAIL(); }
    void operator()(Trace<::cme::sbe::ilink::ExecutionReportTradeOutright525> const &) { FAIL(); }
    void operator()(Trace<::cme::sbe::ilink::ExecutionReportTradeSpread526> const &) { FAIL(); }
    void operator()(Trace<::cme::sbe::ilink::ExecutionReportTradeSpreadLeg527> const &) { FAIL(); }
    void operator()(Trace<::cme::sbe::ilink::ExecutionReportModify531> const &) { FAIL(); }
    void operator()(Trace<::cme::sbe::ilink::ExecutionReportStatus532> const &) { FAIL(); }
    void operator()(Trace<::cme::sbe::ilink::ExecutionReportCancel534> const &) { FAIL(); }
    void operator()(Trace<::cme::sbe::ilink::ExecutionReportPendingCancel564> const &) { FAIL(); }
    void operator()(Trace<::cme::sbe::ilink::ExecutionReportPendingReplace565> const &) { FAIL(); }
    // order
    void operator()(Trace<::cme::sbe::ilink::OrderCancelReject535> const &) { FAIL(); }
    void operator()(Trace<::cme::sbe::ilink::OrderCancelReplaceReject536> const &) { FAIL(); }
    // order mass action
    void operator()(Trace<::cme::sbe::ilink::OrderMassActionReport562> const &) { FAIL(); }
    // security definition
    void operator()(Trace<::cme::sbe::ilink::SecurityDefinitionResponse561> const &) { FAIL(); }
    int counter = 0;
  } handler;
  std::span buffer{reinterpret_cast<std::byte const *>(std::data(message)), std::size(message)};
  TraceInfo trace_info;
  while (true) {
    auto bytes = protocol::ilink::Parser::dispatch(handler, buffer, trace_info);
    if (!bytes) {
      break;
    }
    buffer = buffer.subspan(bytes);
  }
  CHECK(handler.counter == 2);
}
