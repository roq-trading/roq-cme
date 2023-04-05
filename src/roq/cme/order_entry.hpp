/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <string>

#include "roq/core/buffer.hpp"

#include "roq/core/metrics/counter.hpp"
#include "roq/core/metrics/latency.hpp"
#include "roq/core/metrics/profile.hpp"

#include "roq/io/context.hpp"
#include "roq/io/net/connection_factory.hpp"
#include "roq/io/net/connection_manager.hpp"

#include "roq/server.hpp"

#include "roq/cme/authenticator.hpp"
#include "roq/cme/shared.hpp"

#include "roq/cme/ilink/parser.hpp"

namespace roq {
namespace cme {

struct OrderEntry final : public io::net::ConnectionManager::Handler, public ilink::Parser::Handler {
  struct Handler {
    virtual void operator()(Trace<StreamStatus> const &) = 0;
    virtual void operator()(Trace<ExternalLatency> const &) = 0;
    virtual void operator()(Trace<oms::TradeUpdate> const &, uint16_t stream_id, bool is_last, uint8_t user_id) = 0;
  };

  OrderEntry(Handler &, io::Context &, uint16_t stream_id, Authenticator &, Shared &);

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

 private:
  // session
  void operator()(Trace<cme_ilink::NegotiationResponse501> const &) override;
  void operator()(Trace<cme_ilink::NegotiationReject502> const &) override;
  void operator()(Trace<cme_ilink::EstablishmentAck504> const &) override;
  void operator()(Trace<cme_ilink::EstablishmentReject505> const &) override;
  void operator()(Trace<cme_ilink::Sequence506> const &) override;
  void operator()(Trace<cme_ilink::Terminate507> const &) override;
  void operator()(Trace<cme_ilink::Retransmission509> const &) override;
  void operator()(Trace<cme_ilink::RetransmitReject510> const &) override;
  void operator()(Trace<cme_ilink::NotApplied513> const &) override;
  // business
  void operator()(Trace<cme_ilink::BusinessReject521> const &) override;
  // execution report
  void operator()(Trace<cme_ilink::ExecutionReportNew522> const &) override;
  void operator()(Trace<cme_ilink::ExecutionReportReject523> const &) override;
  void operator()(Trace<cme_ilink::ExecutionReportTradeOutright525> const &) override;
  void operator()(Trace<cme_ilink::ExecutionReportTradeSpread526> const &) override;
  void operator()(Trace<cme_ilink::ExecutionReportTradeSpreadLeg527> const &) override;
  void operator()(Trace<cme_ilink::ExecutionReportModify531> const &) override;
  void operator()(Trace<cme_ilink::ExecutionReportStatus532> const &) override;
  void operator()(Trace<cme_ilink::ExecutionReportCancel534> const &) override;
  void operator()(Trace<cme_ilink::ExecutionReportPendingCancel564> const &) override;
  void operator()(Trace<cme_ilink::ExecutionReportPendingReplace565> const &) override;
  // order
  void operator()(Trace<cme_ilink::OrderCancelReject535> const &) override;
  void operator()(Trace<cme_ilink::OrderCancelReplaceReject536> const &) override;
  // security definition
  void operator()(Trace<cme_ilink::SecurityDefinitionResponse561> const &) override;

 protected:
  void operator()(io::net::ConnectionManager::Connected const &) override;
  void operator()(io::net::ConnectionManager::Disconnected const &) override;
  void operator()(io::net::ConnectionManager::Read const &) override;

 private:
  void operator()(ConnectionStatus);

  void send_negotiate();
  void send_establish();

  template <typename T>
  void send(T const &);

 private:
  Handler &handler_;
  // config
  uint16_t const stream_id_;
  Source const name_;
  // connection
  std::unique_ptr<io::net::ConnectionFactory> connection_factory_;
  std::unique_ptr<io::net::ConnectionManager> connection_manager_;
  // buffers
  core::Buffer decode_buffer_;
  std::string encode_buffer_;
  std::vector<std::byte> encode_buffer_2_;
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
  // authenticator
  Authenticator &authenticator_;
  // cache
  Shared &shared_;
  // state
  ConnectionStatus status_ = {};
  bool ready_ = false;
  std::chrono::nanoseconds next_heartbeat_ = {};
  // EXPERIMENTAL
  std::chrono::nanoseconds uuid_ = {};
};

}  // namespace cme
}  // namespace roq
