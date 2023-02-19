/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <cme_mdp3/QuoteRequest39.h>

#include "roq/core/sbe/iterator.hpp"

#include "roq/cme/sbe/utils.hpp"

namespace roq {
namespace cme {
namespace sbe {

/*
template <>
inline size_t compute_length(cme_mdp3::QuoteRequest39 &value) {
  // NoRelatedSym
  auto no_md_entries_length = value.noRelatedSym().count();
  value.noRelatedSym().forEach([](auto &e) { e.skip(); });
  return value.computeLength(no_md_entries_length);
}
*/

}  // namespace sbe
}  // namespace cme
}  // namespace roq

// QuoteRequest39

template <>
struct fmt::formatter<cme_mdp3::QuoteRequest39::NoRelatedSym> {
  using value_type = cme_mdp3::QuoteRequest39::NoRelatedSym;
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type const &value, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(symbol={}, )"
        R"(security_id={}, )"
        R"(order_qty={}, )"
        R"(order_type={}, )"
        R"(side={})"
        R"(}})"sv,
        value.symbol(),
        value.securityID(),
        value.orderQty(),
        value.quoteType(),
        value.side());
  }
};

template <>
struct fmt::formatter<cme_mdp3::QuoteRequest39> {
  using value_type = cme_mdp3::QuoteRequest39;
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(value_type &value, Context &context) const {
    using namespace std::literals;
    value.sbeRewind();  // note!
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(version={}, )"
        R"(transact_time={}, )"
        R"(quote_req_id={}, )"
        R"(match_event_indicator={}, )"
        R"(no_related_sym=[{}])"
        R"(}})"sv,
        value.actingVersion(),
        value.transactTime(),
        value.quoteReqID(),
        value.matchEventIndicator(),
        roq::cme::sbe::Group{value.noRelatedSym()});
  }
};
