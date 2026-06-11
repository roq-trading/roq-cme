/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/cme/protocol/ilink/mass_quote.hpp"

#include <cme/sbe/ilink/MassQuote517.h>

using namespace std::literals;

namespace roq {
namespace cme {
namespace protocol {
namespace ilink {

namespace {
void set_price(::cme::sbe::ilink::PRICENULL9 &result, double value) {
  using value_type = std::remove_cvref_t<decltype(result)>;
  if (std::isnan(value)) {
    result.mantissa(value_type::mantissaNullValue());
  } else {
    auto mantissa = static_cast<int64_t>(value * std::pow(10.0, -static_cast<double>(value_type::exponent())) + 0.5);
    result.mantissa(mantissa);
  }
}
}  // namespace

namespace {
using header_type = ::cme::sbe::ilink::MessageHeader;
using value_type = ::cme::sbe::ilink::MassQuote517;
}  // namespace

std::span<std::byte const> MassQuote::encode(std::span<std::byte> const &buffer) const {
  value_type value;
  auto &result = value.wrapAndApplyHeader(reinterpret_cast<char *>(std::data(buffer)), 0, std::size(buffer));
  result.partyDetailsListReqID(party_details_list_req_id);
  result.sendingTimeEpoch(sending_time_epoch.count());
  result.manualOrderIndicator(manual_order_indicator);
  result.seqNum(seq_num);
  result.putSenderID(sender_id);
  result.quoteReqID(quote_req_id);
  result.putLocation(location);
  result.quoteID(quote_id);
  result.totNoQuoteEntries(tot_no_quote_entries);
  result.mMProtectionReset(mmp_protection_reset ? ::cme::sbe::ilink::BooleanFlag::True : ::cme::sbe::ilink::BooleanFlag::False);
  result.liquidityFlag(liquidity_flag ? ::cme::sbe::ilink::BooleanNULL::True : ::cme::sbe::ilink::BooleanNULL::False);
  result.shortSaleType(short_sale_type);
  // reserved
  // reserved_1
  result.quoteEntryOpen(quote_entry_open ? ::cme::sbe::ilink::BooleanNULL::True : ::cme::sbe::ilink::BooleanNULL::False);
  auto &quote_entries = result.noQuoteEntriesCount(std::size(no_quote_entries));
  for (auto &item : no_quote_entries) {
    quote_entries.next();
    set_price(quote_entries.bidPx(), item.bid_px);
    set_price(quote_entries.offerPx(), item.offer_px);
    quote_entries.quoteEntryID(item.quote_entry_id);
    quote_entries.securityID(item.security_id);
    quote_entries.bidSize(item.bid_size);
    quote_entries.offerSize(item.offer_size);
    quote_entries.underlyingSecurityID(item.underlying_security_id);
    quote_entries.quoteSetID(item.quote_set_id);
  }
  auto length = header_type::encodedLength() + value_type::computeLength(std::size(no_quote_entries));
  return {std::data(buffer), length};
}

}  // namespace ilink
}  // namespace protocol
}  // namespace cme
}  // namespace roq
