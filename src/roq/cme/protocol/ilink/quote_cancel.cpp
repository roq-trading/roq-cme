/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/cme/protocol/ilink/quote_cancel.hpp"

#include <cme/sbe/ilink/QuoteCancel528.h>

using namespace std::literals;

namespace roq {
namespace cme {
namespace protocol {
namespace ilink {

namespace {
using header_type = ::cme::sbe::ilink::MessageHeader;
using value_type = ::cme::sbe::ilink::QuoteCancel528;
}  // namespace

std::span<std::byte const> QuoteCancel::encode(std::span<std::byte> const &buffer) const {
  value_type value;
  auto &result = value.wrapAndApplyHeader(reinterpret_cast<char *>(std::data(buffer)), 0, std::size(buffer));
  result.partyDetailsListReqID(party_details_list_req_id);
  result.sendingTimeEpoch(sending_time_epoch.count());
  result.manualOrderIndicator(manual_order_indicator);
  result.seqNum(seq_num);
  result.putSenderID(sender_id);
  result.putLocation(location);
  result.quoteID(quote_id);
  result.quoteCancelType(quote_cancel_type);
  result.liquidityFlag(liquidity_flag ? ::cme::sbe::ilink::BooleanNULL::True : ::cme::sbe::ilink::BooleanNULL::False);
  result.putOrigOrderUser(orig_order_user);
  result.quoteEntryOpen(quote_entry_open ? ::cme::sbe::ilink::BooleanNULL::True : ::cme::sbe::ilink::BooleanNULL::False);
  auto &quote_entries = result.noQuoteEntriesCount(std::size(no_quote_entries));
  for (auto &item : no_quote_entries) {
    quote_entries.next();
    quote_entries.putSecurityGroup(item.security_group);
    quote_entries.securityID(item.security_id);
  }
  auto &quote_sets = result.noQuoteSetsCount(std::size(no_quote_sets));
  for (auto &item : no_quote_sets) {
    quote_sets.next();
    quote_sets.bidSize(item.bid_size);
    quote_sets.offerSize(item.offer_size);
    quote_sets.quoteSetID(item.quote_set_id);
  }
  auto length = header_type::encodedLength() + value_type::computeLength(std::size(no_quote_entries)) + value_type::computeLength(std::size(no_quote_sets));
  return {std::data(buffer), length};
}

}  // namespace ilink
}  // namespace protocol
}  // namespace cme
}  // namespace roq
