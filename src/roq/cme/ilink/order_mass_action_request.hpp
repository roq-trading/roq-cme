/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <fmt/chrono.h>
#include <fmt/format.h>

#include <magic_enum/magic_enum_format.hpp>

#include <chrono>
#include <string_view>

#include "roq/limits.hpp"

#include "roq/cme/ilink/utils.hpp"

#include "cme/sbe/ilink/BooleanNULL.h"
#include "cme/sbe/ilink/ManualOrdIndReq.h"
#include "cme/sbe/ilink/MassActionScope.h"
#include "cme/sbe/ilink/MassCancelTIF.h"
#include "cme/sbe/ilink/MassCxlReqTyp.h"
#include "cme/sbe/ilink/SideNULL.h"

namespace roq {
namespace cme {
namespace ilink {

struct OrderMassActionRequest final {
  std::span<std::byte const> encode(std::span<std::byte> const &buffer) const;

  uint64_t party_details_list_req_id = {};
  uint64_t order_request_id = {};
  ::cme::sbe::ilink::ManualOrdIndReq::Value manual_order_indicator = {};
  uint32_t seq_num = {};
  std::string_view sender_id;
  std::chrono::nanoseconds sending_time_epoch = {};
  std::string_view security_group;
  std::string_view location;
  int32_t security_id = {};
  ::cme::sbe::ilink::MassActionScope::Value mass_action_scope = {};
  uint8_t market_segment_id = {};
  ::cme::sbe::ilink::MassCxlReqTyp::Value mass_cancel_request_type = {};
  ::cme::sbe::ilink::SideNULL::Value side = ::cme::sbe::ilink::SideNULL::NULL_VALUE;
  // note! null is not actually supported
  ::cme::sbe::ilink::MassActionOrdTyp::Value ord_type = ::cme::sbe::ilink::MassActionOrdTyp::NULL_VALUE;
  ::cme::sbe::ilink::MassCancelTIF::Value time_in_force = ::cme::sbe::ilink::MassCancelTIF::NULL_VALUE;
  ::cme::sbe::ilink::BooleanNULL::Value liquidity_flag = ::cme::sbe::ilink::BooleanNULL::NULL_VALUE;
  std::string_view orig_order_user;
};

}  // namespace ilink
}  // namespace cme
}  // namespace roq

template <>
struct fmt::formatter<roq::cme::ilink::OrderMassActionRequest> {
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(roq::cme::ilink::OrderMassActionRequest const &value, format_context &context) const {
    using namespace std::literals;
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
        R"(}})"sv,
        value.party_details_list_req_id,
        value.order_request_id,
        value.manual_order_indicator,
        value.seq_num,
        value.sender_id,
        value.sending_time_epoch,
        value.security_group,
        value.location,
        value.security_id,
        value.mass_action_scope,
        value.market_segment_id,
        value.mass_cancel_request_type,
        value.side,
        value.ord_type,
        value.time_in_force,
        value.liquidity_flag,
        value.orig_order_user);
  }
};
