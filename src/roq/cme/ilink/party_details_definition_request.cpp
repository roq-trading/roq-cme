/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/cme/ilink/party_details_definition_request.hpp"

#include <cme_ilink/PartyDetailsDefinitionRequest518.h>

using namespace std::literals;

namespace roq {
namespace cme {
namespace ilink {

namespace {
using header_type = cme_ilink::MessageHeader;
using value_type = cme_ilink::PartyDetailsDefinitionRequest518;
}  // namespace

std::span<std::byte const> PartyDetailsDefinitionRequest::encode(std::span<std::byte> const &buffer) const {
  value_type value;
  auto &result = value.wrapAndApplyHeader(reinterpret_cast<char *>(std::data(buffer)), 0, std::size(buffer));
  result.partyDetailsListReqID(party_details_list_req_id);
  result.sendingTimeEpoch(sending_time_epoch.count());
  result.listUpdateAction(list_update_action);
  result.seqNum(seq_num);
  result.putMemo(memo);
  result.putAvgPxGroupID(avg_px_group_id);
  result.selfMatchPreventionID(self_match_prevention_id ? self_match_prevention_id : value_type::selfMatchPreventionIDNullValue());
  result.cmtaGiveupCD(cmta_giveup_cd);
  result.custOrderCapacity(cust_order_capacity);
  result.clearingAccountType(clearing_account_type);
  result.selfMatchPreventionInstruction(self_match_prevention_instruction);
  result.avgPxIndicator(avg_px_indicator);
  result.clearingTradePriceType(clearing_trade_price_type);
  result.custOrderHandlingInst(cust_order_handling_inst);
  result.executor(executor);
  result.iDMShortCode(idm_short_code);
  auto &party_details = result.noPartyDetailsCount(std::size(no_party_details));
  for (auto &item : no_party_details) {
    party_details.next();
    party_details.putPartyDetailID(item.party_detail_id);
    party_details.partyDetailRole(item.party_detail_role);
  }
  result.noTrdRegPublicationsCount(0);
  auto length = header_type::encodedLength() + value_type::computeLength(std::size(no_party_details), 0);
  return {std::data(buffer), length};
}

}  // namespace ilink
}  // namespace cme
}  // namespace roq
