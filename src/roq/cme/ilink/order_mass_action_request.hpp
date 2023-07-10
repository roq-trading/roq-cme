/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <fmt/chrono.h>
#include <fmt/format.h>

#include <chrono>
#include <string_view>

#include "roq/numbers.hpp"

#include "roq/cme/ilink/utils.hpp"

#include "cme_ilink/BooleanNULL.h"
#include "cme_ilink/ManualOrdIndReq.h"
#include "cme_ilink/MassActionScope.h"
#include "cme_ilink/MassCancelTIF.h"
#include "cme_ilink/MassCxlReqTyp.h"
#include "cme_ilink/SideNULL.h"

namespace roq {
namespace cme {
namespace ilink {

struct OrderMassActionRequest final {
  std::span<std::byte const> encode(std::span<std::byte> const &buffer) const;

  uint64_t party_details_list_req_id = {};
  uint64_t order_request_id = {};
  cme_ilink::ManualOrdIndReq::Value manual_order_indicator = {};
  uint32_t seq_num = {};
  std::string_view sender_id;
  std::chrono::nanoseconds sending_time_epoch = {};
  std::string_view security_group;
  std::string_view location;
  int32_t security_id = {};
  cme_ilink::MassActionScope::Value mass_action_scope = {};
  uint8_t market_segment_id = {};
  cme_ilink::MassCxlReqTyp::Value mass_cancel_request_type = {};
  cme_ilink::SideNULL::Value side = cme_ilink::SideNULL::NULL_VALUE;
  // note! null is not actually supported
  cme_ilink::MassActionOrdTyp::Value ord_type = cme_ilink::MassActionOrdTyp::NULL_VALUE;
  cme_ilink::MassCancelTIF::Value time_in_force = cme_ilink::MassCancelTIF::NULL_VALUE;
  cme_ilink::BooleanNULL::Value liquidity_flag = cme_ilink::BooleanNULL::NULL_VALUE;
  std::string_view orig_order_user;
};

}  // namespace ilink
}  // namespace cme
}  // namespace roq

template <>
struct fmt::formatter<roq::cme::ilink::OrderMassActionRequest> {
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(roq::cme::ilink::OrderMassActionRequest const &value, Context &context) const {
    using namespace std::literals;
    using namespace fmt::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(party_details_list_req_id={}, )"
        R"(order_request_id={}, )"
        R"(manual_order_indicator={}, )"
        R"(seq_num={}, )"
        R"(sender_id={}, )"
        R"(time_epoch={}, )"
        R"(security_group="{}", )"
        R"(location="{}", )"
        R"(security_id={}, )"
        R"(mass_action_scope={}, )"
        R"(market_segment_id={}, )"
        R"(mass_cancel_request_type={}, )"
        R"(side={}, )"
        R"(ord_type={}, )"
        R"(time_in_force={}, )"
        R"(liquidity_flag={}, )"
        R"(orig_order_user="{}")"
        R"(}})"_cf,
        value.party_details_list_req_id,
        value.order_request_id,
        magic_enum::enum_name(value.manual_order_indicator),
        value.seq_num,
        value.sender_id,
        value.sending_time_epoch,
        value.security_group,
        value.location,
        value.security_id,
        magic_enum::enum_name(value.mass_action_scope),
        value.market_segment_id,
        magic_enum::enum_name(value.mass_cancel_request_type),
        magic_enum::enum_name(value.side),
        magic_enum::enum_name(value.ord_type),
        magic_enum::enum_name(value.time_in_force),
        magic_enum::enum_name(value.liquidity_flag),
        value.orig_order_user);
  }
};
