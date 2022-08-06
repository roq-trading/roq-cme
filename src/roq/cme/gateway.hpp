/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <absl/container/flat_hash_map.h>

#include <memory>
#include <string>
#include <vector>

#include "roq/server.hpp"

#include "roq/io/context.hpp"

#include "roq/cme/config.hpp"
#include "roq/cme/shared.hpp"

#include "roq/cme/udp_incremental.hpp"
#include "roq/cme/udp_instrument_definition.hpp"
#include "roq/cme/udp_mbp_market_recovery.hpp"

namespace roq {
namespace cme {

class Gateway final : public server::Handler,
                      public UDPInstrumentDefinition::Handler,
                      public UDPMBPMarketRecovery::Handler,
                      public UDPIncremental::Handler {
 public:
  Gateway(server::Dispatcher &, Config const &);

  Gateway(Gateway &&) = delete;
  Gateway(Gateway const &) = delete;

 protected:
  // server::Handler

  void operator()(Event<Start> const &) override;
  void operator()(Event<Stop> const &) override;
  void operator()(Event<Timer> const &) override;
  void operator()(Event<Connected> const &) override;
  void operator()(Event<Disconnected> const &) override;

  uint16_t operator()(Event<CreateOrder> const &, oms::Order const &, std::string_view const &request_id) override;
  uint16_t operator()(
      Event<ModifyOrder> const &,
      oms::Order const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id) override;
  uint16_t operator()(
      Event<CancelOrder> const &,
      oms::Order const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id) override;

  uint16_t operator()(Event<CancelAllOrders> const &, std::string_view const &request_id) override;

  void operator()(metrics::Writer &) override;

  // many

  void operator()(Trace<StreamStatus const> const &) override;
  void operator()(Trace<TopOfBook const> const &, bool is_last) override;
  void operator()(Trace<MarketByPriceUpdate const> const &, bool is_last, bool refresh) override;
  void operator()(Trace<TradeSummary const> const &, bool is_last) override;

 private:
  server::Dispatcher &dispatcher_;
  // config
  // io
  std::unique_ptr<io::Context> context_;
  // shared
  Shared shared_;
  // seed
  uint16_t stream_id_ = {};
  // streams
  std::unique_ptr<UDPInstrumentDefinition> udp_instrument_definition_;
  std::unique_ptr<UDPMBPMarketRecovery> udp_mbp_market_recovery_;
  std::unique_ptr<UDPIncremental> udp_incremental_;
};

}  // namespace cme
}  // namespace roq
