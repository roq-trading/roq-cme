/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include <fmt/chrono.h>
#include <fmt/format.h>

#include <magic_enum/magic_enum_format.hpp>

#include <chrono>
#include <string_view>

#include <cme_ilink/AvgPxInd.h>
#include <cme_ilink/ClearingAcctType.h>
#include <cme_ilink/CmtaGiveUpCD.h>
#include <cme_ilink/CustOrdHandlInst.h>
#include <cme_ilink/CustOrderCapacity.h>
#include <cme_ilink/ListUpdAct.h>
#include <cme_ilink/SLEDS.h>
#include <cme_ilink/SMPI.h>

#include "roq/limits.hpp"

#include "roq/cme/ilink/utils.hpp"

namespace roq {
namespace cme {
namespace ilink {

struct PartyDetailsDefinitionRequest final {
  std::span<std::byte const> encode(std::span<std::byte> const &buffer) const;

  uint64_t party_details_list_req_id = {};
  std::chrono::nanoseconds sending_time_epoch = {};
  cme_ilink::ListUpdAct::Value list_update_action = {};
  uint32_t seq_num = {};
  std::string_view memo;
  std::string_view avg_px_group_id;
  uint64_t self_match_prevention_id = {};
  cme_ilink::CmtaGiveUpCD::Value cmta_giveup_cd = {};
  cme_ilink::CustOrderCapacity::Value cust_order_capacity = {};
  cme_ilink::ClearingAcctType::Value clearing_account_type = {};
  cme_ilink::SMPI::Value self_match_prevention_instruction = {};
  cme_ilink::AvgPxInd::Value avg_px_indicator = {};
  cme_ilink::SLEDS::Value clearing_trade_price_type = {};
  cme_ilink::CustOrdHandlInst::Value cust_order_handling_inst = {};
  uint64_t executor = {};
  uint64_t idm_short_code = {};
  // NoPartyDetails
  struct PartyDetails final {
    std::string_view party_detail_id;
    cme_ilink::PartyDetailRole::Value party_detail_role;
  };
  std::span<PartyDetails> no_party_details;
  // NoTrdRegPublications
};

}  // namespace ilink
}  // namespace cme
}  // namespace roq

template <>
struct fmt::formatter<roq::cme::ilink::PartyDetailsDefinitionRequest> {
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(roq::cme::ilink::PartyDetailsDefinitionRequest const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(party_details_list_req_id={}, )"
        R"(sending_time_epoch={}, )"
        R"(list_update_action={}, )"
        R"(seq_num={}, )"
        R"(memo="{}", )"
        R"(avg_px_group_id="{}", )"
        R"(self_match_prevention_id={}, )"
        R"(cmta_giveup_cd={}, )"
        R"(cust_order_capacity={}, )"
        R"(clearing_account_type={}, )"
        R"(self_match_prevention_instruction={}, )"
        R"(avg_px_indicator={}, )"
        R"(clearing_trade_price_type={}, )"
        R"(cust_order_handling_inst={}, )"
        R"(executor={}, )"
        R"(idm_short_code={})"
        R"(}})"sv,
        value.party_details_list_req_id,
        value.sending_time_epoch,
        value.list_update_action,
        value.seq_num,
        value.memo,
        value.avg_px_group_id,
        value.self_match_prevention_id,
        value.cmta_giveup_cd,
        value.cust_order_capacity,
        value.clearing_account_type,
        value.self_match_prevention_instruction,
        value.avg_px_indicator,
        value.clearing_trade_price_type,
        value.cust_order_handling_inst,
        value.executor,
        value.idm_short_code);
  }
};
