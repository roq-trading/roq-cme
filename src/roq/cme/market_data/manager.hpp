/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include "roq/api.hpp"

#include "roq/cache/market_by_order.hpp"
#include "roq/cache/market_by_price.hpp"

#include "roq/cme/mdp/connection_type.hpp"

#include "roq/cme/market_data/channel.hpp"
#include "roq/cme/market_data/shared.hpp"

#include "roq/cme/market_data/incremental.hpp"
#include "roq/cme/market_data/instrument_definition.hpp"
#include "roq/cme/market_data/mbofd_market_recovery.hpp"
#include "roq/cme/market_data/mbp_market_recovery.hpp"

namespace roq {
namespace cme {
namespace market_data {

struct Manager final : public Shared::Handler,
                       public InstrumentDefinition::Handler,
                       public MBPMarketRecovery::Handler,
                       public MBOFDMarketRecovery::Handler,
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
    // - helpers
    virtual bool discard_symbol(std::string_view const &symbol) = 0;
    virtual cache::MarketByPrice &get_market_by_price(
        std::string_view const &exchange, std::string_view const &symbol) = 0;
    virtual cache::MarketByOrder &get_market_by_order(
        std::string_view const &exchange, std::string_view const &symbol) = 0;
  };

  struct Config final {
    bool cache_all_reference_data = {};
  };

  Manager(Handler &, Config const &);

  void dispatch(mdp::ConnectionType, std::span<std::byte const> const &payload, TraceInfo const &, uint16_t stream_id);

 protected:
  void operator()(Trace<StreamStatus> const &event) override { handler_(event); }
  void operator()(Trace<ExternalLatency> const &event) override { handler_(event); }
  void operator()(Trace<ReferenceData> const &event, bool is_last) override;
  void operator()(Trace<MarketStatus> const &event, bool is_last) override { handler_(event, is_last); }
  void operator()(Trace<TopOfBook> const &event, bool is_last) override { handler_(event, is_last); }
  void operator()(Trace<MarketByPriceUpdate> const &event, bool is_last) override;
  void operator()(Trace<MarketByOrderUpdate> const &event, bool is_last) override;
  void operator()(Trace<TradeSummary> const &event, bool is_last) override { handler_(event, is_last); }
  void operator()(Trace<StatisticsUpdate> const &event, bool is_last) override { handler_(event, is_last); }

  // Shared::Handler
  bool discard_symbol(std::string_view const &symbol) override { return handler_.discard_symbol(symbol); }
  cache::MarketByPrice &get_market_by_price(std::string_view const &exchange, std::string_view const &symbol) override {
    return handler_.get_market_by_price(exchange, symbol);
  }
  cache::MarketByOrder &get_market_by_order(std::string_view const &exchange, std::string_view const &symbol) override {
    return handler_.get_market_by_order(exchange, symbol);
  }

 private:
  Handler &handler_;
  Config const config_;
  Shared shared_;
  Channel channel_;
  InstrumentDefinition instrument_definition_;
  MBPMarketRecovery mbp_market_recovery_;
  MBOFDMarketRecovery mbofd_market_recovery_;
  Incremental incremental_;
  std::vector<MBPUpdate> bids_, asks_;
  std::vector<MBOUpdate> orders_;
};

}  // namespace market_data
}  // namespace cme
}  // namespace roq
