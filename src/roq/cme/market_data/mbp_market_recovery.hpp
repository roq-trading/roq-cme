/* Copyright (c) 2017-2026, Hans Erik Thrane */

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
  MBPMarketRecovery(Shared &, Channel &, uint16_t stream_id, mdp::Config const &, uint16_t channel_id, Priority);

  void operator()(Event<Start> const &);
  void operator()(Event<Stop> const &);
  void operator()(Event<Timer> const &);

  void dispatch(std::span<std::byte const> const &payload, TraceInfo const &);

 protected:
  // mdp::Parser::Handler

  void operator()(mdp::Frame const &) override;
  // - admin
  void operator()(Trace<::cme::sbe::mdp::AdminHeartbeat12> const &, mdp::Frame const &) override;
  void operator()(Trace<::cme::sbe::mdp::ChannelReset4> const &, mdp::Frame const &) override;
  // - security status
  void operator()(Trace<::cme::sbe::mdp::SecurityStatus30> const &, mdp::Frame const &) override;
  // - instrument definitions
  void operator()(Trace<::cme::sbe::mdp::MDInstrumentDefinitionFuture54> const &, mdp::Frame const &) override;
  void operator()(Trace<::cme::sbe::mdp::MDInstrumentDefinitionOption55> const &, mdp::Frame const &) override;
  void operator()(Trace<::cme::sbe::mdp::MDInstrumentDefinitionSpread56> const &, mdp::Frame const &) override;
  void operator()(Trace<::cme::sbe::mdp::MDInstrumentDefinitionFixedIncome57> const &, mdp::Frame const &) override;
  void operator()(Trace<::cme::sbe::mdp::MDInstrumentDefinitionRepo58> const &, mdp::Frame const &) override;
  void operator()(Trace<::cme::sbe::mdp::MDInstrumentDefinitionFX63> const &, mdp::Frame const &) override;
  // - market by price
  void operator()(Trace<::cme::sbe::mdp::SnapshotFullRefresh52> const &, mdp::Frame const &) override;
  void operator()(Trace<::cme::sbe::mdp::SnapshotFullRefreshLongQty69> const &, mdp::Frame const &) override;
  void operator()(Trace<::cme::sbe::mdp::MDIncrementalRefreshBook46> const &, mdp::Frame const &) override;
  void operator()(Trace<::cme::sbe::mdp::MDIncrementalRefreshBookLongQty64> const &, mdp::Frame const &) override;
  // - market by order
  void operator()(Trace<::cme::sbe::mdp::SnapshotFullRefreshOrderBook53> const &, mdp::Frame const &) override;
  void operator()(Trace<::cme::sbe::mdp::MDIncrementalRefreshOrderBook47> const &, mdp::Frame const &) override;
  // - trade summary
  void operator()(Trace<::cme::sbe::mdp::MDIncrementalRefreshTradeSummary48> const &, mdp::Frame const &) override;
  void operator()(Trace<::cme::sbe::mdp::MDIncrementalRefreshTradeSummaryLongQty65> const &, mdp::Frame const &) override;
  // - statistics
  void operator()(Trace<::cme::sbe::mdp::MDIncrementalRefreshDailyStatistics49> const &, mdp::Frame const &) override;
  void operator()(Trace<::cme::sbe::mdp::MDIncrementalRefreshSessionStatistics51> const &, mdp::Frame const &) override;
  void operator()(Trace<::cme::sbe::mdp::MDIncrementalRefreshSessionStatisticsLongQty67> const &, mdp::Frame const &) override;
  void operator()(Trace<::cme::sbe::mdp::MDIncrementalRefreshVolume37> const &, mdp::Frame const &) override;
  void operator()(Trace<::cme::sbe::mdp::MDIncrementalRefreshVolumeLongQty66> const &, mdp::Frame const &) override;
  // - misc
  void operator()(Trace<::cme::sbe::mdp::MDIncrementalRefreshLimitsBanding50> const &, mdp::Frame const &) override;
  void operator()(Trace<::cme::sbe::mdp::QuoteRequest39> const &, mdp::Frame const &) override;

  // helpers

  void dispatch_market_by_price(
      auto &trace_info, auto security_id, auto &security, auto exchange_sequence, auto exchange_time_utc, auto sending_time_utc, auto &bids, auto &asks);

  void publish_stream_status(TraceInfo const &, ConnectionStatus);

 public:
  Priority const priority;
  uint16_t const stream_id;
  std::string const name;

 private:
  Shared &shared_;
  Channel &channel_;
  ConnectionStatus connection_status_ = {};
  std::chrono::nanoseconds last_update_time_ = {};
};

}  // namespace market_data
}  // namespace cme
}  // namespace roq
