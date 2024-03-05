/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/cme/market_data/manager.hpp"

#include "roq/logging.hpp"

using namespace std::literals;

namespace roq {
namespace cme {
namespace market_data {

// === CONSTANTS ===

namespace {
auto const BUFFER_SIZE = 2048uz;  // >=1440 MTU
auto const BUFFER_DEPTH = 10uz;
}  // namespace

// === HELPERS ===

namespace {
template <typename R>
R create_channels(auto &handler, auto &shared, auto &channel_ids, auto &config, auto &stream_id) {
  using result_type = std::remove_cvref<R>::type;
  result_type result;
  for (auto &channel_id : channel_ids)
    result.try_emplace(channel_id, handler, shared, config, channel_id, stream_id);
  return result;
}
}  // namespace

// === IMPLEMENTATION ===

Manager::Manager(
    Handler &handler,
    Config const &config,
    std::span<uint16_t const> const &channel_ids,
    mdp::Config const &config_2,
    uint16_t &stream_id)
    : handler_{handler}, config_{config}, shared_{*this, config},
      channels_{create_channels<decltype(channels_)>(*this, shared_, channel_ids, config_2, stream_id)} {
}

void Manager::operator()(Event<Start> const &event) {
  dispatch(event);
}

void Manager::operator()(Event<Stop> const &event) {
  dispatch(event);
}

void Manager::operator()(Event<Timer> const &event) {
  dispatch(event);
}

template <typename T>
void Manager::dispatch(Event<T> const &event) {
  for (auto &[channel_id, item] : channels_) {
    item.instrument_definition_1(event);
    item.instrument_definition_2(event);
    item.mbp_market_recovery_1(event);
    item.mbp_market_recovery_2(event);
    item.mbofd_market_recovery_1(event);
    item.mbofd_market_recovery_2(event);
    item.incremental_1(event);
    item.incremental_2(event);
  }
}

void Manager::dispatch(
    uint16_t channel_id,
    mdp::ConnectionType connection_type,
    Priority priority,
    std::span<std::byte const> const &payload,
    TraceInfo const &trace_info) {
  auto iter = channels_.find(channel_id);
  if (iter != std::end(channels_)) {
    auto &channel = (*iter).second;
    switch (connection_type) {
      using enum mdp::ConnectionType;
      case HISTORICAL_REPLAY:
        log::fatal("Unexpected"sv);
        break;
      case INSTRUMENT_DEFINITION:
        if (priority == Priority::PRIMARY)
          channel.instrument_definition_1.dispatch(payload, trace_info);
        else
          channel.instrument_definition_2.dispatch(payload, trace_info);
        break;
      case MBP_MARKET_RECOVERY:
        if (priority == Priority::PRIMARY)
          channel.mbp_market_recovery_1.dispatch(payload, trace_info);
        else
          channel.mbp_market_recovery_2.dispatch(payload, trace_info);
        break;
      case MBOFD_MARKET_RECOVERY:
        if (priority == Priority::PRIMARY)
          channel.mbofd_market_recovery_1.dispatch(payload, trace_info);
        else
          channel.mbofd_market_recovery_2.dispatch(payload, trace_info);
        break;
      case INCREMENTAL:
        if (priority == Priority::PRIMARY)
          channel.incremental_1.dispatch(payload, trace_info);
        else
          channel.incremental_2.dispatch(payload, trace_info);
        break;
    }
  }
}

void Manager::operator()(Trace<ReferenceData> const &event, bool is_last) {
  if (event.value.discard && !config_.cache_all_reference_data)
    return;
  handler_(event, is_last);
}

void Manager::operator()(Trace<MarketByPriceUpdate> const &event, bool is_last) {
  auto &[trace_info, market_by_price_update] = event;
  auto &market_by_price = handler_.get_market_by_price(market_by_price_update.exchange, market_by_price_update.symbol);
  market_by_price(market_by_price_update, bids_, asks_, [&](auto &market_by_price_update_2) {
    Trace event_2{trace_info, market_by_price_update_2};
    handler_(event_2, is_last);
  });
}

void Manager::operator()(Trace<MarketByOrderUpdate> const &event, bool is_last) {
  auto &[trace_info, market_by_order_update] = event;
  auto &market_by_order = handler_.get_market_by_order(market_by_order_update.exchange, market_by_order_update.symbol);
  market_by_order(market_by_order_update, orders_, [&](auto &market_by_order_update_2) {
    Trace event_2{trace_info, market_by_order_update_2};
    handler_(event_2, is_last);
  });
}

Manager::Channel2::Channel2(
    Manager &handler, Shared &shared, mdp::Config const &config, uint16_t channel_id, uint16_t &stream_id)
    : channel{"344", BUFFER_SIZE, BUFFER_DEPTH},
      instrument_definition_1{handler, shared, ++stream_id, config, channel_id, Priority::PRIMARY},
      instrument_definition_2{handler, shared, ++stream_id, config, channel_id, Priority::SECONDARY},
      mbp_market_recovery_1{handler, shared, channel, ++stream_id, config, channel_id, Priority::PRIMARY},
      mbp_market_recovery_2{handler, shared, channel, ++stream_id, config, channel_id, Priority::SECONDARY},
      mbofd_market_recovery_1{handler, shared, channel, ++stream_id, config, channel_id, Priority::PRIMARY},
      mbofd_market_recovery_2{handler, shared, channel, ++stream_id, config, channel_id, Priority::SECONDARY},
      incremental_1{handler, shared, channel, ++stream_id, config, channel_id, Priority::PRIMARY},
      incremental_2{handler, shared, channel, ++stream_id, config, channel_id, Priority::SECONDARY} {
}

}  // namespace market_data
}  // namespace cme
}  // namespace roq
