/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include <fmt/format.h>

#include <cmath>
#include <span>

#include "roq/cme/ilink/parser.hpp"

using namespace std::literals;

using namespace Catch::literals;

using namespace roq;
using namespace roq::cme;

TEST_CASE("not_complete", "[execution_report_outright]") {
  constexpr auto const message =
      "\x46\x01"  // message length (326)
      "\xfe\xca"
      "\x25\x01"                          // block length
      "\x0d\x02"                          // template id (525)
      "\x08\x00"                          // schema id (8)
      "\x08\x00"                          // version (8)
      "\x12\x00\x00\x00"                  // seqnum
      "\x93\x28\xed\xd0\xd9\x04\x06\x00"  // uuid
      "\x38\x34\x39\x32\x39\x3a\x4d\x3a\x31\x32\x31\x38\x34\x37\x54\x4e\x30\x30\x30\x34"
      "\x35\x32\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"  // exec id
      "\x41\x31\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"  // sender id
      "\x5a\x77\x49\x45\x6e\x64\x69\x34\x6d\x55\x4d\x41\x41\x51\x41\x41\x41\x41\x41\x41"  // cl ord id
      "\x00\x00\x00\x00\x00\x00\x00\x00"                                                  // party details list req id
      "\xe8\xdd\xfd\x95\x19\x00\x00\x00"                                                  // last px
      "\xbf\x3e\xcc\xc8\x13\x00\x00\x00"                                                  // order id
      "\x68\x90\xe4\xa4\x19\x00\x00\x00"                                                  // price
      "\xff\xff\xff\xff\xff\xff\xff\x7f"                                                  // stop px
      "\x05\x94\x30\xa4\xfe\xf2\x82\x17"                                                  // transact time
      "\xff\x89\x7e\xa4\xfe\xf2\x82\x17"                                                  // sending time epoch
      "\x9d\xd8\xb8\x99\x43\x00\x04\x01"                                                  // order request id
      "\xa8\x11\x00\x00\x00\x00\x00\x00"                                                  // sec exec id
      "\xff\xff\xff\xff\xff\xff\xff\xff"                                                  // cross id
      "\xff\xff\xff\xff\xff\xff\xff\xff"                                                  // host cross id
      "\x55\x4b\x00\x00\x00"                                                              // location
      "\xb4\x2f\x00\x00"                                                                  // security id
      "\x01\x00\x00\x00"                                                                  // order qty
      "\x01\x00\x00\x00"                                                                  // last qty
      "\x01\x00\x00\x00"                                                                  // cum qty
      "\x46\xc3\x6c\x00"                                                                  // md trade entry id
      "\xa8\x11\x00\x00"                                                                  // side trade id
      "\xff\xff\xff\xff"                                                                  // trade link id
      "\x00\x00\x00\x00"                                                                  // leaves qty
      "\x98\x4c"                                                                          // trade date
      "\x00\x00"                                                                          // expire date
      "\x02"                                                                              // ord status (filled)
      "\x31"                                  // ord type (1 = market with protection)
      "\x01"                                  // side (1 = buy)
      "\x01"                                  // time in force (1 = gtc)
      "\x00"                                  // manual order indicator
      "\x00"                                  // poss retrans flag
      "\x01"                                  // aggressor indicator (1 = aggressor)
      "\xff"                                  // cross type
      "\x00"                                  // exec inst
      "\x00"                                  // execution mode
      "\xff"                                  // liquidity flag
      "\xff"                                  // managed order
      "\xff"                                  // short sale type
      "\x09"                                  // ownership
      "\xff\xff\xff\xff\xff\xff\xff\x7f"      // discretion price
      "\xff\xff"                              // trd type
      "\xff"                                  // exec restatement reason
      "\xff\xff"                              // settl date
      "\xff\xff"                              // maturity date
      "\xff\xff\xff\xff\xff\xff\xff\x7f\x7f"  // calculated ccy last qty
      "\xff\xff\xff\xff\xff\xff\xff\x7f\x7f"  // gross trade amt
      "\xff\xff\xff\xff\xff\xff\xff\x7f"      // benchmark price
      "\xff\xff\xff\xff\xff\xff\xff\x7f"      // reservation price
      "\xff"                                  // priority indicator
      "\xff\xff\xff\xff\xff"sv;               // display limit price !!! BUT !!! truncated
  static_assert(std::size(message) == 302);
  struct MyHandler final : public ilink::Parser::Handler {
    // session
    void operator()(Trace<cme_ilink::NegotiationResponse501> const &) { FAIL(); }
    void operator()(Trace<cme_ilink::NegotiationReject502> const &) { FAIL(); }
    void operator()(Trace<cme_ilink::EstablishmentAck504> const &) { FAIL(); }
    void operator()(Trace<cme_ilink::EstablishmentReject505> const &) { FAIL(); }
    void operator()(Trace<cme_ilink::Sequence506> const &) { FAIL(); }
    void operator()(Trace<cme_ilink::Terminate507> const &) { FAIL(); }
    void operator()(Trace<cme_ilink::Retransmission509> const &) { FAIL(); }
    void operator()(Trace<cme_ilink::RetransmitReject510> const &) { FAIL(); }
    void operator()(Trace<cme_ilink::NotApplied513> const &) { FAIL(); }
    // business
    void operator()(Trace<cme_ilink::PartyDetailsDefinitionRequestAck519> const &) { FAIL(); }
    void operator()(Trace<cme_ilink::BusinessReject521> const &) { FAIL(); }
    // execution report
    void operator()(Trace<cme_ilink::ExecutionReportNew522> const &) { FAIL(); }
    void operator()(Trace<cme_ilink::ExecutionReportReject523> const &) { FAIL(); }
    void operator()(Trace<cme_ilink::ExecutionReportTradeOutright525> const &) { ++counter; }
    void operator()(Trace<cme_ilink::ExecutionReportTradeSpread526> const &) { FAIL(); }
    void operator()(Trace<cme_ilink::ExecutionReportTradeSpreadLeg527> const &) { FAIL(); }
    void operator()(Trace<cme_ilink::ExecutionReportModify531> const &) { FAIL(); }
    void operator()(Trace<cme_ilink::ExecutionReportStatus532> const &) { FAIL(); }
    void operator()(Trace<cme_ilink::ExecutionReportCancel534> const &) { FAIL(); }
    void operator()(Trace<cme_ilink::ExecutionReportPendingCancel564> const &) { FAIL(); }
    void operator()(Trace<cme_ilink::ExecutionReportPendingReplace565> const &) { FAIL(); }
    // order
    void operator()(Trace<cme_ilink::OrderCancelReject535> const &) { FAIL(); }
    void operator()(Trace<cme_ilink::OrderCancelReplaceReject536> const &) { FAIL(); }
    // order mass action
    void operator()(Trace<cme_ilink::OrderMassActionReport562> const &) { FAIL(); }
    // security definition
    void operator()(Trace<cme_ilink::SecurityDefinitionResponse561> const &) { FAIL(); }
    int counter = 0;
  } handler;
  std::span buffer{reinterpret_cast<std::byte const *>(std::data(message)), std::size(message)};
  TraceInfo trace_info;
  auto res = ilink::Parser::dispatch(handler, buffer, trace_info);
  CHECK(res == 0);  // not complete
  CHECK(handler.counter == 0);
}
