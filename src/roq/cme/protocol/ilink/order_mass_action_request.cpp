/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/cme/protocol/ilink/order_mass_action_request.hpp"

#include <cme/sbe/ilink/OrderMassActionRequest529.h>

using namespace std::literals;

namespace roq {
namespace cme {
namespace protocol {
namespace ilink {

namespace {
using header_type = ::cme::sbe::ilink::MessageHeader;
using value_type = ::cme::sbe::ilink::OrderMassActionRequest529;
}  // namespace

std::span<std::byte const> OrderMassActionRequest::encode(std::span<std::byte> const &buffer) const {
  value_type value;
  auto &result = value.wrapAndApplyHeader(reinterpret_cast<char *>(std::data(buffer)), 0, std::size(buffer));
  result.partyDetailsListReqID(party_details_list_req_id);
  result.orderRequestID(order_request_id);
  result.manualOrderIndicator(manual_order_indicator);
  result.seqNum(seq_num);
  result.putSenderID(sender_id);
  result.sendingTimeEpoch(sending_time_epoch.count());
  result.putSecurityGroup(security_group);
  result.putLocation(location);
  result.securityID(security_id ? security_id : value_type::securityIDNullValue());
  result.massActionScope(mass_action_scope);
  result.marketSegmentID(market_segment_id);
  result.massCancelRequestType(mass_cancel_request_type);
  result.side(side);
  result.ordType(ord_type);
  result.timeInForce(time_in_force);
  result.liquidityFlag(liquidity_flag);
  result.putOrigOrderUser(orig_order_user);
  auto length = header_type::encodedLength() + value_type::computeLength();
  return {std::data(buffer), length};
}

}  // namespace ilink
}  // namespace protocol
}  // namespace cme
}  // namespace roq
