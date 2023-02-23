/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <fmt/format.h>

#include <cme_ilink3/BusinessReject521.h>

#include "roq/core/sbe/iterator.hpp"

#include "roq/cme/ilink3/utils.hpp"

namespace roq {
namespace cme {
namespace ilink3 {}  // namespace ilink3
}  // namespace cme
}  // namespace roq

// messages

template <>
struct fmt::formatter<cme_ilink3::BusinessReject521> {
  using value_type = cme_ilink3::BusinessReject521;
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type &value, Context &context) const {
    using namespace std::literals;
    using namespace roq::cme::ilink3;
    value.sbeRewind();  // note!
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(seq_num={}, )"
        R"(uuid={}, )"
        R"(text="{}", )"
        R"(sender_id={}, )"
        R"(party_details_list_req_id={}, )"
        R"(sending_time_epoch={}, )"
        R"(business_reject_ref_id={}, )"
        R"(location={}, )"
        R"(ref_seq_num={}, )"
        R"(ref_tag_id={}, )"
        R"(business_reject_reason={}, )"
        R"(ref_msg_type={}, )"
        R"(poss_retrans_flag={}, )"
        R"(manual_order_indicator={}, )"
        R"(split_msg={})"
        R"(}})"sv,
        value.seqNum(),
        value.uUID(),
        value.text(),
        value.senderID(),
        value.partyDetailsListReqID(),
        value.sendingTimeEpoch(),
        value.businessRejectRefID(),
        value.location(),
        value.refSeqNum(),
        value.refTagID(),
        value.businessRejectReason(),
        value.refMsgType(),
        value.possRetransFlag(),
        value.manualOrderIndicator(),
        value.splitMsg());
  }
};
