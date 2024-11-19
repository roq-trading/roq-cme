/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include <fmt/format.h>

#include <cmath>
#include <span>

#include <cme_ilink/MessageHeader.h>
#include <cme_ilink/PartyDetailsDefinitionRequest518.h>

#include "roq/utils/debug/hex/message.hpp"

#include "roq/cme/ilink/party_details_definition_request.hpp"

using namespace std::literals;

using namespace Catch::literals;

using namespace cme_ilink;

using namespace roq;
using namespace roq::cme;

TEST_CASE("simple", "[party_details_definition_request]") {
  std::vector<std::byte> buffer(4096);
  auto now = 123s;
  std::array<ilink::PartyDetailsDefinitionRequest::PartyDetails, 1> party_details{{
      {
          .party_detail_id = "ROQ"sv,
          .party_detail_role = cme_ilink::PartyDetailRole::ExecutingFirm,
      },
  }};
  auto party_details_definition_request = ilink::PartyDetailsDefinitionRequest{
      .party_details_list_req_id = {},  // note!
      .sending_time_epoch = now,
      .list_update_action = cme_ilink::ListUpdAct::Add,
      .seq_num = 1,
      .memo = {},
      .avg_px_group_id = {},
      .self_match_prevention_id = {},
      .cmta_giveup_cd = cme_ilink::CmtaGiveUpCD::GiveUp,
      .cust_order_capacity = cme_ilink::CustOrderCapacity::Membertradingfortheirownaccount,
      .clearing_account_type = cme_ilink::ClearingAcctType::Firm,
      .self_match_prevention_instruction = cme_ilink::SMPI::CancelNewest,
      .avg_px_indicator = cme_ilink::AvgPxInd::NoAveragePricing,
      .clearing_trade_price_type = cme_ilink::SLEDS::TradeClearingatExecutionPrice,
      .cust_order_handling_inst = cme_ilink::CustOrdHandlInst::AlgoEngine,
      .executor = {},
      .idm_short_code = {},
      .no_party_details = party_details,
  };
  auto message = party_details_definition_request.encode(buffer);
  fmt::print("{}\n"sv, utils::debug::hex::Message{message});
  [[maybe_unused]] auto tmp = std::string_view{reinterpret_cast<char const *>(std::data(message)), std::size(message)};
  auto tmp_2 = fmt::format("{}"sv, utils::debug::hex::Message{message});
  auto expected =
      "\x93\x00"                          // block length
      "\x06\x02"                          // template id
      "\x08\x00"                          // schema id
      "\x08\x00"                          // version
      "\x00\x00\x00\x00\x00\x00\x00\x00"  // party details list req id
      "\x00\x0e\x5f\xa3\x1c\x00\x00\x00"  // sending time epoch
      "\x41"                              // list update action A=add
      "\x01\x00\x00\x00"                  // seq num
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"                      // memo
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"  // avg px group id
      "\xff\xff\xff\xff\xff\xff\xff\xff"                                                  // self match prevention id
      "\x47"                                                                              // cmta giveup cd G=giveup
      "\x01"                                                                              // cust order capacity
      "\x01"                                                                              // clearing account type
      "\x4e"                                                                              // self match prevention N=newest
      "\x00"                                                                              // avg px indicator
      "\x00"                                                                              // clearing trade price type
      "\x48"                                                                              // cust order handling inst H=algo
      "\x00\x00\x00\x00\x00\x00\x00\x00"                                                  // executor
      "\x00\x00\x00\x00\x00\x00\x00\x00"                                                  // idm short code
      // no party details
      "\x16\x00"                                                                          // block length 22
      "\x01"                                                                              // count
      "\x52\x4f\x51\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"  // party detail id ROQ
      "\x01\x00"                                                                          // party detail role 1=firm
      // no trd reg publications
      "\x02\x00"  // block length
      "\x00"sv;   // count
  auto expected_2 = fmt::format("{}"sv, utils::debug::hex::Message{expected});
  CHECK(tmp_2 == expected_2);
}
