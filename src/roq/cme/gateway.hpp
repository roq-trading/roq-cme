/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include <memory>
#include <string>

#include "roq/server.hpp"

#include "roq/utils/container.hpp"

#include "roq/io/context.hpp"

#include "roq/cme/market_data/manager.hpp"

#include "roq/cme/account.hpp"
#include "roq/cme/config.hpp"
#include "roq/cme/settings.hpp"
#include "roq/cme/shared.hpp"

#include "roq/cme/mdp_receiver.hpp"

#include "roq/cme/order_entry.hpp"

namespace roq {
namespace cme {

struct Gateway final : public server::Handler, public OrderEntry::Handler {
  Gateway(server::Dispatcher &, Settings const &, Config const &, io::Context &);

  Gateway(Gateway &&) = delete;
  Gateway(Gateway const &) = delete;

 protected:
  // server::Handler

  void operator()(Event<Start> const &) override;
  void operator()(Event<Stop> const &) override;
  void operator()(Event<Timer> const &) override;
  void operator()(Event<Connected> const &) override;
  void operator()(Event<Disconnected> const &) override;

  uint16_t operator()(
      Event<CreateOrder> const &, server::oms::Order const &, std::string_view const &request_id) override;
  uint16_t operator()(
      Event<ModifyOrder> const &,
      server::oms::Order const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id) override;
  uint16_t operator()(
      Event<CancelOrder> const &,
      server::oms::Order const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id) override;

  uint16_t operator()(Event<CancelAllOrders> const &, std::string_view const &request_id) override;

  void operator()(metrics::Writer &) override;

  // many

  void operator()(Trace<StreamStatus> const &) override;
  void operator()(Trace<ExternalLatency> const &) override;

  // utilities

  template <typename... Args>
  void dispatch(Args &&...);

  OrderEntry &get_order_entry(std::string_view const &account);

 private:
  server::Dispatcher &dispatcher_;
  // accounts
  utils::unordered_map<std::string, std::unique_ptr<Account>> const accounts_;
  // io
  io::Context &context_;
  // shared
  Shared shared_;
  // seed
  uint16_t stream_id_ = {};
  // mdp
  market_data::Manager manager_;
  // streams
  std::vector<std::unique_ptr<MDPReceiver>> mdp_receivers_;
  utils::unordered_map<std::string, std::unique_ptr<OrderEntry>> order_entry_;
};

}  // namespace cme
}  // namespace roq
