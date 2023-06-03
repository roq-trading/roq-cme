/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <fmt/chrono.h>
#include <fmt/format.h>

#include <chrono>
#include <string_view>

#include "roq/numbers.hpp"

#include "roq/cme/ilink/utils.hpp"

#include "cme_ilink/ExecInst.h"
#include "cme_ilink/ExecMode.h"
#include "cme_ilink/ManualOrdIndReq.h"
#include "cme_ilink/OrderTypeReq.h"
#include "cme_ilink/ShortSaleType.h"
#include "cme_ilink/SideReq.h"
#include "cme_ilink/TimeInForce.h"

namespace roq {
namespace cme {
namespace ilink {

struct NewOrderSingle final {
  std::span<std::byte const> encode(std::span<std::byte> const &buffer) const;

  double price = NaN;
  uint32_t order_qty = {};
  int32_t security_id = {};
  cme_ilink::SideReq::Value side = {};
  uint32_t seq_num = {};
  std::string_view sender_id;
  std::string_view cl_ord_id;
  uint64_t party_details_list_req_id = {};
  uint64_t order_request_id = {};
  std::chrono::nanoseconds sending_time_epoch = {};
  double stop_px = NaN;
  std::string_view location;
  uint32_t min_qty = {};
  uint32_t display_qty = {};
  std::chrono::seconds expire_date = {};
  cme_ilink::OrderTypeReq::Value ord_type = {};
  cme_ilink::TimeInForce::Value time_in_force = {};
  cme_ilink::ManualOrdIndReq::Value manual_order_indicator = {};
  cme_ilink::ExecInst exec_inst = {};
  cme_ilink::ExecMode::Value execution_mode = {};
  bool liquidity_flag = {};
  bool managed_order = {};
  cme_ilink::ShortSaleType::Value short_sale_type = {};
  double discretion_price = NaN;
  double reservation_price = NaN;
};

}  // namespace ilink
}  // namespace cme
}  // namespace roq

template <>
struct fmt::formatter<roq::cme::ilink::NewOrderSingle> {
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(roq::cme::ilink::NewOrderSingle const &value, Context &context) const {
    using namespace std::literals;
    using namespace fmt::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(price={}, )"
        R"(order_qty={}, )"
        R"(security_id={}, )"
        R"(side={}, )"
        R"(seq_num={}, )"
        R"(sender_id="{}", )"
        R"(cl_ord_id="{}", )"
        R"(party_details_list_req_id={}, )"
        R"(order_request_id={}, )"
        R"(sending_time_epoch={}, )"
        R"(stop_px={}, )"
        R"(location="{}", )"
        R"(min_qty={}, )"
        R"(display_qty={}, )"
        R"(expire_date={}, )"
        R"(ord_type={}, )"
        R"(time_in_force={}, )"
        R"(manual_order_indicator={}, )"
        R"(exec_inst={}, )"
        R"(execution_mode={}, )"
        R"(liquidity_flag={}, )"
        R"(managed_order={}, )"
        R"(short_sale_type={}, )"
        R"(discretion_price={}, )"
        R"(reservation_price="{}")"
        R"(}})"_cf,
        value.price,
        value.order_qty,
        value.security_id,
        value.side,
        value.seq_num,
        value.sender_id,
        value.cl_ord_id,
        value.party_details_list_req_id,
        value.order_request_id,
        value.sending_time_epoch,
        value.stop_px,
        value.location,
        value.min_qty,
        value.display_qty,
        value.expire_date,
        value.ord_type,
        value.time_in_force,
        value.manual_order_indicator,
        value.exec_inst,
        value.execution_mode,
        value.liquidity_flag,
        value.managed_order,
        value.short_sale_type,
        value.discretion_price,
        value.reservation_price);
  }
};
