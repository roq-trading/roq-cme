/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <fmt/format.h>

#include <cme_ilink/SecurityDefinitionResponse561.h>

#include "roq/core/sbe/iterator.hpp"

#include "roq/cme/ilink/utils.hpp"

namespace roq {
namespace cme {
namespace ilink {}  // namespace ilink
}  // namespace cme
}  // namespace roq

// messages

template <>
struct fmt::formatter<cme_ilink::SecurityDefinitionResponse561> {
  using value_type = cme_ilink::SecurityDefinitionResponse561;
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type &value, Context &context) const {
    using namespace std::literals;
    using namespace roq::cme::ilink;
    value.sbeRewind();  // note!
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(seq_num={}, )"
        R"(uuid={}, )"
        R"(text="{}", )"
        R"(financial_instrument_full_name={}, )"
        R"(sender_id={}, )"
        R"(symbol={}, )"
        R"(party_details_list_req_id={}, )"
        R"(security_req_id={}, )"
        R"(security_response_id={}, )"
        R"(sending_time_epoch={}, )"
        R"(security_group={}, )"
        R"(security_type={}, )"
        R"(location={}, )"
        R"(security_id={}, )"
        R"(currency={}, )"
        R"(security_id_source={}, )"
        R"(maturity_month_year={}, )"
        R"(delay_duration={}, )"
        R"(start_date={}, )"
        R"(end_date={}, )"
        R"(max_no_of_substitutions={}, )"
        R"(source_repo_id={}, )"
        R"(termination_type={}, )"
        R"(security_response_type={}, )"
        R"(user_defined_instrument={}, )"
        R"(expiration_cycle={}, )"
        R"(manual_order_indicator={}, )"
        R"(split_msg={}, )"
        R"(auto_quote_request={}, )"
        R"(poss_retrans_flag={}, )"
        R"(no_legs=[...])"
        R"(}})"sv,
        value.seqNum(),
        value.uUID(),
        value.text(),
        value.financialInstrumentFullName(),
        value.senderID(),
        value.symbol(),
        value.partyDetailsListReqID(),
        value.securityReqID(),
        value.securityResponseID(),
        value.sendingTimeEpoch(),
        value.securityGroup(),
        value.securityType(),
        value.location(),
        value.securityID(),
        value.currency(),
        value.securityIDSource(),
        value.maturityMonthYear(),
        value.delayDuration(),
        value.startDate(),
        value.endDate(),
        value.maxNoOfSubstitutions(),
        value.sourceRepoID(),
        value.terminationType(),
        value.securityResponseType(),
        value.userDefinedInstrument(),
        value.expirationCycle(),
        value.manualOrderIndicator(),
        value.splitMsg(),
        value.autoQuoteRequest(),
        value.possRetransFlag());
    // + noLegs()
  }
};
