/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <fmt/chrono.h>
#include <fmt/format.h>

#include <chrono>
#include <string_view>

#include "roq/numbers.hpp"

#include "roq/cme/ilink/utils.hpp"

namespace roq {
namespace cme {
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
}  // namespace cme
}  // namespace roq

template <>
struct fmt::formatter<roq::cme::ilink::PartyDetailsListRequest> {
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(roq::cme::ilink::PartyDetailsListRequest const &value, Context &context) const {
    using namespace std::literals;
    using namespace fmt::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(party_details_list_req_id={}, )"
        R"(sending_time_epoch={}, )"
        R"(seq_num={})"
        R"(}})"_cf,
        value.party_details_list_req_id,
        value.sending_time_epoch,
        value.seq_num);
  }
};
