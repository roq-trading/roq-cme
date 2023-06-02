/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/cme/ilink/new_order_single.hpp"

#include <cme_ilink/NewOrderSingle514.h>

using namespace std::literals;

namespace roq {
namespace cme {
namespace ilink {

namespace {
using header_type = cme_ilink::MessageHeader;
using value_type = cme_ilink::NewOrderSingle514;
}  // namespace

std::span<std::byte const> NewOrderSingle::encode(std::span<std::byte> const &buffer) const {
  value_type value;
  auto &result = value.wrapAndApplyHeader(reinterpret_cast<char *>(std::data(buffer)), 0, std::size(buffer));
  result.price().mantissa(100);
  result.orderQty(order_qty);
  result.securityID(security_id);
  result.side(side);
  result.seqNum(seq_num);
  result.putSenderID(sender_id);
  result.putClOrdID(cl_ord_id);
  result.partyDetailsListReqID(party_details_list_req_id);
  result.orderRequestID(order_request_id);
  result.sendingTimeEpoch(sending_time_epoch);
  result.stopPx().mantissa(cme_ilink::PRICENULL9::mantissaNullValue());
  result.putLocation(location);
  result.minQty(min_qty);
  result.displayQty(display_qty);
  result.expireDate(value_type::expireDateNullValue());
  result.ordType(ord_type);
  result.timeInForce(time_in_force);
  result.manualOrderIndicator(manual_order_indicator);
  result.execInst().rawValue(exec_inst.rawValue());
  // exec_inst_2.aON(true);
  result.executionMode(execution_mode);
  result.liquidityFlag(liquidity_flag ? cme_ilink::BooleanNULL::True : cme_ilink::BooleanNULL::False);
  result.managedOrder(managed_order ? cme_ilink::BooleanNULL::True : cme_ilink::BooleanNULL::False);
  result.shortSaleType(short_sale_type);
  result.discretionPrice().mantissa(cme_ilink::PRICENULL9::mantissaNullValue());
  result.reservationPrice().mantissa(cme_ilink::PRICENULL9::mantissaNullValue());
  auto length = header_type::encodedLength() + value_type::sbeBlockLength();
  return {std::data(buffer), length};
}

}  // namespace ilink
}  // namespace cme
}  // namespace roq
