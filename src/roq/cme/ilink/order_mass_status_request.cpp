/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/cme/ilink/order_mass_status_request.hpp"

#include <cme_ilink/OrderMassStatusRequest530.h>

using namespace std::literals;

namespace roq {
namespace cme {
namespace ilink {

namespace {
using header_type = cme_ilink::MessageHeader;
using value_type = cme_ilink::OrderMassStatusRequest530;
}  // namespace

std::span<std::byte const> OrderMassStatusRequest::encode(std::span<std::byte> const &buffer) const {
  value_type value;
  auto &result = value.wrapAndApplyHeader(reinterpret_cast<char *>(std::data(buffer)), 0, std::size(buffer));
  result.partyDetailsListReqID(party_details_list_req_id);
  result.massStatusReqID(mass_status_req_id);
  result.manualOrderIndicator(manual_order_indicator);
  result.seqNum(seq_num);
  result.putSenderID(sender_id);
  result.sendingTimeEpoch(sending_time_epoch.count());
  result.putSecurityGroup(security_group);
  result.putLocation(location);
  result.securityID(security_id ? security_id : value_type::securityIDNullValue());
  result.massStatusReqType(mass_status_req_type);
  result.ordStatusReqType(ord_status_req_type);
  result.timeInForce(time_in_force);
  result.marketSegmentID(market_segment_id ? market_segment_id : value_type::marketSegmentIDNullValue());
  auto length = header_type::encodedLength() + value_type::computeLength();
  return {std::data(buffer), length};
}

}  // namespace ilink
}  // namespace cme
}  // namespace roq
