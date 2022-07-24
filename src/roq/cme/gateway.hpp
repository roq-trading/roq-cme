/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <absl/container/flat_hash_map.h>

#include <memory>
#include <string>
#include <vector>

#include "roq/server.hpp"

#include "roq/io/context.hpp"

#include "roq/deribit/config.hpp"
#include "roq/deribit/drop_copy.hpp"
#include "roq/deribit/market_data.hpp"
#include "roq/deribit/order_entry.hpp"
#include "roq/deribit/security.hpp"
#include "roq/deribit/shared.hpp"
#include "roq/deribit/udp_events.hpp"
#include "roq/deribit/udp_snapshot.hpp"
#include "roq/deribit/web_socket.hpp"

namespace roq {
namespace deribit {

class Gateway final : public server::Handler,
                      public OrderEntry::Handler,
                      public DropCopy::Handler,
                      public WebSocket::Handler,
                      public MarketData::Handler,
                      public UDPSnapshot::Handler,
                      public UDPEvents::Handler {
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
  void operator()(Trace<ExternalLatency const> const &) override;
  void operator()(Trace<ReferenceData const> const &, bool is_last) override;
  void operator()(Trace<MarketStatus const> const &, bool is_last) override;
  void operator()(Trace<TopOfBook const> const &, bool is_last) override;
  void operator()(Trace<MarketByPriceUpdate const> const &, bool is_last, bool refresh) override;
  void operator()(Trace<TradeSummary const> const &, bool is_last) override;
  void operator()(Trace<StatisticsUpdate const> const &, bool is_last) override;
  void operator()(Trace<TradeUpdate const> const &, bool is_last, uint8_t user_id) override;
  void operator()(Trace<PositionUpdate const> const &, bool is_last) override;
  void operator()(Trace<FundsUpdate const> const &, bool is_last) override;

  void operator()(WebSocket::CurrenciesUpdate &) override;
  void operator()(WebSocket::SymbolsUpdate &) override;

  void operator()(MarketData::SymbolsUpdate &) override;

  void ensure_symbol_slices(size_t size);

  // utilities

  OrderEntry &get_order_entry(std::string_view const &account);

 private:
  server::Dispatcher &dispatcher_;
  // config
  const std::string master_account_;
  // security
  absl::flat_hash_map<Account, std::unique_ptr<Security>> security_;
  // io
  std::unique_ptr<io::Context> context_;
  // shared
  Shared shared_;
  // seed
  uint16_t stream_id_ = {};
  // streams
  absl::flat_hash_map<Account, std::unique_ptr<OrderEntry>> order_entry_;
  absl::flat_hash_map<Account, std::unique_ptr<DropCopy>> drop_copy_;
  std::vector<std::unique_ptr<WebSocket>> web_socket_;
  std::vector<std::unique_ptr<MarketData>> market_data_;
  std::unique_ptr<UDPSnapshot> udp_snapshot_;
  std::unique_ptr<UDPEvents> udp_events_;
};

}  // namespace deribit
}  // namespace roq
