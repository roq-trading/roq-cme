/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/cme/ilink/security_definition_request.hpp"

#include <cme/sbe/ilink/SecurityDefinitionRequest560.h>

using namespace std::literals;

namespace roq {
namespace cme {
namespace ilink {

namespace {
using header_type = ::cme::sbe::ilink::MessageHeader;
using value_type = ::cme::sbe::ilink::SecurityDefinitionRequest560;
}  // namespace

std::span<std::byte const> SecurityDefinitionRequest::encode(std::span<std::byte> const &buffer) const {
  value_type value;
  auto &result = value.wrapAndApplyHeader(reinterpret_cast<char *>(std::data(buffer)), 0, std::size(buffer));
  result.partyDetailsListReqID(party_details_list_req_id);
  result.securityReqID(security_req_id);
  result.manualOrderIndicator(manual_order_indicator);
  result.seqNum(seq_num);
  result.putSenderID(sender_id);
  result.sendingTimeEpoch(sending_time_epoch.count());
  result.putSecuritySubType(security_sub_type);
  result.putLocation(location);
  // startDate
  // endDate
  result.maxNoOfSubstitutions(max_no_of_substitutions);
  result.sourceRepoID(source_repo_id);
  result.brokenDateTermType(broken_date_term_type);
  result.noLegsCount(0);
  result.noBrokenDatesCount(0);
  auto length = header_type::encodedLength() + value_type::computeLength(0, 0);
  return {std::data(buffer), length};
}

}  // namespace ilink
}  // namespace cme
}  // namespace roq
