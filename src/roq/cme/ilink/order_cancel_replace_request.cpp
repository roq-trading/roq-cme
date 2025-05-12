/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/cme/ilink/order_cancel_replace_request.hpp"

#include <cme_ilink/OrderCancelReplaceRequest515.h>

using namespace std::literals;

namespace roq {
namespace cme {
namespace ilink {

namespace {
void set_price(cme_ilink::PRICENULL9 &result, double value) {
  using value_type = std::remove_cvref_t<decltype(result)>;
  if (std::isnan(value)) {
    result.mantissa(value_type::mantissaNullValue());
  } else {
    auto mantissa = static_cast<int64_t>(value * std::pow(10.0, -static_cast<double>(value_type::exponent())) + 0.5);
    result.mantissa(mantissa);
  }
}
}  // namespace

namespace {
using header_type = cme_ilink::MessageHeader;
using value_type = cme_ilink::OrderCancelReplaceRequest515;
}  // namespace

std::span<std::byte const> OrderCancelReplaceRequest::encode(std::span<std::byte> const &buffer) const {
  value_type value;
  auto &result = value.wrapAndApplyHeader(reinterpret_cast<char *>(std::data(buffer)), 0, std::size(buffer));
  set_price(result.price(), price);
  result.orderQty(order_qty);
  result.securityID(security_id);
  result.side(side);
  result.seqNum(seq_num);
  result.putSenderID(sender_id);
  result.putClOrdID(cl_ord_id);
  result.partyDetailsListReqID(party_details_list_req_id);
  result.orderID(value_type::orderIDNullValue());
  set_price(result.stopPx(), stop_px);
  result.orderRequestID(order_request_id);
  result.sendingTimeEpoch(sending_time_epoch.count());
  result.putLocation(location);
  result.minQty(min_qty ? min_qty : value_type::minQtyNullValue());
  result.displayQty(display_qty ? display_qty : value_type::displayQtyNullValue());
  result.expireDate(value_type::expireDateNullValue());
  result.ordType(ord_type);
  result.timeInForce(time_in_force);
  result.manualOrderIndicator(manual_order_indicator);
  result.oFMOverride(ofm_override);
  result.execInst().rawValue(exec_inst);
  result.executionMode(execution_mode);
  result.liquidityFlag(liquidity_flag ? cme_ilink::BooleanNULL::True : cme_ilink::BooleanNULL::False);
  result.managedOrder(managed_order ? cme_ilink::BooleanNULL::True : cme_ilink::BooleanNULL::False);
  result.shortSaleType(short_sale_type);
  set_price(result.discretionPrice(), discretion_price);
  auto length = header_type::encodedLength() + value_type::computeLength();
  return {std::data(buffer), length};
}

}  // namespace ilink
}  // namespace cme
}  // namespace roq
