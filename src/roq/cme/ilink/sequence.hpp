/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <fmt/chrono.h>
#include <fmt/format.h>

#include <chrono>
#include <span>
#include <string_view>

#include "roq/debug/hex/message.hpp"

#include <cme_ilink/FTI.h>
#include <cme_ilink/KeepAliveLapsed.h>

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
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(roq::cme::ilink::Sequence const &value, Context &context) const {
    using namespace std::literals;
    using namespace fmt::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(uuid={}, )"
        R"(next_seq_no={}, )"
        R"(fault_tolerance_indicator={}, )"
        R"(keep_alive_interval_lapsed="{}")"
        R"(}})"_cf,
        value.uuid,
        value.next_seq_no,
        value.fault_tolerance_indicator,
        value.keep_alive_interval_lapsed);
  }
};
