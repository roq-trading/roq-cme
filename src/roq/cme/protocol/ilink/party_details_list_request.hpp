/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <fmt/chrono.h>
#include <fmt/format.h>

#include <chrono>
#include <string_view>

#include "roq/limits.hpp"

#include "roq/cme/protocol/ilink/utils.hpp"

namespace roq {
namespace cme {
namespace protocol {
namespace ilink {

struct PartyDetailsListRequest final {
  std::span<std::byte const> encode(std::span<std::byte> const &buffer) const;

  uint64_t party_details_list_req_id = {};
  std::chrono::nanoseconds sending_time_epoch = {};
  uint32_t seq_num = {};
  // NoRequestingPartyIDs
  // NoPartyIDs
};

}  // namespace ilink
}  // namespace protocol
}  // namespace cme
}  // namespace roq

template <>
struct fmt::formatter<roq::cme::protocol::ilink::PartyDetailsListRequest> {
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(roq::cme::protocol::ilink::PartyDetailsListRequest const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(party_details_list_req_id={}, )"
        R"(sending_time_epoch={}, )"
        R"(seq_num={})"
        R"(}})"sv,
        value.party_details_list_req_id,
        value.sending_time_epoch,
        value.seq_num);
  }
};
