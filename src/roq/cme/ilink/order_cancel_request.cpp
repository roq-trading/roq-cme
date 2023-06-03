/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/cme/ilink/order_cancel_request.hpp"

#include <cme_ilink/OrderCancelRequest516.h>

using namespace std::literals;

namespace roq {
namespace cme {
namespace ilink {

namespace {
using header_type = cme_ilink::MessageHeader;
using value_type = cme_ilink::OrderCancelRequest516;
}  // namespace

std::span<std::byte const> OrderCancelRequest::encode(std::span<std::byte> const &buffer) const {
  value_type value;
  auto &result = value.wrapAndApplyHeader(reinterpret_cast<char *>(std::data(buffer)), 0, std::size(buffer));
  result.orderID(value_type::orderIDNullValue());
  result.partyDetailsListReqID(party_details_list_req_id);
  result.manualOrderIndicator(manual_order_indicator);
  result.seqNum(seq_num);
  result.putSenderID(sender_id);
  result.putClOrdID(cl_ord_id);
  result.orderRequestID(order_request_id);
  result.sendingTimeEpoch(sending_time_epoch.count());
  result.putLocation(location);
  result.securityID(security_id);
  result.side(side);
  result.liquidityFlag(liquidity_flag ? cme_ilink::BooleanNULL::True : cme_ilink::BooleanNULL::False);
  result.putOrigOrderUser(orig_order_user);
  auto length = header_type::encodedLength() + value_type::computeLength();
  return {std::data(buffer), length};
}

}  // namespace ilink
}  // namespace cme
}  // namespace roq
