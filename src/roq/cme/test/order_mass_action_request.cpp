/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include <fmt/format.h>

#include <cmath>
#include <span>

#include <cme_ilink/MessageHeader.h>
#include <cme_ilink/OrderMassActionRequest529.h>

#include "roq/debug/hex/message.hpp"

#include "roq/cme/ilink/order_mass_action_request.hpp"

using namespace std::literals;

using namespace Catch::literals;

using namespace cme_ilink;

using namespace roq;
using namespace roq::cme;

namespace {
using value_type = cme_ilink::OrderMassActionRequest529;
}

TEST_CASE("order_mass_action_request_simple", "[order_mass_action_request]") {
  std::vector<std::byte> buffer(4096);
  auto order_mass_action_request = ilink::OrderMassActionRequest{
      .party_details_list_req_id = {},  // note!
      .order_request_id = 1,
      .manual_order_indicator = cme_ilink::ManualOrdIndReq::Automated,
      .seq_num = 1,
      .sender_id = "A1"sv,
      .sending_time_epoch = 1ns,
      .security_group = {},
      .location = "UK"sv,
      .security_id = {},
      .mass_action_scope = cme_ilink::MassActionScope::MarketSegmentID,
      .market_segment_id = 84,
      .mass_cancel_request_type = cme_ilink::MassCxlReqTyp::Account,
      .side = cme_ilink::SideNULL::NULL_VALUE,
      // .ord_type = cme_ilink::MassActionOrdTyp::NULL_VALUE,
      .ord_type = cme_ilink::MassActionOrdTyp::Limit,
      .time_in_force = cme_ilink::MassCancelTIF::NULL_VALUE,
      .liquidity_flag = cme_ilink::BooleanNULL::NULL_VALUE,
      .orig_order_user = {},
  };
  auto message = order_mass_action_request.encode(buffer);
  fmt::print(stderr, "{}\n"sv, debug::hex::Message{message});
  fmt::print(stderr, "{}\n"sv, debug::hex::Message{message.subspan(8 + 68, 1)});
  // fmt::print(stderr, "{}\n"sv, order_mass_action_request.ord_type);
  fmt::print(stderr, "{}\n"sv, debug::hex::Message{message.subspan(8 + 70, 1)});
  // fmt::print(stderr, "{}\n"sv, order_mass_action_request.liquidity_flag);
}

TEST_CASE("simple", "[order_mass_action_request]") {
  auto message =
      "\x5b\x00"
      "\xfe\xca"
      "\x4f\x00"
      "\x11\x02"
      "\x08\x00"
      "\x08\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00"  // party details list req id
      "\x01\x00\x00\x00\x00\x00\x00\x00"  // order request id (1)
      "\x00"                              // manual order indicator (0=automated)
      "\x02\x00\x00\x00"                  // seq num
      "\x41\x31\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"  // sender id (A1)
      "\x82\xc6\xf9\x54\xfa\xb6\x65\x17"          // sending time epoch
      "\x00\x00\x00\x00\x00\x00"                  // security group
      "\x55\x4b\x00\x00\x00"                      // location (UK)
      "\xff\xff\xff\x7f"                          // security id (null)
      "\x09"                                      // mass action scope (9=market segment)
      "\x54"                                      // market segment id (84)
      "\x65"                                      // mass cancel request type (101=account)
      "\xff"                                      // side (null)
      "\x30"                                      // ord type (null)
      "\xff"                                      // time in force (null)
      "\x00"                                      // liquidity flag
      "\x00\x00\x00\x00\x00\x00\x00\x00"sv;       // orig order user
  //\x5b\x00\xfe\xca\x4f\x00\x11\x02\x08\x00\x08\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00\x00\x00\x41\x31\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xcb\x8c\x9b\x71\xc0\x40\x66\x17\x00\x00\x00\x00\x00\x00\x55\x4b\x00\x00\x00\xff\xff\xff\x7f\x09\x54\x65\xff\x30\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00
  auto msg =
      "\x5b\x00"
      "\xfe\xca"
      "\x4f\x00"
      "\x11\x02"
      "\x08\x00"
      "\x08\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x01\x00\x00\x00\x00\x00\x00\x00"
      "\x00"
      "\x02\x00\x00\x00"
      "\x41\x31\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\xcb\x8c\x9b\x71\xc0\x40\x66\x17"
      "\x00\x00\x00\x00\x00\x00"
      "\x55\x4b\x00\x00\x00"
      "\xff\xff\xff\x7f"
      "\x09"
      "\x54"
      "\x65"
      "\xff"
      "\x30"
      "\xff"
      "\xff"
      "\x00\x00\x00\x00\x00\x00\x00\x00"sv;
}
