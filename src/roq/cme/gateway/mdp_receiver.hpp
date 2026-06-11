/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include "roq/utils/metrics/counter.hpp"
#include "roq/utils/metrics/profile.hpp"

#include "roq/io/context.hpp"

#include "roq/io/net/udp/receiver.hpp"

#include "roq/cme/market_data/manager.hpp"

#include "roq/cme/gateway/shared.hpp"

namespace roq {
namespace cme {
namespace gateway {

struct MDPReceiver final : public io::net::udp::Receiver::Handler {
  MDPReceiver(io::Context &, Shared &, market_data::Manager &, uint16_t channel_id, protocol::mdp::ConnectionType, Priority);

  MDPReceiver(MDPReceiver const &) = delete;

  void operator()(metrics::Writer &) const;

 protected:
  void operator()(io::net::udp::Receiver::Read const &) override;
  void operator()(io::net::udp::Receiver::Error const &) override;

 public:
  uint16_t const channel_id;
  protocol::mdp::ConnectionType const connection_type;
  Priority const priority;

 private:
  market_data::Manager &manager_;
  // config
  std::string const name_;
  // receiver
  std::unique_ptr<io::net::udp::Receiver> const receiver_;
  // metrics
  struct {
    utils::metrics::Counter disconnect;
  } counter_;
  struct {
    utils::metrics::Profile parse;
  } profile_;
  // cache
  Shared &shared_;
};

}  // namespace gateway
}  // namespace cme
}  // namespace roq
