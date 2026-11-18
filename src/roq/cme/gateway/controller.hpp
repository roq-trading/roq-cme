/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include "roq/compat.hpp"

#include <memory>
#include <string>

#include "roq/server.hpp"

#include "roq/utils/container.hpp"

#include "roq/io/context.hpp"

#include "roq/cme/gateway/account.hpp"
#include "roq/cme/gateway/config.hpp"
#include "roq/cme/gateway/settings.hpp"
#include "roq/cme/gateway/shared.hpp"

#include "roq/cme/gateway/mdp_receiver.hpp"
#include "roq/cme/gateway/order_entry.hpp"

#include "roq/cme/market_data/manager.hpp"
#include "roq/cme/market_data/security_definitions.hpp"

namespace roq {
namespace cme {
namespace gateway {

struct Controller final : public server::Handler, public market_data::SecurityDefinitions::Dispatcher, public OrderEntry::Handler {
  ROQ_PUBLIC static std::unique_ptr<server::Handler> create(server::Dispatcher &, Settings const &, Config const &, io::Context &);

  ROQ_PUBLIC static uint8_t parse_api(Settings const &);

  Controller(server::Dispatcher &, Settings const &, Config const &, io::Context &);

  Controller(Controller const &) = delete;

 protected:
  // server::Handler

  void operator()(Trace<Start> const &) override;
  void operator()(Trace<Stop> const &) override;
  void operator()(Trace<Timer> const &) override;

  void operator()(Event<Connected> const &) override;
  void operator()(Event<Disconnected> const &) override;

  void operator()(Trace<Subscribe> const &) override;
  void operator()(Trace<Control> const &) override;

  void operator()(Trace<server::oms::OrderActionBatchBegin> const &) override;
  void operator()(Trace<server::oms::OrderActionBatchCommit> const &) override;
  void operator()(Trace<server::oms::OrderActionBatchRollback> const &) override;

  uint16_t operator()(Trace<CreateOrder> const &, server::oms::Order const &, server::oms::RefData const &, std::string_view const &request_id) override;
  uint16_t operator()(
      Trace<ModifyOrder> const &,
      server::oms::Order const &,
      server::oms::RefData const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id) override;
  uint16_t operator()(
      Trace<CancelOrder> const &,
      server::oms::Order const &,
      server::oms::RefData const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id) override;

  uint16_t operator()(Trace<CancelAllOrders> const &, std::string_view const &request_id) override;

  uint16_t operator()(Trace<MassQuote> const &) override;

  uint16_t operator()(Trace<CancelQuotes> const &) override;

  void operator()(metrics::Writer &) const override;

  // streams

  void operator()(Trace<StreamStatus> const &) override;
  void operator()(Trace<ExternalLatency> const &) override;

  // market_data::SecurityDefinitions::Dispatcher

  bool discard_symbol(std::string_view const &name) override { return dispatcher_.discard_symbol(name); }

  // helpers

  template <typename... Args>
  void dispatch(Args &&...);

  template <typename... Args>
  static void dispatch_helper(auto &self, Args &&...);

  template <typename Res, typename T, typename... Args>
  Res dispatch_order_action(Trace<T> const &, Args &&...);

 private:
  server::Dispatcher &dispatcher_;
  // accounts
  utils::unordered_map<std::string, std::unique_ptr<Account>> const accounts_;
  // io
  io::Context &context_;
  // shared
  market_data::SecurityDefinitions security_definitions_;
  Shared shared_;
  // seed
  uint16_t stream_id_ = {};
  // mdp
  market_data::Manager market_data_;
  // streams
  std::vector<std::unique_ptr<MDPReceiver>> mdp_receivers_;
  utils::unordered_map<std::string, std::unique_ptr<OrderEntry>> order_entry_;
};

}  // namespace gateway
}  // namespace cme
}  // namespace roq
