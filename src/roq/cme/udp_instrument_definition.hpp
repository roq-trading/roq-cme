/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <absl/container/flat_hash_map.h>
#include <absl/container/node_hash_map.h>

#include "roq/core/metrics/counter.hpp"
#include "roq/core/metrics/profile.hpp"

#include "roq/io/buffer.hpp"
#include "roq/io/context.hpp"
#include "roq/io/net/udp/receiver.hpp"

#include "roq/server.hpp"

#include "roq/cme/aggregator.hpp"
#include "roq/cme/channel.hpp"
#include "roq/cme/shared.hpp"

#include "roq/cme/sbe/parser.hpp"

namespace roq {
namespace cme {

struct UDPInstrumentDefinition final : public io::net::udp::Receiver::Handler, public sbe::Parser::Handler {
  struct Handler {
    virtual void operator()(Trace<StreamStatus> const &) = 0;
    virtual void operator()(Trace<ExternalLatency> const &) = 0;
    virtual void operator()(Trace<ReferenceData> const &, bool is_last) = 0;
    virtual void operator()(Trace<MarketStatus> const &, bool is_last) = 0;
  };

  UDPInstrumentDefinition(Handler &, io::Context &, uint16_t stream_id, Shared &, Channel &);

  UDPInstrumentDefinition(UDPInstrumentDefinition const &) = delete;
  UDPInstrumentDefinition(UDPInstrumentDefinition &&) = delete;

  void operator()(Event<Start> const &);
  void operator()(Event<Stop> const &);
  void operator()(Event<Timer> const &);

  void operator()(metrics::Writer &);

 protected:
  void operator()(io::net::udp::Receiver::Read const &) override;
  void operator()(io::net::udp::Receiver::Error const &) override;

 protected:
  // sbe::Parser::Handler
  void operator()(sbe::Frame const &) override;
  // - admin
  void operator()(Trace<cme_mdp3::AdminHeartbeat12> const &, sbe::Frame const &) override;
  void operator()(Trace<cme_mdp3::ChannelReset4> const &, sbe::Frame const &) override;
  // - security status
  void operator()(Trace<cme_mdp3::SecurityStatus30> const &, sbe::Frame const &) override;
  // - instrument definitions
  void operator()(Trace<cme_mdp3::MDInstrumentDefinitionFuture54> const &, sbe::Frame const &) override;
  void operator()(Trace<cme_mdp3::MDInstrumentDefinitionOption55> const &, sbe::Frame const &) override;
  void operator()(Trace<cme_mdp3::MDInstrumentDefinitionSpread56> const &, sbe::Frame const &) override;
  void operator()(Trace<cme_mdp3::MDInstrumentDefinitionFixedIncome57> const &, sbe::Frame const &) override;
  void operator()(Trace<cme_mdp3::MDInstrumentDefinitionRepo58> const &, sbe::Frame const &) override;
  void operator()(Trace<cme_mdp3::MDInstrumentDefinitionFX63> const &, sbe::Frame const &) override;
  // - market by price
  void operator()(Trace<cme_mdp3::SnapshotFullRefresh52> const &, sbe::Frame const &) override;
  void operator()(Trace<cme_mdp3::SnapshotFullRefreshLongQty69> const &, sbe::Frame const &) override;
  void operator()(Trace<cme_mdp3::MDIncrementalRefreshBook46> const &, sbe::Frame const &) override;
  void operator()(Trace<cme_mdp3::MDIncrementalRefreshBookLongQty64> const &, sbe::Frame const &) override;
  // - market by order
  void operator()(Trace<cme_mdp3::SnapshotFullRefreshOrderBook53> const &, sbe::Frame const &) override;
  void operator()(Trace<cme_mdp3::MDIncrementalRefreshOrderBook47> const &, sbe::Frame const &) override;
  // - trade summary
  void operator()(Trace<cme_mdp3::MDIncrementalRefreshTradeSummary48> const &, sbe::Frame const &) override;
  void operator()(Trace<cme_mdp3::MDIncrementalRefreshTradeSummaryLongQty65> const &, sbe::Frame const &) override;
  // - statistics
  void operator()(Trace<cme_mdp3::MDIncrementalRefreshDailyStatistics49> const &, sbe::Frame const &) override;
  void operator()(Trace<cme_mdp3::MDIncrementalRefreshSessionStatistics51> const &, sbe::Frame const &) override;
  void operator()(Trace<cme_mdp3::MDIncrementalRefreshSessionStatisticsLongQty67> const &, sbe::Frame const &) override;
  void operator()(Trace<cme_mdp3::MDIncrementalRefreshVolume37> const &, sbe::Frame const &) override;
  void operator()(Trace<cme_mdp3::MDIncrementalRefreshVolumeLongQty66> const &, sbe::Frame const &) override;
  // - misc
  void operator()(Trace<cme_mdp3::MDIncrementalRefreshLimitsBanding50> const &, sbe::Frame const &) override;
  void operator()(Trace<cme_mdp3::QuoteRequest39> const &, sbe::Frame const &) override;

 protected:
  void publish_stream_status(TraceInfo const &, ConnectionStatus connection_status);

  // utils
  template <typename T, typename U>
  static void emplace_back(T const &item, double multiplier, U &bids, U &asks);

 private:
  Handler &handler_;
  // config
  const uint16_t stream_id_;
  const std::string name_;
  // receiver
  std::unique_ptr<io::net::udp::Receiver> receiver_;
  io::Buffer receive_buffer_;
  // metrics
  struct {
    core::metrics::Counter disconnect;
  } counter_;
  struct {
    core::metrics::Profile parse, admin_heartbeat, channel_reset,  //
        md_instrument_definition_future,                           //
        md_instrument_definition_option,                           //
        md_instrument_definition_spread,                           //
        md_instrument_definition_fixed_income,                     //
        md_instrument_definition_repo,                             //
        md_instrument_definition_fx;
  } profile_;
  // cache
  Shared &shared_;
  Channel &channel_;
  ConnectionStatus connection_status_ = {};
  // state
  std::chrono::nanoseconds last_update_time_ = {};
};

}  // namespace cme
}  // namespace roq
