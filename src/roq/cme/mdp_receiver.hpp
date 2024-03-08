/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include "roq/utils/metrics/counter.hpp"
#include "roq/utils/metrics/profile.hpp"

#include "roq/io/context.hpp"

#include "roq/io/net/udp/receiver.hpp"

#include "roq/cme/market_data/manager.hpp"

#include "roq/cme/shared.hpp"

namespace roq {
namespace cme {

struct MDPReceiver final : public io::net::udp::Receiver::Handler {
  MDPReceiver(io::Context &, Shared &, market_data::Manager &, uint16_t channel_id, mdp::ConnectionType, Priority);

  MDPReceiver(MDPReceiver const &) = delete;
  MDPReceiver(MDPReceiver &&) = delete;

  void operator()(metrics::Writer &);

 protected:
  void operator()(io::net::udp::Receiver::Read const &) override;
  void operator()(io::net::udp::Receiver::Error const &) override;

 public:
  uint16_t const channel_id;
  mdp::ConnectionType const connection_type;
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

}  // namespace cme
}  // namespace roq
