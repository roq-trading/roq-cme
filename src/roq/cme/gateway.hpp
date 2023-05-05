/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <absl/container/flat_hash_map.h>

#include <memory>
#include <string>
#include <vector>

#include "roq/server.hpp"

#include "roq/io/context.hpp"

#include "roq/cme/account.hpp"
#include "roq/cme/channel.hpp"
#include "roq/cme/config.hpp"
#include "roq/cme/shared.hpp"

#include "roq/cme/udp_incremental.hpp"
#include "roq/cme/udp_instrument_definition.hpp"
#include "roq/cme/udp_mbo_market_recovery.hpp"
#include "roq/cme/udp_mbp_market_recovery.hpp"

#include "roq/cme/order_entry.hpp"

namespace roq {
namespace cme {

struct Gateway final : public server::Handler,
                       public UDPIncremental::Handler,
                       public UDPInstrumentDefinition::Handler,
                       public UDPMBPMarketRecovery::Handler,
                       public UDPMBOMarketRecovery::Handler,
                       public OrderEntry::Handler {
  Gateway(server::Dispatcher &, Config const &, io::Context &);

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

  void operator()(Trace<StreamStatus> const &) override;
  void operator()(Trace<ExternalLatency> const &) override;
  void operator()(Trace<ReferenceData> const &, bool is_last) override;
  void operator()(Trace<MarketStatus> const &, bool is_last) override;
  void operator()(Trace<TopOfBook> const &, bool is_last) override;
  void operator()(Trace<MarketByPriceUpdate> const &, bool is_last) override;
  void operator()(Trace<MarketByOrderUpdate> const &, bool is_last) override;
  void operator()(Trace<TradeSummary> const &, bool is_last) override;
  void operator()(Trace<StatisticsUpdate> const &, bool is_last) override;

  // utilities

  template <typename... Args>
  void dispatch(Args &&...);

 private:
  server::Dispatcher &dispatcher_;
  // accounts
  absl::flat_hash_map<std::string, std::unique_ptr<Account>> const accounts_;
  // io
  io::Context &context_;
  // shared
  Shared shared_;
  std::vector<Channel> channels_;
  // seed
  uint16_t stream_id_ = {};
  // streams
  std::vector<std::unique_ptr<UDPIncremental>> udp_incremental_;
  std::vector<std::unique_ptr<UDPInstrumentDefinition>> udp_instrument_definition_;
  std::vector<std::unique_ptr<UDPMBPMarketRecovery>> udp_mbp_market_recovery_;
  std::vector<std::unique_ptr<UDPMBOMarketRecovery>> udp_mbo_market_recovery_;
  absl::flat_hash_map<std::string, std::unique_ptr<OrderEntry>> order_entry_;
  // cache
  std::vector<MBPUpdate> bids_, asks_;
  std::vector<MBOUpdate> orders_;
};

}  // namespace cme
}  // namespace roq
