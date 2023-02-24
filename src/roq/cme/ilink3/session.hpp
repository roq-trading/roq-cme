/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <fmt/format.h>

#include <cme_ilink3/EstablishmentAck504.h>
#include <cme_ilink3/EstablishmentReject505.h>
#include <cme_ilink3/NegotiationReject502.h>
#include <cme_ilink3/NegotiationResponse501.h>
#include <cme_ilink3/NotApplied513.h>
#include <cme_ilink3/Retransmission509.h>
#include <cme_ilink3/RetransmitReject510.h>
#include <cme_ilink3/Sequence506.h>
#include <cme_ilink3/Terminate507.h>

#include "roq/core/sbe/iterator.hpp"

#include "roq/cme/ilink3/utils.hpp"

namespace roq {
namespace cme {
namespace ilink3 {}  // namespace ilink3
}  // namespace cme
}  // namespace roq

// messages

template <>
struct fmt::formatter<cme_ilink3::NegotiationResponse501> {
  using value_type = cme_ilink3::NegotiationResponse501;
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
struct fmt::formatter<cme_ilink3::NegotiationReject502> {
  using value_type = cme_ilink3::NegotiationReject502;
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
struct fmt::formatter<cme_ilink3::EstablishmentAck504> {
  using value_type = cme_ilink3::EstablishmentAck504;
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
struct fmt::formatter<cme_ilink3::EstablishmentReject505> {
  using value_type = cme_ilink3::EstablishmentReject505;
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
struct fmt::formatter<cme_ilink3::Sequence506> {
  using value_type = cme_ilink3::Sequence506;
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
struct fmt::formatter<cme_ilink3::Terminate507> {
  using value_type = cme_ilink3::Terminate507;
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
        R"(reason={}, )"
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
struct fmt::formatter<cme_ilink3::Retransmission509> {
  using value_type = cme_ilink3::Retransmission509;
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
struct fmt::formatter<cme_ilink3::RetransmitReject510> {
  using value_type = cme_ilink3::RetransmitReject510;
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
struct fmt::formatter<cme_ilink3::NotApplied513> {
  using value_type = cme_ilink3::NotApplied513;
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
