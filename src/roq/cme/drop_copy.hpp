/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "roq/core/download.hpp"

#include "roq/core/metrics/counter.hpp"
#include "roq/core/metrics/latency.hpp"
#include "roq/core/metrics/profile.hpp"

#include "roq/io/context.hpp"

#include "roq/web/socket/client.hpp"

#include "roq/core/jsonrpc/parser.hpp"

#include "roq/server.hpp"

#include "roq/deribit/drop_copy_state.hpp"
#include "roq/deribit/security.hpp"
#include "roq/deribit/shared.hpp"

#include "roq/deribit/json/auth.hpp"
#include "roq/deribit/json/changes.hpp"
#include "roq/deribit/json/order.hpp"
#include "roq/deribit/json/parser.hpp"
#include "roq/deribit/json/portfolio.hpp"
#include "roq/deribit/json/positions.hpp"
#include "roq/deribit/json/trades.hpp"

namespace roq {
namespace deribit {

class DropCopy final : public web::socket::Client::Handler,
                       public core::jsonrpc::Parser::Handler,
                       public json::Parser::Handler {
 public:
  struct Handler {
    virtual void operator()(Trace<StreamStatus const> const &) = 0;
    virtual void operator()(Trace<ExternalLatency const> const &) = 0;
    virtual void operator()(Trace<FundsUpdate const> const &, bool is_last) = 0;
    virtual void operator()(Trace<PositionUpdate const> const &, bool is_last) = 0;
  };

  DropCopy(Handler &, io::Context &, uint16_t stream_id, Security &, Shared &);

  DropCopy(DropCopy &&) = delete;
  DropCopy(DropCopy const &) = delete;

  void operator()(Event<Start> const &);
  void operator()(Event<Stop> const &);
  void operator()(Event<Timer> const &);

  void operator()(metrics::Writer &);

  void update_subscriptions(std::span<std::string> const &currencies);

 protected:
  void operator()(web::socket::Client::Connected const &) override;
  void operator()(web::socket::Client::Disconnected const &) override;
  void operator()(web::socket::Client::Ready const &) override;
  void operator()(web::socket::Client::Close const &) override;
  void operator()(web::socket::Client::Latency const &) override;
  void operator()(web::socket::Client::Text const &) override;
  void operator()(web::socket::Client::Binary const &) override;

 private:
  void operator()(ConnectionStatus);

  void login();

  uint32_t download(DropCopyState);

  void subscribe_portfolios(std::span<std::string> const &currencies);
  void subscribe_changes();
  void subscribe_orders();
  void subscribe_trades();

  void get_account_summary(std::span<std::string> const &currencies);
  void get_trades(std::span<std::string> const &currencies);

  void parse(std::string_view const &message);

  void operator()(Trace<core::jsonrpc::Error const> const &, core::json::Value &) override;
  void operator()(Trace<core::jsonrpc::Result const> const &, core::json::Value &) override;
  void operator()(Trace<core::jsonrpc::Notification const> const &, core::json::Value &) override;

  void operator()(Trace<json::Auth const> const &);

 public:
  void operator()(Trace<json::PlatformState const> const &) override;
  void operator()(Trace<json::InstrumentState const> const &) override;
  void operator()(Trace<json::Quote const> const &) override;
  void operator()(Trace<json::Ticker const> const &) override;
  void operator()(Trace<json::Portfolio const> const &) override;
  void operator()(Trace<json::Changes const> const &) override;

  void operator()(Trace<json::Trades const> const &);
  void operator()(Trace<json::Positions const> const &);
  void operator()(Trace<json::Order const> const &) override;
  void operator()(Trace<json::Trades2 const> const &) override;

  void operator()(Trace<json::Trade const> const &, bool is_last);

 private:
  Handler &handler_;
  // config
  const uint16_t stream_id_;
  const std::string name_;
  // web socket
  std::unique_ptr<web::socket::Client> connection_;
  // buffers
  core::Buffer decode_buffer_;
  // metrics
  struct {
    core::metrics::Counter disconnect;
  } counter_;
  struct {
    core::metrics::Profile parse, auth;
  } profile_;
  struct {
    core::metrics::Latency ping, heartbeat;
  } latency_;
  // security
  Security &security_;
  // cache
  Shared &shared_;
  std::vector<std::string> currencies_;
  // state
  bool ready_ = false;
  ConnectionStatus status_ = {};
  core::Download<DropCopyState> download_;
};

}  // namespace deribit
}  // namespace roq
