/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include <fmt/chrono.h>
#include <fmt/format.h>

#include <chrono>
#include <span>
#include <string_view>

#include <cme_ilink/FTI.h>
#include <cme_ilink/KeepAliveLapsed.h>

#include "roq/cme/ilink/utils.hpp"

namespace roq {
namespace cme {
namespace ilink {

struct Sequence final {
  std::span<std::byte const> encode(std::span<std::byte> const &buffer) const;

  uint64_t uuid = {};
  uint32_t next_seq_no = {};
  cme_ilink::FTI::Value fault_tolerance_indicator = {};
  cme_ilink::KeepAliveLapsed::Value keep_alive_interval_lapsed = {};
};

}  // namespace ilink
}  // namespace cme
}  // namespace roq

template <>
struct fmt::formatter<roq::cme::ilink::Sequence> {
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(roq::cme::ilink::Sequence const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(uuid={}, )"
        R"(next_seq_no={}, )"
        R"(fault_tolerance_indicator={}, )"
        R"(keep_alive_interval_lapsed="{}")"
        R"(}})"sv,
        value.uuid,
        value.next_seq_no,
        value.fault_tolerance_indicator,
        value.keep_alive_interval_lapsed);
  }
};
