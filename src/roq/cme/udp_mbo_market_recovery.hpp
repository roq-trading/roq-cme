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

#include "roq/cme/mdp/parser.hpp"

namespace roq {
namespace cme {

struct UDPMBOMarketRecovery final : public io::net::udp::Receiver::Handler, public mdp::Parser::Handler {
  struct Handler {
    virtual void operator()(Trace<StreamStatus> const &) = 0;
    virtual void operator()(Trace<ExternalLatency> const &) = 0;
    virtual void operator()(Trace<MarketByOrderUpdate> const &, bool is_last) = 0;
  };

  UDPMBOMarketRecovery(Handler &, io::Context &, uint16_t stream_id, Shared &, Channel &);

  UDPMBOMarketRecovery(UDPMBOMarketRecovery const &) = delete;
  UDPMBOMarketRecovery(UDPMBOMarketRecovery &&) = delete;

  void operator()(Event<Start> const &);
  void operator()(Event<Stop> const &);
  void operator()(Event<Timer> const &);

  void operator()(metrics::Writer &);

 protected:
  void operator()(io::net::udp::Receiver::Read const &) override;
  void operator()(io::net::udp::Receiver::Error const &) override;

 protected:
  // mdp::Parser::Handler
  void operator()(mdp::Frame const &) override;
  // - admin
  void operator()(Trace<cme_mdp::AdminHeartbeat12> const &, mdp::Frame const &) override;
  void operator()(Trace<cme_mdp::ChannelReset4> const &, mdp::Frame const &) override;
  // - security status
  void operator()(Trace<cme_mdp::SecurityStatus30> const &, mdp::Frame const &) override;
  // - instrument definitions
  void operator()(Trace<cme_mdp::MDInstrumentDefinitionFuture54> const &, mdp::Frame const &) override;
  void operator()(Trace<cme_mdp::MDInstrumentDefinitionOption55> const &, mdp::Frame const &) override;
  void operator()(Trace<cme_mdp::MDInstrumentDefinitionSpread56> const &, mdp::Frame const &) override;
  void operator()(Trace<cme_mdp::MDInstrumentDefinitionFixedIncome57> const &, mdp::Frame const &) override;
  void operator()(Trace<cme_mdp::MDInstrumentDefinitionRepo58> const &, mdp::Frame const &) override;
  void operator()(Trace<cme_mdp::MDInstrumentDefinitionFX63> const &, mdp::Frame const &) override;
  // - market by price
  void operator()(Trace<cme_mdp::SnapshotFullRefresh52> const &, mdp::Frame const &) override;
  void operator()(Trace<cme_mdp::SnapshotFullRefreshLongQty69> const &, mdp::Frame const &) override;
  void operator()(Trace<cme_mdp::MDIncrementalRefreshBook46> const &, mdp::Frame const &) override;
  void operator()(Trace<cme_mdp::MDIncrementalRefreshBookLongQty64> const &, mdp::Frame const &) override;
  // - market by order
  void operator()(Trace<cme_mdp::SnapshotFullRefreshOrderBook53> const &, mdp::Frame const &) override;
  void operator()(Trace<cme_mdp::MDIncrementalRefreshOrderBook47> const &, mdp::Frame const &) override;
  // - trade summary
  void operator()(Trace<cme_mdp::MDIncrementalRefreshTradeSummary48> const &, mdp::Frame const &) override;
  void operator()(Trace<cme_mdp::MDIncrementalRefreshTradeSummaryLongQty65> const &, mdp::Frame const &) override;
  // - statistics
  void operator()(Trace<cme_mdp::MDIncrementalRefreshDailyStatistics49> const &, mdp::Frame const &) override;
  void operator()(Trace<cme_mdp::MDIncrementalRefreshSessionStatistics51> const &, mdp::Frame const &) override;
  void operator()(Trace<cme_mdp::MDIncrementalRefreshSessionStatisticsLongQty67> const &, mdp::Frame const &) override;
  void operator()(Trace<cme_mdp::MDIncrementalRefreshVolume37> const &, mdp::Frame const &) override;
  void operator()(Trace<cme_mdp::MDIncrementalRefreshVolumeLongQty66> const &, mdp::Frame const &) override;
  // - misc
  void operator()(Trace<cme_mdp::MDIncrementalRefreshLimitsBanding50> const &, mdp::Frame const &) override;
  void operator()(Trace<cme_mdp::QuoteRequest39> const &, mdp::Frame const &) override;

 protected:
  void publish_stream_status(TraceInfo const &, ConnectionStatus connection_status);

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
    core::metrics::Profile parse,        //
        admin_heartbeat, channel_reset,  //
        snapshot_full_refresh_order_book;
    ;
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
