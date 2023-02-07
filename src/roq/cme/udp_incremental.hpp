/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <absl/container/flat_hash_map.h>
#include <absl/container/node_hash_map.h>

#include "roq/core/metrics/counter.hpp"
#include "roq/core/metrics/profile.hpp"

#include "roq/io/context.hpp"

#include "roq/io/net/udp/receiver.hpp"

#include "roq/server.hpp"

#include "roq/cme/aggregator.hpp"
#include "roq/cme/channel.hpp"
#include "roq/cme/shared.hpp"

#include "roq/cme/sbe/parser.hpp"

namespace roq {
namespace cme {

struct UDPIncremental final : public io::net::udp::Receiver::Handler, public sbe::Parser::Handler {
  struct Handler {
    virtual void operator()(Trace<StreamStatus> const &) = 0;
    virtual void operator()(Trace<ExternalLatency> const &) = 0;
    virtual void operator()(Trace<MarketStatus> const &, bool is_last) = 0;
    virtual void operator()(Trace<TopOfBook> const &, bool is_last) = 0;
    virtual void operator()(Trace<MarketByPriceUpdate> const &, bool is_last) = 0;
    virtual void operator()(Trace<MarketByOrderUpdate> const &, bool is_last) = 0;
    virtual void operator()(Trace<TradeSummary> const &, bool is_last) = 0;
    virtual void operator()(Trace<StatisticsUpdate> const &, bool is_last) = 0;
  };

  UDPIncremental(Handler &, io::Context &, uint16_t stream_id, Shared &, Channel &, Priority);

  UDPIncremental(UDPIncremental const &) = delete;
  UDPIncremental(UDPIncremental &&) = delete;

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
  void operator()(Trace<cme_mdp::AdminHeartbeat12> const &, sbe::Frame const &) override;
  void operator()(Trace<cme_mdp::ChannelReset4> const &, sbe::Frame const &) override;
  // - security status
  void operator()(Trace<cme_mdp::SecurityStatus30> const &, sbe::Frame const &) override;
  // - instrument definitions
  void operator()(Trace<cme_mdp::MDInstrumentDefinitionFuture54> const &, sbe::Frame const &) override;
  void operator()(Trace<cme_mdp::MDInstrumentDefinitionOption55> const &, sbe::Frame const &) override;
  void operator()(Trace<cme_mdp::MDInstrumentDefinitionSpread56> const &, sbe::Frame const &) override;
  void operator()(Trace<cme_mdp::MDInstrumentDefinitionFixedIncome57> const &, sbe::Frame const &) override;
  void operator()(Trace<cme_mdp::MDInstrumentDefinitionRepo58> const &, sbe::Frame const &) override;
  void operator()(Trace<cme_mdp::MDInstrumentDefinitionFX63> const &, sbe::Frame const &) override;
  // - market by price
  void operator()(Trace<cme_mdp::SnapshotFullRefresh52> const &, sbe::Frame const &) override;
  void operator()(Trace<cme_mdp::SnapshotFullRefreshLongQty69> const &, sbe::Frame const &) override;
  void operator()(Trace<cme_mdp::MDIncrementalRefreshBook46> const &, sbe::Frame const &) override;
  void operator()(Trace<cme_mdp::MDIncrementalRefreshBookLongQty64> const &, sbe::Frame const &) override;
  // - market by order
  void operator()(Trace<cme_mdp::SnapshotFullRefreshOrderBook53> const &, sbe::Frame const &) override;
  void operator()(Trace<cme_mdp::MDIncrementalRefreshOrderBook47> const &, sbe::Frame const &) override;
  // - trade summary
  void operator()(Trace<cme_mdp::MDIncrementalRefreshTradeSummary48> const &, sbe::Frame const &) override;
  void operator()(Trace<cme_mdp::MDIncrementalRefreshTradeSummaryLongQty65> const &, sbe::Frame const &) override;
  // - statistics
  void operator()(Trace<cme_mdp::MDIncrementalRefreshDailyStatistics49> const &, sbe::Frame const &) override;
  void operator()(Trace<cme_mdp::MDIncrementalRefreshSessionStatistics51> const &, sbe::Frame const &) override;
  void operator()(Trace<cme_mdp::MDIncrementalRefreshSessionStatisticsLongQty67> const &, sbe::Frame const &) override;
  void operator()(Trace<cme_mdp::MDIncrementalRefreshVolume37> const &, sbe::Frame const &) override;
  void operator()(Trace<cme_mdp::MDIncrementalRefreshVolumeLongQty66> const &, sbe::Frame const &) override;
  // - misc
  void operator()(Trace<cme_mdp::MDIncrementalRefreshLimitsBanding50> const &, sbe::Frame const &) override;
  void operator()(Trace<cme_mdp::QuoteRequest39> const &, sbe::Frame const &) override;

 protected:
  void dispatch_market_by_price(
      auto &trace_info,
      auto security_id,
      auto &security,
      auto exchange_sequence,
      auto exchange_time_utc,
      auto &bids,
      auto &asks);

  void dispatch_market_by_order(
      auto &trace_info,
      auto security_id,
      auto &security,
      auto exchange_sequence,
      auto exchange_time_utc,
      auto &bids,
      auto &asks);

  template <typename T>
  void dispatch_trade_summary(Trace<T> const &);

  template <typename T, typename Callback>
  void dispatch_statistics(Trace<T> const &, Callback);

 protected:
  void publish_stream_status(TraceInfo const &, ConnectionStatus connection_status);

 private:
  Handler &handler_;
  // config
  const uint16_t stream_id_;
  const std::string name_;
  // receiver
  std::unique_ptr<io::net::udp::Receiver> receiver_;
  // metrics
  struct {
    core::metrics::Counter disconnect, sequence_reset;
  } counter_;
  struct {
    core::metrics::Profile parse,                                             //
        admin_heartbeat, channel_reset,                                       //
        security_status,                                                      //
        snapshot_full_refresh, snapshot_full_refresh_long_qty,                //
        md_incremental_refresh_book, md_incremental_refresh_book_long_qty,    //
        snapshot_full_refresh_order_book, md_incremental_refresh_order_book,  //
        md_incremental_refresh_trade_summary,
        md_incremental_refresh_trade_summary_long_qty,  //
        md_incremental_refresh_daily_statistics,        //
        md_incremental_refresh_session_statistics,
        md_incremental_refresh_session_statistics_long_qty,                     //
        md_incremental_refresh_volume, md_incremental_refresh_volume_long_qty,  //
        quote_request;
  } profile_;
  // cache
  Shared &shared_;
  Channel &channel_;
  ConnectionStatus connection_status_ = {};
  // state
  std::chrono::nanoseconds last_update_time_ = {};
  // DEBUG
  bool log_this_message_ = false;
  std::vector<std::tuple<int32_t, Side, double>> entries_46_;
};

}  // namespace cme
}  // namespace roq
