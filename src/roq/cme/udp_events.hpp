/* Copyright (c) 2017-2022, Hans Erik Thrane */

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
#include "roq/cme/shared.hpp"

#include "roq/cme/sbe/parser.hpp"

namespace roq {
namespace cme {

class UDPEvents final : public io::net::udp::Receiver::Handler, public sbe::Parser::Handler {
 public:
  struct Handler {
    virtual void operator()(Trace<StreamStatus const> const &) = 0;
    virtual void operator()(Trace<TopOfBook const> const &, bool is_last) = 0;
    virtual void operator()(Trace<MarketByPriceUpdate const> const &, bool is_last, bool refresh) = 0;
    virtual void operator()(Trace<TradeSummary const> const &, bool is_last) = 0;
  };

  UDPEvents(Handler &, io::Context &, uint16_t stream_id, Shared &);

  UDPEvents(UDPEvents const &) = delete;
  UDPEvents(UDPEvents &&) = delete;

  void operator()(Event<Start> const &);
  void operator()(Event<Stop> const &);
  void operator()(Event<Timer> const &);

  void operator()(metrics::Writer &);

 protected:
  void operator()(io::net::udp::Receiver::Read const &) override;
  void operator()(io::net::udp::Receiver::Error const &) override;

 protected:
  // sbe::Parser::Handler
  // - MDInstrumentDefinition
  void operator()(Trace<cme_mdp::MDInstrumentDefinitionFuture54> const &, sbe::Frame const &) override;
  void operator()(Trace<cme_mdp::MDInstrumentDefinitionOption55> const &, sbe::Frame const &) override;
  void operator()(Trace<cme_mdp::MDInstrumentDefinitionSpread56> const &, sbe::Frame const &) override;
  void operator()(Trace<cme_mdp::MDInstrumentDefinitionFixedIncome57> const &, sbe::Frame const &) override;
  void operator()(Trace<cme_mdp::MDInstrumentDefinitionRepo58> const &, sbe::Frame const &) override;
  void operator()(Trace<cme_mdp::MDInstrumentDefinitionFX63> const &, sbe::Frame const &) override;
  // - MDIncrementalRefresh
  void operator()(Trace<cme_mdp::MDIncrementalRefreshBook46> const &, sbe::Frame const &) override;

 protected:
  void publish_stream_status(TraceInfo const &, ConnectionStatus connection_status);

  // utils
  template <typename T, typename U>
  static void emplace_back(const T &item, double multiplier, U &bids, U &asks);

  Aggregator &get_aggregator(uint16_t channel_id);

 private:
  Handler &handler_;
  // config
  const uint16_t stream_id_;
  const std::string name_;
  bool const publish_top_of_book_;
  bool const publish_market_by_price_;
  bool const publish_trade_summary_;
  Mask<SupportType> const supports_;
  // receiver
  std::unique_ptr<io::net::udp::Receiver> receiver_;
  io::Buffer receive_buffer_;
  // metrics
  struct {
    core::metrics::Counter disconnect;
  } counter_;
  struct {
    core::metrics::Profile parse;
  } profile_;
  // cache
  Shared &shared_;
  ConnectionStatus connection_status_ = {};
  absl::flat_hash_map<uint32_t, uint32_t> last_ticker_;
  absl::flat_hash_map<uint32_t, uint32_t> last_trades_;
  absl::node_hash_map<uint16_t, Aggregator> aggregator_;
  // state
  std::chrono::nanoseconds last_update_time_ = {};
};

}  // namespace cme
}  // namespace roq
