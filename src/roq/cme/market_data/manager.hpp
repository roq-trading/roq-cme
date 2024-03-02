/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include "roq/api.hpp"

#include "roq/cme/mdp/connection_type.hpp"

#include "roq/cme/market_data/channel.hpp"
#include "roq/cme/market_data/shared.hpp"

#include "roq/cme/market_data/incremental.hpp"
#include "roq/cme/market_data/instrument_definition.hpp"
#include "roq/cme/market_data/market_by_order_recovery.hpp"
#include "roq/cme/market_data/market_by_price_recovery.hpp"

namespace roq {
namespace cme {
namespace market_data {

struct Manager final : public InstrumentDefinition::Handler,
                       public MarketByPriceRecovery::Handler,
                       public MarketByOrderRecovery::Handler,
                       public Incremental::Handler {
  struct Handler {
    virtual void operator()(Trace<StreamStatus> const &) = 0;
    virtual void operator()(Trace<ExternalLatency> const &) = 0;
    virtual void operator()(Trace<ReferenceData> const &, bool is_last) = 0;
    virtual void operator()(Trace<MarketStatus> const &, bool is_last) = 0;
    virtual void operator()(Trace<TopOfBook> const &, bool is_last) = 0;
    virtual void operator()(Trace<MarketByPriceUpdate> const &, bool is_last) = 0;
    virtual void operator()(Trace<MarketByOrderUpdate> const &, bool is_last) = 0;
    virtual void operator()(Trace<TradeSummary> const &, bool is_last) = 0;
    virtual void operator()(Trace<StatisticsUpdate> const &, bool is_last) = 0;
  };

  explicit Manager(Handler &);

  void dispatch(mdp::ConnectionType, std::span<std::byte const> const &payload, TraceInfo const &, uint16_t stream_id);

 protected:
  void operator()(Trace<StreamStatus> const &event) override { handler_(event); }
  void operator()(Trace<ExternalLatency> const &event) override { handler_(event); }
  void operator()(Trace<ReferenceData> const &event, bool is_last) override { handler_(event, is_last); }
  void operator()(Trace<MarketStatus> const &event, bool is_last) override { handler_(event, is_last); }
  void operator()(Trace<TopOfBook> const &event, bool is_last) override { handler_(event, is_last); }
  void operator()(Trace<MarketByPriceUpdate> const &event, bool is_last) override { handler_(event, is_last); }
  void operator()(Trace<MarketByOrderUpdate> const &event, bool is_last) override { handler_(event, is_last); }
  void operator()(Trace<TradeSummary> const &event, bool is_last) override { handler_(event, is_last); }
  void operator()(Trace<StatisticsUpdate> const &event, bool is_last) override { handler_(event, is_last); }

 private:
  Handler &handler_;
  Shared shared_;
  Channel channel_;
  InstrumentDefinition instrument_definition_;
  MarketByPriceRecovery market_by_price_recovery_;
  MarketByOrderRecovery market_by_order_recovery_;
  Incremental incremental_;
};

}  // namespace market_data
}  // namespace cme
}  // namespace roq
