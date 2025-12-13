/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <fmt/format.h>

#include <cme_ilink/EstablishmentAck504.h>
#include <cme_ilink/EstablishmentReject505.h>
#include <cme_ilink/NegotiationReject502.h>
#include <cme_ilink/NegotiationResponse501.h>
#include <cme_ilink/NotApplied513.h>
#include <cme_ilink/Retransmission509.h>
#include <cme_ilink/RetransmitReject510.h>
#include <cme_ilink/Sequence506.h>
#include <cme_ilink/Terminate507.h>

#include "roq/core/sbe/iterator.hpp"

#include "roq/cme/ilink/utils.hpp"

namespace roq {
namespace cme {
namespace ilink {}  // namespace ilink
}  // namespace cme
}  // namespace roq

// messages

template <>
struct fmt::formatter<cme_ilink::NegotiationResponse501> {
  using value_type = cme_ilink::NegotiationResponse501;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type &value, format_context &context) const {
    using namespace std::literals;
    using namespace roq::cme::ilink;
    value.sbeRewind();  // note!
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(server_flow={}, )"
        R"(uuid={}, )"
        R"(request_timestamp={}, )"
        R"(secret_key_secure_id_expiration={}, )"
        R"(fault_tolerance_indicator={}, )"
        R"(split_msg={}, )"
        R"(previous_seq_no={}, )"
        R"(previous_uuid={}, )"
        R"(credentials={})"
        R"(}})"sv,
        value.serverFlow(),
        value.uUID(),
        value.requestTimestamp(),
        value.secretKeySecureIDExpiration(),
        value.faultToleranceIndicator(),
        value.splitMsg(),
        value.previousSeqNo(),
        value.previousUUID(),
        value.credentials());
  }
};

template <>
struct fmt::formatter<cme_ilink::NegotiationReject502> {
  using value_type = cme_ilink::NegotiationReject502;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type &value, format_context &context) const {
    using namespace std::literals;
    using namespace roq::cme::ilink;
    value.sbeRewind();  // note!
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(reason={}, )"
        R"(uuid={}, )"
        R"(request_timestamp={}, )"
        R"(error_codes={}, )"
        R"(fault_tolerance_indicator={}, )"
        R"(split_msg={})"
        R"(}})"sv,
        value.reason(),
        value.uUID(),
        value.requestTimestamp(),
        value.errorCodes(),
        value.faultToleranceIndicator(),
        value.splitMsg());
  }
};

template <>
struct fmt::formatter<cme_ilink::EstablishmentAck504> {
  using value_type = cme_ilink::EstablishmentAck504;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type &value, format_context &context) const {
    using namespace std::literals;
    using namespace roq::cme::ilink;
    value.sbeRewind();  // note!
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(uuid={}, )"
        R"(request_timestamp={}, )"
        R"(next_seq_no={}, )"
        R"(previous_seq_no={}, )"
        R"(previous_uuid={}, )"
        R"(keep_alive_interval={}, )"
        R"(secret_key_secure_id_expiration={}, )"
        R"(fault_tolerance_indicator={}, )"
        R"(split_msg={})"
        R"(}})"sv,
        value.uUID(),
        value.requestTimestamp(),
        value.nextSeqNo(),
        value.previousSeqNo(),
        value.previousUUID(),
        value.keepAliveInterval(),
        value.secretKeySecureIDExpiration(),
        value.faultToleranceIndicator(),
        value.splitMsg());
  }
};

template <>
struct fmt::formatter<cme_ilink::EstablishmentReject505> {
  using value_type = cme_ilink::EstablishmentReject505;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type &value, format_context &context) const {
    using namespace std::literals;
    using namespace roq::cme::ilink;
    value.sbeRewind();  // note!
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(reason={}, )"
        R"(uuid={}, )"
        R"(request_timestamp={}, )"
        R"(next_seq_no={}, )"
        R"(error_codes={}, )"
        R"(fault_tolerance_indicator={}, )"
        R"(split_msg={})"
        R"(}})"sv,
        value.reason(),
        value.uUID(),
        value.requestTimestamp(),
        value.nextSeqNo(),
        value.errorCodes(),
        value.faultToleranceIndicator(),
        value.splitMsg());
  }
};

template <>
struct fmt::formatter<cme_ilink::Sequence506> {
  using value_type = cme_ilink::Sequence506;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type &value, format_context &context) const {
    using namespace std::literals;
    using namespace roq::cme::ilink;
    value.sbeRewind();  // note!
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(uuid={}, )"
        R"(next_seq_no={}, )"
        R"(fault_tolerance_indicator={}, )"
        R"(keep_alive_interval_elapsed={})"
        R"(}})"sv,
        value.uUID(),
        value.nextSeqNo(),
        value.faultToleranceIndicator(),
        value.keepAliveIntervalLapsed());
  }
};

template <>
struct fmt::formatter<cme_ilink::Terminate507> {
  using value_type = cme_ilink::Terminate507;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type &value, format_context &context) const {
    using namespace std::literals;
    using namespace roq::cme::ilink;
    value.sbeRewind();  // note!
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(reason="{}", )"
        R"(uuid={}, )"
        R"(request_timestamp={}, )"
        R"(error_codes={}, )"
        R"(split_msg={})"
        R"(}})"sv,
        value.reason(),
        value.uUID(),
        value.requestTimestamp(),
        value.errorCodes(),
        value.splitMsg());
  }
};

template <>
struct fmt::formatter<cme_ilink::Retransmission509> {
  using value_type = cme_ilink::Retransmission509;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type &value, format_context &context) const {
    using namespace std::literals;
    using namespace roq::cme::ilink;
    value.sbeRewind();  // note!
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(uuid={}, )"
        R"(last_uuid={}, )"
        R"(request_timestamp={}, )"
        R"(from_seq_no={}, )"
        R"(msg_count={}, )"
        R"(split_msg={})"
        R"(}})"sv,
        value.uUID(),
        value.lastUUID(),
        value.requestTimestamp(),
        value.fromSeqNo(),
        value.msgCount(),
        value.splitMsg());
  }
};

template <>
struct fmt::formatter<cme_ilink::RetransmitReject510> {
  using value_type = cme_ilink::RetransmitReject510;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type &value, format_context &context) const {
    using namespace std::literals;
    using namespace roq::cme::ilink;
    value.sbeRewind();  // note!
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(reason={}, )"
        R"(uuid={}, )"
        R"(last_uuid={}, )"
        R"(request_timestamp={}, )"
        R"(error_codes={}, )"
        R"(split_msg={})"
        R"(}})"sv,
        value.reason(),
        value.uUID(),
        value.lastUUID(),
        value.requestTimestamp(),
        value.errorCodes(),
        value.splitMsg());
  }
};

template <>
struct fmt::formatter<cme_ilink::NotApplied513> {
  using value_type = cme_ilink::NotApplied513;
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(value_type &value, format_context &context) const {
    using namespace std::literals;
    using namespace roq::cme::ilink;
    value.sbeRewind();  // note!
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(uuid={}, )"
        R"(from_seq_no={}, )"
        R"(msg_count={}, )"
        R"(split_msg={})"
        R"(}})"sv,
        value.uUID(),
        value.fromSeqNo(),
        value.msgCount(),
        value.splitMsg());
  }
};
