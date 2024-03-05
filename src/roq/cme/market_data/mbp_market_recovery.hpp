/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include "roq/api.hpp"

#include "roq/cme/mdp/config.hpp"
#include "roq/cme/mdp/parser.hpp"

#include "roq/cme/market_data/channel.hpp"
#include "roq/cme/market_data/shared.hpp"

namespace roq {
namespace cme {
namespace market_data {

struct MBPMarketRecovery final : public mdp::Parser::Handler {
  struct Handler {
    virtual void operator()(Trace<StreamStatus> const &) = 0;
    virtual void operator()(Trace<ExternalLatency> const &) = 0;
    virtual void operator()(Trace<MarketByPriceUpdate> const &, bool is_last) = 0;
  };

  MBPMarketRecovery(
      Handler &, Shared &, Channel &, uint16_t stream_id, mdp::Config const &, uint16_t channel_id, Priority);

  void operator()(Event<Start> const &);
  void operator()(Event<Stop> const &);
  void operator()(Event<Timer> const &);

  void dispatch(std::span<std::byte const> const &payload, TraceInfo const &);

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

  // helpers

  void dispatch_market_by_price(
      auto &trace_info,
      auto security_id,
      auto &security,
      auto exchange_sequence,
      auto exchange_time_utc,
      auto sending_time_utc,
      auto &bids,
      auto &asks);

  void publish_stream_status(TraceInfo const &, ConnectionStatus);

 private:
  Handler &handler_;
  Shared &shared_;
  Channel &channel_;
  uint16_t const stream_id_;
  std::string const name_;
  Priority const priority_;
  ConnectionStatus connection_status_ = {};
  std::chrono::nanoseconds last_update_time_ = {};
};

}  // namespace market_data
}  // namespace cme
}  // namespace roq
