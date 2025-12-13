/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <fmt/chrono.h>
#include <fmt/format.h>

#include <magic_enum/magic_enum_format.hpp>

#include <chrono>
#include <span>
#include <string_view>

#include "roq/limits.hpp"

#include "roq/cme/ilink/utils.hpp"

#include "cme_ilink/ManualOrdIndReq.h"
#include "cme_ilink/QuoteCxlTyp.h"

namespace roq {
namespace cme {
namespace ilink {

struct QuoteCancel final {
  std::span<std::byte const> encode(std::span<std::byte> const &buffer) const;

  uint64_t party_details_list_req_id = {};
  std::chrono::nanoseconds sending_time_epoch = {};
  cme_ilink::ManualOrdIndReq::Value manual_order_indicator = {};
  uint32_t seq_num = {};
  std::string_view sender_id;
  std::string_view location;
  uint32_t quote_id = {};
  cme_ilink::QuoteCxlTyp::Value quote_cancel_type = {};
  bool liquidity_flag = {};
  std::string_view orig_order_user;
  bool quote_entry_open = {};
  // NoQuoteEntries
  struct QuoteEntry final {
    std::string_view security_group;
    int32_t security_id = {};
  };
  std::span<QuoteEntry> no_quote_entries;
  // NoQuoteSets
  struct QuoteSet final {
    uint32_t bid_size = {};
    uint32_t offer_size = {};
    uint16_t quote_set_id = {};
  };
  std::span<QuoteSet> no_quote_sets;
};

}  // namespace ilink
}  // namespace cme
}  // namespace roq

template <>
struct fmt::formatter<roq::cme::ilink::QuoteCancel::QuoteEntry> {
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(roq::cme::ilink::QuoteCancel::QuoteEntry const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(security_group="{}", )"
        R"(security_id={})"
        R"(}})"sv,
        value.security_group,
        value.security_id);
  }
};

template <>
struct fmt::formatter<roq::cme::ilink::QuoteCancel::QuoteSet> {
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(roq::cme::ilink::QuoteCancel::QuoteSet const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(bid_size={}, )"
        R"(offer_size={}, )"
        R"(quote_set_id={})"
        R"(}})"sv,
        value.bid_size,
        value.offer_size,
        value.quote_set_id);
  }
};

template <>
struct fmt::formatter<roq::cme::ilink::QuoteCancel> {
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(roq::cme::ilink::QuoteCancel const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(party_details_list_req_id={}, )"
        R"(sending_time_epoch={}, )"
        R"(manual_order_indicator={}, )"
        R"(seq_num={}, )"
        R"(sender_id={}, )"
        R"(location="{}", )"
        R"(quote_id={}, )"
        R"(quote_cancel_type={}, )"
        R"(liquidity_flag={}, )"
        R"(orig_order_user="{}", )"
        R"(quote_entry_open={}, )"
        R"(no_quote_entries=[{}], )"
        R"(no_quote_sets=[{}])"
        R"(}})"sv,
        value.party_details_list_req_id,
        value.sending_time_epoch,
        value.manual_order_indicator,
        value.seq_num,
        value.sender_id,
        value.location,
        value.quote_id,
        value.quote_cancel_type,
        value.liquidity_flag,
        value.orig_order_user,
        value.quote_entry_open,
        fmt::join(value.no_quote_entries, ", "sv),
        fmt::join(value.no_quote_sets, ", "sv));
  }
};
