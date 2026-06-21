/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <string>

#include "roq/utils/metrics/counter.hpp"
#include "roq/utils/metrics/latency.hpp"
#include "roq/utils/metrics/profile.hpp"

#include "roq/io/context.hpp"

#include "roq/io/net/connection_factory.hpp"
#include "roq/io/net/connection_manager.hpp"

#include "roq/core/download.hpp"

#include "roq/server.hpp"

#include "roq/cme/gateway/account.hpp"
#include "roq/cme/gateway/order_entry_state.hpp"
#include "roq/cme/gateway/shared.hpp"

#include "roq/cme/protocol/ilink/parser.hpp"

namespace roq {
namespace cme {
namespace gateway {

struct OrderEntry final : public io::net::ConnectionManager::Handler, public protocol::ilink::Parser::Handler {
  struct Handler {
    virtual void operator()(Trace<StreamStatus> const &) = 0;
    virtual void operator()(Trace<ExternalLatency> const &) = 0;
  };

  OrderEntry(Handler &, io::Context &, uint16_t stream_id, Account &, Shared &, uint8_t market_segment_id, io::web::URI const &);

  OrderEntry(OrderEntry const &) = delete;

  void operator()(Event<Start> const &);
  void operator()(Event<Stop> const &);
  void operator()(Event<Timer> const &);

  uint16_t operator()(Event<CreateOrder> const &, server::oms::Order const &, server::oms::RefData const &, std::string_view const &request_id);
  uint16_t operator()(
      Event<ModifyOrder> const &,
      server::oms::Order const &,
      server::oms::RefData const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id);
  uint16_t operator()(
      Event<CancelOrder> const &,
      server::oms::Order const &,
      server::oms::RefData const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id);

  uint16_t operator()(Event<CancelAllOrders> const &, std::string_view const &request_id);

  void operator()(metrics::Writer &) const;

 protected:
  // session
  void operator()(Trace<::cme::sbe::ilink::NegotiationResponse501> const &) override;
  void operator()(Trace<::cme::sbe::ilink::NegotiationReject502> const &) override;
  void operator()(Trace<::cme::sbe::ilink::EstablishmentAck504> const &) override;
  void operator()(Trace<::cme::sbe::ilink::EstablishmentReject505> const &) override;
  void operator()(Trace<::cme::sbe::ilink::Sequence506> const &) override;
  void operator()(Trace<::cme::sbe::ilink::Terminate507> const &) override;
  void operator()(Trace<::cme::sbe::ilink::Retransmission509> const &) override;
  void operator()(Trace<::cme::sbe::ilink::RetransmitReject510> const &) override;
  void operator()(Trace<::cme::sbe::ilink::NotApplied513> const &) override;
  // business
  void operator()(Trace<::cme::sbe::ilink::PartyDetailsDefinitionRequestAck519> const &) override;
  void operator()(Trace<::cme::sbe::ilink::BusinessReject521> const &) override;
  // execution report
  void operator()(Trace<::cme::sbe::ilink::ExecutionReportNew522> const &) override;
  void operator()(Trace<::cme::sbe::ilink::ExecutionReportReject523> const &) override;
  void operator()(Trace<::cme::sbe::ilink::ExecutionReportTradeOutright525> const &) override;
  void operator()(Trace<::cme::sbe::ilink::ExecutionReportTradeSpread526> const &) override;
  void operator()(Trace<::cme::sbe::ilink::ExecutionReportTradeSpreadLeg527> const &) override;
  void operator()(Trace<::cme::sbe::ilink::ExecutionReportModify531> const &) override;
  void operator()(Trace<::cme::sbe::ilink::ExecutionReportStatus532> const &) override;
  void operator()(Trace<::cme::sbe::ilink::ExecutionReportCancel534> const &) override;
  void operator()(Trace<::cme::sbe::ilink::ExecutionReportPendingCancel564> const &) override;
  void operator()(Trace<::cme::sbe::ilink::ExecutionReportPendingReplace565> const &) override;
  // order
  void operator()(Trace<::cme::sbe::ilink::OrderCancelReject535> const &) override;
  void operator()(Trace<::cme::sbe::ilink::OrderCancelReplaceReject536> const &) override;
  // order mass action
  virtual void operator()(Trace<::cme::sbe::ilink::OrderMassActionReport562> const &) override;
  // security definition
  void operator()(Trace<::cme::sbe::ilink::SecurityDefinitionResponse561> const &) override;

  // io::net::ConnectionManager::Handler

  void operator()(io::net::ConnectionManager::Connected const &) override;
  void operator()(io::net::ConnectionManager::Disconnected const &) override;
  void operator()(io::net::ConnectionManager::Read const &) override;
  void operator()(io::net::ConnectionManager::Write const &) override;

  // helpers

  size_t parse(std::span<std::byte const> const &);

  bool ready() const { return connection_status_ == ConnectionStatus::READY; }

  void operator()(ConnectionStatus, std::string_view const &reason = {});

  uint32_t download(OrderEntryState state);

  uint32_t peek_next_seq_num();
  uint32_t fetch_next_seq_num();

  uint64_t fetch_next_request_id();

  template <typename T>
  void send(T const &);

  void send_negotiate();
  void send_establish();
  void send_sequence();
  void send_terminate();

  void send_party_details_list_request();
  void send_party_details_definition_request();

  void send_security_definition_request();

  void send_order_mass_status_request();

  void send_new_order_single(CreateOrder const &, server::oms::Order const &, std::string_view const &request_id);
  void send_order_cancel_replace_request(ModifyOrder const &, server::oms::Order const &);
  void send_order_cancel_request(CancelOrder const &, server::oms::Order const &);
  void send_order_mass_action_request(CancelAllOrders const &);

 private:
  Handler &handler_;
  // config
  uint16_t const stream_id_;
  std::string const name_;
  uint8_t const market_segment_id_;
  uint64_t const party_details_list_req_id_ = {};  // XXX const until we get access to a service GW
  // connection
  std::unique_ptr<io::net::ConnectionFactory> const connection_factory_;
  std::unique_ptr<io::net::ConnectionManager> const connection_manager_;
  // buffers
  std::vector<std::byte> decode_buffer_;
  std::string encode_buffer_;
  std::vector<std::byte> encode_buffer_2_;
  // metrics
  struct {
    utils::metrics::Counter disconnect;
  } counter_;
  struct {
    utils::metrics::Profile parse,
        // session
        negotiation_response,   //
        negotiation_reject,     //
        establishment_ack,      //
        establishment_reject,   //
        sequence,               //
        terminate,              //
        retransmission,         //
        retransmission_reject,  //
        not_applied,            //
        // business
        party_details_definition_request_ack,  //
        business_reject,                       //
        // execution report
        execution_report_new,               //
        execution_report_reject,            //
        execution_report_trade_outright,    //
        execution_report_trade_spread,      //
        execution_report_trade_spread_leg,  //
        execution_report_modify,            //
        execution_report_status,            //
        execution_report_cancel,            //
        execution_report_pending_cancel,    //
        execution_report_pending_replace,   //
        // order
        order_cancel_reject,          //
        order_cancel_replace_reject,  //
        // order mass action
        order_mass_action_report,  //
        // security definition
        security_definition_response;  //
  } profile_;
  struct {
    utils::metrics::Latency ping;
  } latency_;
  // state
  struct {
    uint32_t msg_seq_num = {};
  } outbound_;
  struct {
    uint32_t msg_seq_num = {};
  } inbound_;
  // account
  Account &account_;
  // cache
  Shared &shared_;
  // state
  ConnectionStatus connection_status_ = {};
  core::Download<OrderEntryState> download_;
  std::chrono::nanoseconds next_heartbeat_ = {};
  uint64_t uuid_ = {};
  uint64_t request_id_ = {};
};

}  // namespace gateway
}  // namespace cme
}  // namespace roq
