/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include <fmt/chrono.h>
#include <fmt/format.h>

#include <chrono>
#include <string_view>

#include "roq/numbers.hpp"

#include "roq/cme/ilink/utils.hpp"

#include "cme_ilink/ManualOrdIndReq.h"
#include "cme_ilink/SideReq.h"

namespace roq {
namespace cme {
namespace ilink {

struct OrderCancelRequest final {
  std::span<std::byte const> encode(std::span<std::byte> const &buffer) const;

  uint64_t order_id = {};
  uint64_t party_details_list_req_id = {};
  cme_ilink::ManualOrdIndReq::Value manual_order_indicator = {};
  uint32_t seq_num = {};
  std::string_view sender_id;
  std::string_view cl_ord_id;
  uint64_t order_request_id = {};
  std::chrono::nanoseconds sending_time_epoch = {};
  std::string_view location;
  int32_t security_id = {};
  cme_ilink::SideReq::Value side = {};
  bool liquidity_flag = {};
  std::string_view orig_order_user;
};

}  // namespace ilink
}  // namespace cme
}  // namespace roq

template <>
struct fmt::formatter<roq::cme::ilink::OrderCancelRequest> {
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(roq::cme::ilink::OrderCancelRequest const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(order_id={}, )"
        R"(party_details_list_req_id={}, )"
        R"(manual_order_indicator={}, )"
        R"(seq_num={}, )"
        R"(sender_id="{}", )"
        R"(cl_ord_id="{}", )"
        R"(order_request_id={}, )"
        R"(sending_time_epoch={}, )"
        R"(location={}, )"
        R"(security_id={}, )"
        R"(side={}, )"
        R"(liquidity_flag={}, )"
        R"(orig_order_user="{}")"
        R"(}})"sv,
        value.order_id,
        value.party_details_list_req_id,
        value.manual_order_indicator,
        value.seq_num,
        value.sender_id,
        value.cl_ord_id,
        value.order_request_id,
        value.sending_time_epoch,
        value.location,
        value.security_id,
        value.side,
        value.liquidity_flag,
        value.orig_order_user);
  }
};
