/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>

#include <string>

#include "roq/core/download.hpp"

#include "roq/core/stack/buffer.hpp"

#include "roq/core/metrics/counter.hpp"
#include "roq/core/metrics/latency.hpp"
#include "roq/core/metrics/profile.hpp"

#include "roq/io/context.hpp"
#include "roq/io/net/connection_factory.hpp"
#include "roq/io/net/connection_manager.hpp"

#include "roq/server.hpp"

#include "roq/deribit/order_entry_state.hpp"
#include "roq/deribit/security.hpp"
#include "roq/deribit/shared.hpp"

// session
#include "roq/deribit/fix/heartbeat.hpp"
#include "roq/deribit/fix/logon.hpp"
#include "roq/deribit/fix/logout.hpp"
#include "roq/deribit/fix/resend_request.hpp"
#include "roq/deribit/fix/test_request.hpp"

// business (inbound)
#include "roq/deribit/fix/execution_report.hpp"
#include "roq/deribit/fix/order_cancel_reject.hpp"
#include "roq/deribit/fix/order_mass_cancel_report.hpp"
#include "roq/deribit/fix/position_report.hpp"
#include "roq/deribit/fix/reject.hpp"  // ... normally session level

namespace roq {
namespace deribit {

class OrderEntry final : public io::net::ConnectionManager::Handler {
 public:
  struct Handler {
    virtual void operator()(Trace<StreamStatus const> const &) = 0;
    virtual void operator()(Trace<ExternalLatency const> const &) = 0;
    virtual void operator()(Trace<TradeUpdate const> const &, bool is_last, uint8_t user_id) = 0;
    virtual void operator()(Trace<PositionUpdate const> const &, bool is_last) = 0;
  };

  OrderEntry(Handler &, io::Context &, uint16_t stream_id, Security &, Shared &);

  OrderEntry(OrderEntry const &) = delete;
  OrderEntry(OrderEntry &&) = delete;

  bool ready() const { return status_ == ConnectionStatus::READY; }

  void operator()(Event<Start> const &);
  void operator()(Event<Stop> const &);
  void operator()(Event<Timer> const &);

  uint16_t operator()(Event<CreateOrder> const &, oms::Order const &, std::string_view const &request_id);
  uint16_t operator()(
      Event<ModifyOrder> const &,
      oms::Order const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id);
  uint16_t operator()(
      Event<CancelOrder> const &,
      oms::Order const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id);

  uint16_t operator()(Event<CancelAllOrders> const &, std::string_view const &request_id);

  void operator()(metrics::Writer &);

  void operator()(Trace<fix::Heartbeat const> const &, core::fix::Header const &);
  void operator()(Trace<fix::Logon const> const &, core::fix::Header const &);
  void operator()(Trace<fix::Logout const> const &, core::fix::Header const &);
  void operator()(Trace<fix::ResendRequest const> const &, core::fix::Header const &);
  void operator()(Trace<fix::TestRequest const> const &, core::fix::Header const &);

  void operator()(Trace<fix::PositionReport const> const &, core::fix::Header const &);

  void operator()(Trace<fix::ExecutionReport const> const &, core::fix::Header const &);
  void operator()(Trace<fix::OrderCancelReject const> const &, core::fix::Header const &);
  void operator()(Trace<fix::Reject const> const &, core::fix::Header const &);
  void operator()(Trace<fix::OrderMassCancelReport const> const &, core::fix::Header const &);

 protected:
  void operator()(io::net::ConnectionManager::Connected const &) override;
  void operator()(io::net::ConnectionManager::Disconnected const &) override;
  void operator()(io::net::ConnectionManager::Read const &) override;

 private:
  void operator()(ConnectionStatus);

  void send_logon();
  void send_logout(std::string_view const &text);
  void send_heartbeat(std::string_view const &test_req_id);
  void send_test_request(std::chrono::nanoseconds now);

  uint32_t download(OrderEntryState);

  void subscribe_positions();
  void download_orders();

  void parse(Trace<core::fix::Message const> const &);
  void parse_helper(Trace<core::fix::Message const> const &);

  // utilities

  template <typename T>
  uint64_t send(const T &event);

  template <typename T>
  uint64_t send(const T &event, std::chrono::nanoseconds sending_time);

  void check(core::fix::Header const &);

 private:
  Handler &handler_;
  // config
  const uint16_t stream_id_;
  const std::string name_;
  // connection
  std::unique_ptr<io::net::ConnectionFactory> connection_factory_;
  std::unique_ptr<io::net::ConnectionManager> connection_manager_;
  // buffers
  core::Buffer encode_buffer_;
  core::Buffer decode_buffer_;
  core::stack::Buffer<char, 32> stack_buffer_;
  // metrics
  struct {
    core::metrics::Counter disconnect;
  } counter_;
  struct {
    core::metrics::Profile parse, position_report, execution_report, order_cancel_reject, reject,
        order_mass_cancel_report;
  } profile_;
  struct {
    core::metrics::Latency ping;
  } latency_;
  // state
  struct {
    uint64_t msg_seq_num = {};
  } outbound_;
  struct {
    uint64_t msg_seq_num = {};
  } inbound_;
  // security
  Security &security_;
  // cache
  Shared &shared_;
  absl::flat_hash_set<std::string> all_currencies_;  // only master
  // state
  bool ready_ = false;
  std::chrono::nanoseconds next_heartbeat_ = {};
  ConnectionStatus status_ = {};
  core::Download<OrderEntryState> download_;
  std::chrono::nanoseconds last_logon_or_heartbeat_ = {};
  // EXPERIMENTAL
  absl::flat_hash_map<uint64_t, std::string> msg_seq_num_to_request_id_;
  std::chrono::nanoseconds test_disconnect_time_ = {};
  std::chrono::nanoseconds test_logon_time_ = {};
};

}  // namespace deribit
}  // namespace roq
