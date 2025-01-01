/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include <fmt/core.h>

#include <magic_enum/magic_enum_format.hpp>

#include <cme_ilink/BusinessReject521.h>
#include <cme_ilink/PartyDetailsDefinitionRequestAck519.h>

#include "roq/core/sbe/iterator.hpp"

#include "roq/cme/ilink/utils.hpp"

namespace roq {
namespace cme {
namespace ilink {}  // namespace ilink
}  // namespace cme
}  // namespace roq

// messages

template <>
struct fmt::formatter<cme_ilink::PartyDetailsDefinitionRequestAck519::NoPartyDetails> {
  using value_type = cme_ilink::PartyDetailsDefinitionRequestAck519::NoPartyDetails;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format([[maybe_unused]] value_type &value, format_context &context) const {
    using namespace std::literals;
    using namespace roq::cme::ilink;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(}})"sv);
  }
};

template <>
struct fmt::formatter<cme_ilink::PartyDetailsDefinitionRequestAck519::NoTrdRegPublications> {
  using value_type = cme_ilink::PartyDetailsDefinitionRequestAck519::NoTrdRegPublications;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format([[maybe_unused]] value_type &value, format_context &context) const {
    using namespace std::literals;
    using namespace roq::cme::ilink;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(}})"sv);
  }
};

template <>
struct fmt::formatter<cme_ilink::PartyDetailsDefinitionRequestAck519> {
  using value_type = cme_ilink::PartyDetailsDefinitionRequestAck519;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type &value, format_context &context) const {
    using namespace std::literals;
    using namespace roq::cme::ilink;
    value.sbeRewind();  // note!
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(seq_num={}, )"
        R"(uuid={}, )"
        R"(memo="{}", )"
        R"(avg_px_group_id="{}", )"
        R"(party_details_list_req_id={}, )"
        R"(sending_time_epoch={}, )"
        R"(self_match_prevention_id={}, )"
        R"(party_details_request_status={}, )"
        R"(cust_order_capacity={}, )"
        R"(clearing_account_type={}, )"
        R"(self_match_prevention_instruction={}, )"
        R"(avg_px_indicator={}, )"
        R"(clearing_trade_price_type={}, )"
        R"(cmta_giveup_cd={}, )"
        R"(cust_order_handling_inst={}, )"
        R"(no_party_updates={}, )"
        R"(list_update_action={}, )"
        R"(party_detail_definition_status={}, )"
        R"(executor={}, )"
        R"(idm_short_code={}, )"
        R"(poss_retrans_flag={}, )"
        R"(split_msg={}, )"
        // R"(no_party_details=[{}], )"
        // R"(no_trd_reg_publications=[{}])"
        R"(}})"sv,
        value.seqNum(),
        value.uUID(),
        value.memo(),
        value.avgPxGroupID(),
        value.partyDetailsListReqID(),
        value.sendingTimeEpoch(),
        value.selfMatchPreventionID(),
        value.partyDetailRequestStatus(),
        value.custOrderCapacity(),
        value.clearingAccountType(),
        value.selfMatchPreventionInstruction(),
        value.avgPxIndicator(),
        value.clearingTradePriceType(),
        value.cmtaGiveupCD(),
        value.custOrderHandlingInst(),
        value.noPartyUpdates(),
        value.listUpdateAction(),
        value.partyDetailDefinitionStatus(),
        value.executor(),
        value.iDMShortCode(),
        value.possRetransFlag(),
        value.splitMsg());
    // roq::cme::ilink::Group{value.noPartyDetails()},
    // roq::cme::ilink::Group{value.noTrdRegPublications()});
  }
};

template <>
struct fmt::formatter<cme_ilink::BusinessReject521> {
  using value_type = cme_ilink::BusinessReject521;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type &value, format_context &context) const {
    using namespace std::literals;
    using namespace roq::cme::ilink;
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
