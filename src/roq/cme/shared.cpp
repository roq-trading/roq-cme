/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/cme/shared.hpp"

#include "roq/logging.hpp"

#include "roq/cme/flags/common.hpp"
#include "roq/cme/flags/fix.hpp"
#include "roq/cme/flags/multicast.hpp"

using namespace std::literals;

namespace roq {
namespace cme {

Shared::Shared(server::Dispatcher &dispatcher)
    : multicast_config_(flags::Multicast::multicast_config_file()), fills(server::Flags::cache_fills_max_depth()),
      bids(server::Flags::cache_mbp_max_depth()), asks(server::Flags::cache_mbp_max_depth()),
      final_bids(server::Flags::cache_mbp_max_depth()), final_asks(server::Flags::cache_mbp_max_depth()),
      trades(server::Flags::cache_trades_max_depth()), statistics(magic_enum::enum_count<StatisticsType>()),
      dispatcher_(dispatcher), rate_limiter(flags::Common::request_limit(), flags::Common::request_limit_interval()),
      symbols(flags::FIX::fix_market_data_max_subscriptions_per_stream()) {
}

std::pair<std::string, uint16_t> Shared::get_multicast_config(multicast::Type type, Priority priority) const {
  std::pair<std::string, uint16_t> result;
  auto channel_id = flags::Multicast::multicast_channel_id();
  if (multicast_config_.find(channel_id, type, priority, [&](auto &connection) {
        result = {connection.multicast_address, connection.port};
      })) {
  } else {
    throw RuntimeError(
        R"(Unable to find multicast configuration using channel_id="{}", type={}, priority={})"sv,
        channel_id,
        magic_enum::enum_name(type),
        priority);
  }
  return result;
}

std::string_view Shared::next_request_id() {
  auto request_id = ++request_id_;
  stack_buffer_.clear();
  fmt::format_to(std::back_inserter(stack_buffer_), "roq-{}"sv, request_id);
  return std::string_view{std::data(stack_buffer_), std::size(stack_buffer_)};
}

}  // namespace cme
}  // namespace roq
