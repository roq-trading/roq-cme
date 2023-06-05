/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include <fmt/format.h>

#include <cmath>
#include <span>

#include <cme_ilink/MessageHeader.h>
#include <cme_ilink/OrderMassActionRequest529.h>

#include "roq/debug/hex/message.hpp"

using namespace std::literals;

using namespace Catch::literals;

using namespace cme_ilink;

using namespace roq;
// using namespace roq::cme;

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
}
