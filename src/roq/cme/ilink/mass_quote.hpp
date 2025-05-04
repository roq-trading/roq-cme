/* Copyright (c) 2017-2025, Hans Erik Thrane */

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
#include "cme_ilink/ShortSaleType.h"

namespace roq {
namespace cme {
namespace ilink {

struct MassQuote final {
  std::span<std::byte const> encode(std::span<std::byte> const &buffer) const;

  uint64_t party_details_list_req_id = {};
  std::chrono::nanoseconds sending_time_epoch = {};
  cme_ilink::ManualOrdIndReq::Value manual_order_indicator = {};
  uint32_t seq_num = {};
  std::string_view sender_id;
  uint64_t quote_req_id = {};
  std::string_view location;
  uint32_t quote_id = {};
  uint8_t tot_no_quote_entries = {};
  bool mmp_protection_reset = {};
  bool liquidity_flag = {};
  cme_ilink::ShortSaleType::Value short_sale_type = {};
  std::string_view reserved;
  std::string_view reserved_1;
  bool quote_entry_open = {};
  // NoQuoteEntries
  struct QuoteEntry final {
    double bid_px = NaN;
    double offer_px = NaN;
    uint32_t quote_entry_id = {};
    int32_t security_id = {};
    uint32_t bid_size = {};
    uint32_t offer_size = {};
    int32_t underlying_security_id = {};
    uint16_t quote_set_id = {};
  };
  std::span<QuoteEntry> no_quote_entries;
};

}  // namespace ilink
}  // namespace cme
}  // namespace roq

template <>
struct fmt::formatter<roq::cme::ilink::MassQuote::QuoteEntry> {
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(roq::cme::ilink::MassQuote::QuoteEntry const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(bid_px={}, )"
        R"(offer_px={}, )"
        R"(quote_entry_id={}, )"
        R"(security_id={}, )"
        R"(bid_size={}, )"
        R"(offer_size={}, )"
        R"(underyling_security_id={}, )"
        R"(quote_set_id={})"
        R"(}})"sv,
        value.bid_px,
        value.offer_px,
        value.quote_entry_id,
        value.security_id,
        value.bid_size,
        value.offer_size,
        value.underlying_security_id,
        value.quote_set_id);
  }
};

template <>
struct fmt::formatter<roq::cme::ilink::MassQuote> {
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(roq::cme::ilink::MassQuote const &value, format_context &context) const {
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
        R"(tot_no_quote_entries={}, )"
        R"(mmp_protection_reset={}, )"
        R"(liquidity_flag={}, )"
        R"(short_sale_type={}, )"
        R"(reserved="{}", )"
        R"(reserved_1="{}", )"
        R"(quote_entry_open={}, )"
        R"(no_quote_entries=[{}])"
        R"(}})"sv,
        value.party_details_list_req_id,
        value.sending_time_epoch,
        value.manual_order_indicator,
        value.seq_num,
        value.sender_id,
        value.location,
        value.quote_id,
        value.tot_no_quote_entries,
        value.mmp_protection_reset,
        value.liquidity_flag,
        value.short_sale_type,
        value.reserved,
        value.reserved_1,
        value.quote_entry_open,
        fmt::join(value.no_quote_entries, ", "sv));
  }
};
