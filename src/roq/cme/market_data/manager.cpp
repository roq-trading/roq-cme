/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/cme/market_data/manager.hpp"

#include <fmt/core.h>

#include <magic_enum/magic_enum_format.hpp>

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
R create_channels(auto &shared, auto &channel_ids, auto &config, auto &stream_id) {
  using result_type = std::remove_cvref<R>::type;
  result_type result;
  for (auto &channel_id : channel_ids)
    result.try_emplace(channel_id, shared, config, channel_id, stream_id);
  return result;
}
}  // namespace

// === IMPLEMENTATION ===

Manager::Manager(
    server::md::Dispatcher &dispatcher,
    Options const &options,
    SecurityDefinitions &security_definitions,
    std::span<uint16_t const> const &channel_ids,
    mdp::Config const &config,
    uint16_t &stream_id)
    : dispatcher_{dispatcher}, options_{options}, shared_{dispatcher, options, security_definitions},
      channels_{create_channels<decltype(channels_)>(shared_, channel_ids, config, stream_id)}, secdef_config_file_{options.secdef_config_file} {
}

std::string_view const Manager::get_name(uint16_t channel_id, mdp::ConnectionType connection_type, Priority priority) const {
  auto iter = channels_.find(channel_id);
  if (iter != std::end(channels_)) {
    auto &channel = (*iter).second;
    switch (connection_type) {
      using enum mdp::ConnectionType;
      case HISTORICAL_REPLAY:
        log::fatal("Unexpected"sv);
        break;
      case INSTRUMENT_DEFINITION:
        if (priority == Priority::PRIMARY) {
          return channel.instrument_definition_1.name;
        } else {
          return channel.instrument_definition_2.name;
        }
        break;
      case MBP_MARKET_RECOVERY:
        if (priority == Priority::PRIMARY) {
          return channel.mbp_market_recovery_1.name;
        } else {
          return channel.mbp_market_recovery_2.name;
        }
        break;
      case MBOFD_MARKET_RECOVERY:
        if (priority == Priority::PRIMARY) {
          return channel.mbofd_market_recovery_1.name;
        } else {
          return channel.mbofd_market_recovery_2.name;
        }
        break;
      case INCREMENTAL:
        if (priority == Priority::PRIMARY) {
          return channel.incremental_1.name;
        } else {
          return channel.incremental_2.name;
        }
        break;
    }
  }
  log::fatal("Unexpected: channel_id={}, connection_type={}, priority={}"sv, channel_id, connection_type, priority);
}

void Manager::operator()(Event<Start> const &event) {
  dispatch(event);
  shared_.read_secdef(secdef_config_file_, options_.pcap_first_timestamp);  // note!
}

void Manager::operator()(Event<Stop> const &event) {
  dispatch(event);
}

void Manager::operator()(Event<Timer> const &event) {
  dispatch(event);
}

void Manager::operator()(metrics::Writer &) {
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
    uint16_t channel_id, mdp::ConnectionType connection_type, Priority priority, std::span<std::byte const> const &payload, TraceInfo const &trace_info) {
  auto iter = channels_.find(channel_id);
  if (iter == std::end(channels_)) [[unlikely]]
    return;  // note!
  auto &channel = (*iter).second;
  switch (connection_type) {
    using enum mdp::ConnectionType;
    case HISTORICAL_REPLAY:
      log::fatal("Unexpected"sv);
      break;
    case INSTRUMENT_DEFINITION:
      if (priority == Priority::PRIMARY) {
        channel.instrument_definition_1.dispatch(payload, trace_info);
      } else {
        channel.instrument_definition_2.dispatch(payload, trace_info);
      }
      break;
    case MBP_MARKET_RECOVERY:
      if (priority == Priority::PRIMARY) {
        channel.mbp_market_recovery_1.dispatch(payload, trace_info);
      } else {
        channel.mbp_market_recovery_2.dispatch(payload, trace_info);
      }
      break;
    case MBOFD_MARKET_RECOVERY:
      if (priority == Priority::PRIMARY) {
        channel.mbofd_market_recovery_1.dispatch(payload, trace_info);
      } else {
        channel.mbofd_market_recovery_2.dispatch(payload, trace_info);
      }
      break;
    case INCREMENTAL:
      if (priority == Priority::PRIMARY) {
        channel.incremental_1.dispatch(payload, trace_info);
      } else {
        channel.incremental_2.dispatch(payload, trace_info);
      }
      break;
  }
}

Manager::Channel2::Channel2(Shared &shared, mdp::Config const &config, uint16_t channel_id, uint16_t &stream_id)
    : channel{channel_id, BUFFER_SIZE, BUFFER_DEPTH}, instrument_definition_1{shared, ++stream_id, config, channel_id, Priority::PRIMARY},
      instrument_definition_2{shared, ++stream_id, config, channel_id, Priority::SECONDARY},
      mbp_market_recovery_1{shared, channel, ++stream_id, config, channel_id, Priority::PRIMARY},
      mbp_market_recovery_2{shared, channel, ++stream_id, config, channel_id, Priority::SECONDARY},
      mbofd_market_recovery_1{shared, channel, ++stream_id, config, channel_id, Priority::PRIMARY},
      mbofd_market_recovery_2{shared, channel, ++stream_id, config, channel_id, Priority::SECONDARY},
      incremental_1{shared, incremental_cache, channel, ++stream_id, config, channel_id, Priority::PRIMARY},
      incremental_2{shared, incremental_cache, channel, ++stream_id, config, channel_id, Priority::SECONDARY} {
}

}  // namespace market_data
}  // namespace cme
}  // namespace roq
