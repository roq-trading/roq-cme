/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include <span>

#include "roq/api.hpp"

#include "roq/server/md/dispatcher.hpp"

#include "roq/cme/mdp/config.hpp"
#include "roq/cme/mdp/connection_type.hpp"

#include "roq/cme/market_data/channel.hpp"
#include "roq/cme/market_data/options.hpp"
#include "roq/cme/market_data/shared.hpp"

#include "roq/cme/market_data/incremental.hpp"
#include "roq/cme/market_data/instrument_definition.hpp"
#include "roq/cme/market_data/mbofd_market_recovery.hpp"
#include "roq/cme/market_data/mbp_market_recovery.hpp"

namespace roq {
namespace cme {
namespace market_data {

struct Manager final {
  Manager(
      server::md::Dispatcher &,
      Options const &,
      std::span<uint16_t const> const &channel_ids,
      mdp::Config const &,
      uint16_t &stream_id);

  std::string_view const get_name(uint16_t channel_id, mdp::ConnectionType, Priority) const;

  void operator()(Event<Start> const &);
  void operator()(Event<Stop> const &);
  void operator()(Event<Timer> const &);

  void dispatch(
      uint16_t channel_id, mdp::ConnectionType, Priority, std::span<std::byte const> const &payload, TraceInfo const &);

 protected:
  template <typename T>
  void dispatch(Event<T> const &);

 private:
  server::md::Dispatcher &dispatcher_;
  Options const options_;
  Shared shared_;
  struct Channel2 final {
    Channel2(Shared &, mdp::Config const &, uint16_t channel_id, uint16_t &stream_id);

    Channel channel;
    InstrumentDefinition instrument_definition_1;
    InstrumentDefinition instrument_definition_2;
    MBPMarketRecovery mbp_market_recovery_1;
    MBPMarketRecovery mbp_market_recovery_2;
    MBOFDMarketRecovery mbofd_market_recovery_1;
    MBOFDMarketRecovery mbofd_market_recovery_2;
    Incremental incremental_1;
    Incremental incremental_2;
  };
  utils::unordered_map<uint16_t, Channel2> channels_;
};

}  // namespace market_data
}  // namespace cme
}  // namespace roq
