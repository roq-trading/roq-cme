/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include <fmt/chrono.h>
#include <fmt/format.h>

#include <chrono>
#include <string_view>

#include "roq/limits.hpp"

#include "roq/cme/ilink/utils.hpp"

#include "cme_ilink/ManualOrdIndReq.h"

namespace roq {
namespace cme {
namespace ilink {

struct SecurityDefinitionRequest final {
  std::span<std::byte const> encode(std::span<std::byte> const &buffer) const;

  uint64_t party_details_list_req_id = {};
  uint64_t security_req_id = {};
  cme_ilink::ManualOrdIndReq::Value manual_order_indicator = {};
  uint32_t seq_num = {};
  std::string_view sender_id;
  std::chrono::nanoseconds sending_time_epoch = {};
  std::string_view security_sub_type;
  std::string_view location;
  std::chrono::seconds start_date = {};
  std::chrono::seconds end_date = {};
  uint8_t max_no_of_substitutions = {};
  int32_t source_repo_id = {};
  uint8_t broken_date_term_type = {};
  // NoLegs
  // NoBrokenDates
};

}  // namespace ilink
}  // namespace cme
}  // namespace roq

template <>
struct fmt::formatter<roq::cme::ilink::SecurityDefinitionRequest> {
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(roq::cme::ilink::SecurityDefinitionRequest const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(party_details_list_req_id={}, )"
        R"(security_req_id={}, )"
        R"(manual_order_indicator={}, )"
        R"(seq_num={}, )"
        R"(sender_id="{}", )"
        R"(time_epoch={}, )"
        R"(security_sub_type="{}", )"
        R"(location="{}", )"
        R"(start_date={}, )"
        R"(end_date={}, )"
        R"(max_no_of_substitutions={}, )"
        R"(source_repo_id={}, )"
        R"(broken_date_term_type={})"
        R"(}})"sv,
        value.party_details_list_req_id,
        value.security_req_id,
        value.manual_order_indicator,
        value.seq_num,
        value.sender_id,
        value.sending_time_epoch,
        value.security_sub_type,
        value.location,
        value.start_date,
        value.end_date,
        value.max_no_of_substitutions,
        value.source_repo_id,
        value.broken_date_term_type);
  }
};
