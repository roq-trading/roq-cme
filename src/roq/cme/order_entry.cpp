/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/cme/order_entry.hpp"

#include "roq/mask.hpp"

#include "roq/utils/update.hpp"

#include "roq/core/metrics/factory.hpp"

#include "roq/cme/flags/ilink.hpp"

using namespace std::literals;

using namespace fmt::literals;

namespace roq {
namespace cme {

// === CONSTANTS ===

namespace {
auto const NAME = "msgw"sv;

auto const SUPPORTS = Mask{
    SupportType::CREATE_ORDER,
    SupportType::MODIFY_ORDER,
    SupportType::CANCEL_ORDER,
    SupportType::ORDER_ACK,
    SupportType::ORDER,
    SupportType::TRADE,
};

auto const LOGOUT_RESPONSE = "LOGOUT"sv;
}  // namespace

// === HELPERS ===

namespace {
auto create_name(auto stream_id, auto const &account) {
  return fmt::format("{}:{}:{}"_cf, stream_id, NAME, account);
}

auto create_connection_factory(auto &context) {
  auto uri = flags::iLink::test_uri();
  auto config = io::net::ConnectionFactory::Config{
      .interface = {},
      .uris = {&uri, 1},
      .validate_certificate = server::Flags::net_tls_validate_certificate(),
  };
  return io::net::ConnectionFactory::create(context, config);
}

auto create_connection_manager(auto &handler, auto &connection_factory) {
  auto config = io::net::ConnectionManager::Config{
      .connection_timeout = server::Flags::net_connection_timeout(),
      .disconnect_on_idle_timeout = {},
      .always_reconnect = true,
  };
  return io::net::ConnectionManager::create(handler, connection_factory, config);
}

struct create_metrics final : public core::metrics::Factory {
  explicit create_metrics(auto const &group, auto const &function)
      : core::metrics::Factory(server::Flags::name(), group, function) {}
};
}  // namespace

// === IMPLEMENTATION ===

OrderEntry::OrderEntry(
    Handler &handler, io::Context &context, uint16_t stream_id, Authenticator &authenticator, Shared &shared)
    : handler_{handler}, stream_id_{stream_id}, name_{create_name(stream_id_, authenticator.get_account())},
      connection_factory_{create_connection_factory(context)},
      connection_manager_{create_connection_manager(*this, *connection_factory_)},
      decode_buffer_{flags::iLink::decode_buffer_size()},
      counter_{
          .disconnect = create_metrics(name_, "disconnect"sv),
      },
      profile_{
          .parse = create_metrics(name_, "parse"sv),
          .position_report = create_metrics(name_, "position_report"sv),
          .execution_report = create_metrics(name_, "execution_report"sv),
          .order_cancel_reject = create_metrics(name_, "order_cancel_reject"sv),
          .reject = create_metrics(name_, "reject"sv),
          .order_mass_cancel_report = create_metrics(name_, "order_mass_cancel_report"sv),
      },
      latency_{
          .ping = create_metrics(name_, "ping"sv),
      },
      authenticator_{authenticator}, shared_{shared} {
}

void OrderEntry::operator()(Event<Start> const &) {
  (*connection_manager_).start();
}

void OrderEntry::operator()(Event<Stop> const &) {
  (*connection_manager_).stop();
}

void OrderEntry::operator()(Event<Timer> const &event) {
  if (!(*connection_manager_).refresh(event.value.now))
    return;
}

uint16_t OrderEntry::operator()(
    Event<CreateOrder> const &, oms::Order const &, [[maybe_unused]] std::string_view const &request_id) {
  if (!ready()) [[unlikely]]
    throw oms::NotReady{"not ready"sv};
  // XXX
  return stream_id_;
}

uint16_t OrderEntry::operator()(
    Event<ModifyOrder> const &,
    oms::Order const &,
    [[maybe_unused]] std::string_view const &request_id,
    [[maybe_unused]] std::string_view const &previous_request_id) {
  if (!ready()) [[unlikely]]
    throw oms::NotReady{"not ready"sv};
  // XXX
  return stream_id_;
}

uint16_t OrderEntry::operator()(
    Event<CancelOrder> const &,
    oms::Order const &,
    [[maybe_unused]] std::string_view const &request_id,
    [[maybe_unused]] std::string_view const &previous_request_id) {
  if (!ready()) [[unlikely]]
    throw oms::NotReady{"not ready"sv};
  // XXX
  return stream_id_;
}

uint16_t OrderEntry::operator()(Event<CancelAllOrders> const &, [[maybe_unused]] std::string_view const &request_id) {
  // XXX
  return stream_id_;
}

void OrderEntry::operator()(metrics::Writer &writer) {
  writer  //
      .write(counter_.disconnect, metrics::COUNTER)
      .write(profile_.parse, metrics::PROFILE)
      .write(profile_.execution_report, metrics::PROFILE)
      .write(profile_.order_cancel_reject, metrics::PROFILE)
      .write(profile_.reject, metrics::PROFILE)
      .write(profile_.order_mass_cancel_report, metrics::PROFILE)
      .write(latency_.ping, metrics::LATENCY);
}

// session

void OrderEntry::operator()(Trace<cme_ilink::NegotiationResponse501> const &) {
}

void OrderEntry::operator()(Trace<cme_ilink::NegotiationReject502> const &) {
}

void OrderEntry::operator()(Trace<cme_ilink::EstablishmentAck504> const &) {
}

void OrderEntry::operator()(Trace<cme_ilink::EstablishmentReject505> const &) {
}

void OrderEntry::operator()(Trace<cme_ilink::Sequence506> const &) {
}

void OrderEntry::operator()(Trace<cme_ilink::Terminate507> const &) {
}

void OrderEntry::operator()(Trace<cme_ilink::Retransmission509> const &) {
}

void OrderEntry::operator()(Trace<cme_ilink::RetransmitReject510> const &) {
}

void OrderEntry::operator()(Trace<cme_ilink::NotApplied513> const &) {
}

// business

void OrderEntry::operator()(Trace<cme_ilink::BusinessReject521> const &) {
}

// execution report
void OrderEntry::operator()(Trace<cme_ilink::ExecutionReportNew522> const &) {
}

void OrderEntry::operator()(Trace<cme_ilink::ExecutionReportReject523> const &) {
}

void OrderEntry::operator()(Trace<cme_ilink::ExecutionReportTradeOutright525> const &) {
}

void OrderEntry::operator()(Trace<cme_ilink::ExecutionReportTradeSpread526> const &) {
}

void OrderEntry::operator()(Trace<cme_ilink::ExecutionReportTradeSpreadLeg527> const &) {
}

void OrderEntry::operator()(Trace<cme_ilink::ExecutionReportModify531> const &) {
}

void OrderEntry::operator()(Trace<cme_ilink::ExecutionReportStatus532> const &) {
}

void OrderEntry::operator()(Trace<cme_ilink::ExecutionReportCancel534> const &) {
}

void OrderEntry::operator()(Trace<cme_ilink::ExecutionReportPendingCancel564> const &) {
}

void OrderEntry::operator()(Trace<cme_ilink::ExecutionReportPendingReplace565> const &) {
}

// order

void OrderEntry::operator()(Trace<cme_ilink::OrderCancelReject535> const &) {
}

void OrderEntry::operator()(Trace<cme_ilink::OrderCancelReplaceReject536> const &) {
}

// security definition

void OrderEntry::operator()(Trace<cme_ilink::SecurityDefinitionResponse561> const &) {
}

void OrderEntry::operator()(io::net::ConnectionManager::Connected const &) {
}

void OrderEntry::operator()(io::net::ConnectionManager::Disconnected const &) {
  ++counter_.disconnect;
}

void OrderEntry::operator()(io::net::ConnectionManager::Read const &) {
  auto buffer = (*connection_manager_).buffer();
  size_t total_bytes = 0;
  while (!std::empty(buffer)) {
    TraceInfo trace_info;
    auto bytes = ilink::Parser::dispatch(*this, buffer, trace_info);
    if (bytes == 0)
      break;
    assert(bytes <= std::size(buffer));
    total_bytes += bytes;
    buffer = buffer.subspan(bytes);
  }
  (*connection_manager_).drain(total_bytes);
}

void OrderEntry::operator()(ConnectionStatus status) {
  if (utils::update(status_, status)) {
    TraceInfo trace_info;
    auto stream_status = StreamStatus{
        .stream_id = stream_id_,
        .account = authenticator_.get_account(),
        .supports = SUPPORTS,
        .transport = Transport::TCP,
        .protocol = Protocol::FIX,
        .encoding = {Encoding::SBE},
        .priority = Priority::PRIMARY,
        .connection_status = status_,
        .interface = (*connection_factory_).get_interface(),
        .authority = (*connection_factory_).get_current_authority(),
        .path = (*connection_factory_).get_current_path(),
        .proxy = {},
    };
    log::info("stream_status={}"sv, stream_status);
    create_trace_and_dispatch(handler_, trace_info, stream_status);
  }
}

}  // namespace cme
}  // namespace roq
