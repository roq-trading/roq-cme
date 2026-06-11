/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <fmt/chrono.h>
#include <fmt/format.h>

#include <magic_enum/magic_enum_format.hpp>

#include <chrono>
#include <string_view>

#include "roq/limits.hpp"

#include "roq/cme/protocol/ilink/utils.hpp"

#include "cme/sbe/ilink/ManualOrdIndReq.h"
#include "cme/sbe/ilink/MassStatusOrdTyp.h"
#include "cme/sbe/ilink/MassStatusReqTyp.h"
#include "cme/sbe/ilink/MassStatusTIF.h"

namespace roq {
namespace cme {
namespace protocol {
namespace ilink {

struct OrderMassStatusRequest final {
  std::span<std::byte const> encode(std::span<std::byte> const &buffer) const;

  uint64_t party_details_list_req_id = {};
  uint64_t mass_status_req_id = {};
  ::cme::sbe::ilink::ManualOrdIndReq::Value manual_order_indicator = {};
  uint32_t seq_num = {};
  std::string_view sender_id;
  std::chrono::nanoseconds sending_time_epoch = {};
  std::string_view security_group;
  std::string_view location;
  int32_t security_id = {};
  ::cme::sbe::ilink::MassStatusReqTyp::Value mass_status_req_type = {};
  ::cme::sbe::ilink::MassStatusOrdTyp::Value ord_status_req_type = {};
  ::cme::sbe::ilink::MassStatusTIF::Value time_in_force = {};
  std::uint8_t market_segment_id = {};
};

}  // namespace ilink
}  // namespace protocol
}  // namespace cme
}  // namespace roq

template <>
struct fmt::formatter<roq::cme::protocol::ilink::OrderMassStatusRequest> {
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(roq::cme::protocol::ilink::OrderMassStatusRequest const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(party_details_list_req_id={}, )"
        R"(mass_status_req_id={}, )"
        R"(manual_order_indicator={}, )"
        R"(seq_num={}, )"
        R"(sender_id={}, )"
        R"(time_epoch={}, )"
        R"(security_group="{}", )"
        R"(location="{}", )"
        R"(security_id={}, )"
        R"(mass_status_req_type={}, )"
        R"(ord_status_req_type={}, )"
        R"(time_in_force={}, )"
        R"(market_segment_id="{}")"
        R"(}})"sv,
        value.party_details_list_req_id,
        value.mass_status_req_id,
        value.manual_order_indicator,
        value.seq_num,
        value.sender_id,
        value.sending_time_epoch,
        value.security_group,
        value.location,
        value.security_id,
        value.mass_status_req_type,
        value.ord_status_req_type,
        value.time_in_force,
        value.market_segment_id);
  }
};
