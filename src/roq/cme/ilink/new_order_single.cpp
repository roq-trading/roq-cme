/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/cme/ilink/new_order_single.hpp"

#include <cme_ilink/NewOrderSingle514.h>

using namespace std::literals;

namespace roq {
namespace cme {
namespace ilink {

namespace {
void set_price(cme_ilink::PRICENULL9 &result, double value) {
  using value_type = std::remove_cvref<decltype(result)>::type;
  if (std::isnan(value)) {
    result.mantissa(value_type::mantissaNullValue());
  } else {
    auto mantissa = static_cast<int64_t>(value * std::pow(10.0, static_cast<double>(-value_type::exponent())) + 0.5);
    result.mantissa(mantissa);
  }
}
}  // namespace

namespace {
using header_type = cme_ilink::MessageHeader;
using value_type = cme_ilink::NewOrderSingle514;
}  // namespace

std::span<std::byte const> NewOrderSingle::encode(std::span<std::byte> const &buffer) const {
  value_type value;
  auto &result = value.wrapAndApplyHeader(reinterpret_cast<char *>(std::data(buffer)), 0, std::size(buffer));
  fmt::print(stderr, "{}\n"sv, __LINE__);
  set_price(result.price(), price);
  fmt::print(stderr, "{}\n"sv, __LINE__);
  result.orderQty(order_qty);
  fmt::print(stderr, "{}\n"sv, __LINE__);
  result.securityID(security_id);
  fmt::print(stderr, "{}\n"sv, __LINE__);
  result.side(side);
  fmt::print(stderr, "{}\n"sv, __LINE__);
  result.seqNum(seq_num);
  fmt::print(stderr, "{}\n"sv, __LINE__);
  result.putSenderID(sender_id);
  fmt::print(stderr, "{}\n"sv, __LINE__);
  result.putClOrdID(cl_ord_id);
  fmt::print(stderr, "{}\n"sv, __LINE__);
  result.partyDetailsListReqID(party_details_list_req_id);
  fmt::print(stderr, "{}\n"sv, __LINE__);
  result.orderRequestID(order_request_id);
  fmt::print(stderr, "{}\n"sv, __LINE__);
  result.sendingTimeEpoch(sending_time_epoch.count());
  fmt::print(stderr, "{}\n"sv, __LINE__);
  set_price(result.stopPx(), stop_px);
  fmt::print(stderr, "{}\n"sv, __LINE__);
  result.putLocation(location);
  fmt::print(stderr, "{}\n"sv, __LINE__);
  result.minQty(min_qty ? min_qty : value_type::minQtyNullValue());
  fmt::print(stderr, "{}\n"sv, __LINE__);
  result.displayQty(display_qty ? display_qty : value_type::displayQtyNullValue());
  fmt::print(stderr, "{}\n"sv, __LINE__);
  result.expireDate(value_type::expireDateNullValue());
  fmt::print(stderr, "{}\n"sv, __LINE__);
  result.ordType(ord_type);
  fmt::print(stderr, "{}\n"sv, __LINE__);
  result.timeInForce(time_in_force);
  fmt::print(stderr, "{}\n"sv, __LINE__);
  result.manualOrderIndicator(manual_order_indicator);
  fmt::print(stderr, "{}\n"sv, __LINE__);
  // result.execInst().rawValue(exec_inst.rawValue());
  auto &exec_inst_2 = result.execInst();
  fmt::print(stderr, "{}\n"sv, __LINE__);
  exec_inst_2.clear();
  fmt::print(stderr, "{}\n"sv, __LINE__);
  exec_inst_2.aON(true);
  fmt::print(stderr, "{}\n"sv, __LINE__);
  result.executionMode(execution_mode);
  fmt::print(stderr, "{}\n"sv, __LINE__);
  result.liquidityFlag(liquidity_flag ? cme_ilink::BooleanNULL::True : cme_ilink::BooleanNULL::False);
  fmt::print(stderr, "{}\n"sv, __LINE__);
  result.managedOrder(managed_order ? cme_ilink::BooleanNULL::True : cme_ilink::BooleanNULL::False);
  fmt::print(stderr, "{}\n"sv, __LINE__);
  result.shortSaleType(short_sale_type);
  fmt::print(stderr, "{}\n"sv, __LINE__);
  set_price(result.discretionPrice(), discretion_price);
  fmt::print(stderr, "{}\n"sv, __LINE__);
  set_price(result.reservationPrice(), reservation_price);
  fmt::print(stderr, "{}\n"sv, __LINE__);
  auto length = header_type::encodedLength() + value_type::computeLength();
  return {std::data(buffer), length};
}

}  // namespace ilink
}  // namespace cme
}  // namespace roq
