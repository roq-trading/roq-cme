/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/cme/shared.hpp"

#include "roq/logging.hpp"

#include "roq/cme/flags/common.hpp"
#include "roq/cme/flags/fix.hpp"
#include "roq/cme/flags/multicast.hpp"

using namespace std::literals;

namespace roq {
namespace cme {

namespace {
auto get_multicast() {
  // XXX maybe check more flags?
  if (std::empty(flags::Multicast::local_interface()))
    return false;
  log::info("Using multicast"sv);
  return true;
}
}  // namespace

Shared::Shared(server::Dispatcher &dispatcher)
    : fills(server::Flags::cache_fills_max_depth()), bids(server::Flags::cache_mbp_max_depth()),
      asks(server::Flags::cache_mbp_max_depth()), final_bids(server::Flags::cache_mbp_max_depth()),
      final_asks(server::Flags::cache_mbp_max_depth()), trades(server::Flags::cache_trades_max_depth()),
      statistics(magic_enum::enum_count<StatisticsType>()), dispatcher_(dispatcher), multicast_(get_multicast()),
      rate_limiter(flags::Common::request_limit(), flags::Common::request_limit_interval()),
      symbols(flags::FIX::fix_market_data_max_subscriptions_per_stream()) {
}

std::string_view Shared::next_request_id() {
  auto request_id = ++request_id_;
  stack_buffer_.clear();
  fmt::format_to(std::back_inserter(stack_buffer_), "roq-{}"sv, request_id);
  return std::string_view{std::data(stack_buffer_), std::size(stack_buffer_)};
}

}  // namespace cme
}  // namespace roq
