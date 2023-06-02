/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/cme/ilink/party_details_list_request.hpp"

#include <cme_ilink/PartyDetailsListRequest537.h>

using namespace std::literals;

namespace roq {
namespace cme {
namespace ilink {

namespace {
using header_type = cme_ilink::MessageHeader;
using value_type = cme_ilink::PartyDetailsListRequest537;
}  // namespace

std::span<std::byte const> PartyDetailsListRequest::encode(std::span<std::byte> const &buffer) const {
  value_type value;
  auto &result = value.wrapAndApplyHeader(reinterpret_cast<char *>(std::data(buffer)), 0, std::size(buffer));
  result.partyDetailsListReqID(party_details_list_req_id);
  result.sendingTimeEpoch(sending_time_epoch.count());
  result.seqNum(seq_num);
  result.noRequestingPartyIDsCount(0);
  result.noPartyIDsCount(0);
  auto length = header_type::encodedLength() + value_type::computeLength(0, 0);
  return {std::data(buffer), length};
}

}  // namespace ilink
}  // namespace cme
}  // namespace roq
