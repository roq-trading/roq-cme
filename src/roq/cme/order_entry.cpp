/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/cme/order_entry.hpp"

#include "roq/mask.hpp"

#include "roq/utils/safe_cast.hpp"
#include "roq/utils/update.hpp"

#include "roq/core/byte_order.hpp"

#include "roq/core/metrics/factory.hpp"

#include "roq/cme/ilink/session.hpp"

#include "roq/cme/ilink/establish.hpp"
#include "roq/cme/ilink/negotiate.hpp"
#include "roq/cme/ilink/sequence.hpp"
#include "roq/cme/ilink/terminate.hpp"

#include "roq/cme/ilink/party_details_definition_request.hpp"
#include "roq/cme/ilink/party_details_list_request.hpp"

#include "roq/cme/ilink/security_definition_request.hpp"

#include "roq/cme/ilink/order_mass_status_request.hpp"

#include "roq/cme/ilink/new_order_single.hpp"
#include "roq/cme/ilink/order_cancel_replace_request.hpp"
#include "roq/cme/ilink/order_cancel_request.hpp"
#include "roq/cme/ilink/order_mass_action_request.hpp"

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

auto const KEEP_ALIVE_INTERVAL = 30s;
}  // namespace

// === HELPERS ===

namespace {
auto create_name(auto stream_id, auto const &account) {
  return fmt::format("{}:{}:{}"_cf, stream_id, NAME, account);
}

auto create_connection_factory(auto &settings, auto &context) {
  auto uri = settings.test.uri;
  auto config = io::net::ConnectionFactory::Config{
      .interface = {},
      .uris = {&uri, 1},
      .validate_certificate = settings.net.tls_validate_certificate,
  };
  return io::net::ConnectionFactory::create(context, config);
}

auto create_connection_manager(auto &handler, auto &settings, auto &connection_factory) {
  auto config = io::net::ConnectionManager::Config{
      .connection_timeout = settings.net.connection_timeout,
      .disconnect_on_idle_timeout = {},
      .always_reconnect = true,
  };
  return io::net::ConnectionManager::create(handler, connection_factory, config);
}

struct create_metrics final : public core::metrics::Factory {
  explicit create_metrics(auto &settings, auto const &group, auto const &function)
      : core::metrics::Factory(settings.app.name, group, function) {}
};
}  // namespace

// === IMPLEMENTATION ===

OrderEntry::OrderEntry(Handler &handler, io::Context &context, uint16_t stream_id, Account &account, Shared &shared)
    : handler_{handler}, stream_id_{stream_id}, name_{create_name(stream_id_, account.get_name())},
      connection_factory_{create_connection_factory(shared.settings, context)},
      connection_manager_{create_connection_manager(*this, shared.settings, *connection_factory_)},
      decode_buffer_(shared.settings.common.decode_buffer_size),
      encode_buffer_2_(shared.settings.common.encode_buffer_size),
      counter_{
          .disconnect = create_metrics(shared.settings, name_, "disconnect"sv),
      },
      profile_{
          .parse = create_metrics(shared.settings, name_, "parse"sv),
          .position_report = create_metrics(shared.settings, name_, "position_report"sv),
          .execution_report = create_metrics(shared.settings, name_, "execution_report"sv),
          .order_cancel_reject = create_metrics(shared.settings, name_, "order_cancel_reject"sv),
          .reject = create_metrics(shared.settings, name_, "reject"sv),
          .order_mass_cancel_report = create_metrics(shared.settings, name_, "order_mass_cancel_report"sv),
      },
      latency_{
          .ping = create_metrics(shared.settings, name_, "ping"sv),
      },
      account_{account}, shared_{shared} {
}

void OrderEntry::operator()(Event<Start> const &) {
  (*connection_manager_).start();
}

void OrderEntry::operator()(Event<Stop> const &) {
  (*connection_manager_).stop();
}

void OrderEntry::operator()(Event<Timer> const &event) {
  auto now = event.value.now;
  if (!(*connection_manager_).refresh(now))
    return;
  if (!ready() || now < next_heartbeat_)
    return;
  send_sequence();  // note! send() updates next_heartbeat
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

void OrderEntry::operator()(Trace<cme_ilink::NegotiationResponse501> const &event) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &[trace_info, value] = event;
  log::info("DEBUG negotiation_response={}"sv, const_cast<value_type &>(value));
  send_establish();
}

void OrderEntry::operator()(Trace<cme_ilink::NegotiationReject502> const &event) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &[trace_info, value] = event;
  log::error("negotiation_reject={}"sv, const_cast<value_type &>(value));
  // XXX now what?
  (*connection_manager_).close();
}

void OrderEntry::operator()(Trace<cme_ilink::EstablishmentAck504> const &event) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &[trace_info, value] = event;
  log::info("DEBUG establishment_ack={}"sv, const_cast<value_type &>(value));
  (*this)(ConnectionStatus::READY);
  // EXPERIMENTAL
  // send_party_details_list_request();
  send_party_details_definition_request();
  // send_order_mass_status_request();
}

void OrderEntry::operator()(Trace<cme_ilink::EstablishmentReject505> const &event) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &[trace_info, value] = event;
  log::error("establishment_reject={}"sv, const_cast<value_type &>(value));
  // XXX now what?
  (*connection_manager_).close();
}

void OrderEntry::operator()(Trace<cme_ilink::Sequence506> const &event) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &[trace_info, value] = event;
  log::info("DEBUG sequence={}"sv, const_cast<value_type &>(value));
}

void OrderEntry::operator()(Trace<cme_ilink::Terminate507> const &event) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &[trace_info, value] = event;
  log::warn("terminate={}"sv, const_cast<value_type &>(value));
  (*connection_manager_).close();
}

void OrderEntry::operator()(Trace<cme_ilink::Retransmission509> const &event) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &[trace_info, value] = event;
  log::warn("retransmission={}"sv, const_cast<value_type &>(value));
}

void OrderEntry::operator()(Trace<cme_ilink::RetransmitReject510> const &event) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &[trace_info, value] = event;
  log::warn("retransmit_reject={}"sv, const_cast<value_type &>(value));
}

void OrderEntry::operator()(Trace<cme_ilink::NotApplied513> const &event) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &[trace_info, value] = event;
  log::warn("not_applied={}"sv, const_cast<value_type &>(value));
}

// business

void OrderEntry::operator()(Trace<cme_ilink::BusinessReject521> const &event) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &[trace_info, value] = event;
  log::error("business_reject={}"sv, const_cast<value_type &>(value));
}

// execution report
void OrderEntry::operator()(Trace<cme_ilink::ExecutionReportNew522> const &event) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &[trace_info, value] = event;
  log::info("DEBUG execution_report_new={}"sv, const_cast<value_type &>(value));
}

void OrderEntry::operator()(Trace<cme_ilink::ExecutionReportReject523> const &event) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &[trace_info, value] = event;
  log::info("DEBUG execution_report_reject={}"sv, const_cast<value_type &>(value));
}

void OrderEntry::operator()(Trace<cme_ilink::ExecutionReportTradeOutright525> const &event) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &[trace_info, value] = event;
  log::info("DEBUG execution_report_trade_outright={}"sv, const_cast<value_type &>(value));
}

void OrderEntry::operator()(Trace<cme_ilink::ExecutionReportTradeSpread526> const &event) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &[trace_info, value] = event;
  log::info("DEBUG execution_report_trade_spread={}"sv, const_cast<value_type &>(value));
}

void OrderEntry::operator()(Trace<cme_ilink::ExecutionReportTradeSpreadLeg527> const &event) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &[trace_info, value] = event;
  log::info("DEBUG execution_report_trade_spread_leg={}"sv, const_cast<value_type &>(value));
}

void OrderEntry::operator()(Trace<cme_ilink::ExecutionReportModify531> const &event) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &[trace_info, value] = event;
  log::info("DEBUG execution_report_modify={}"sv, const_cast<value_type &>(value));
}

void OrderEntry::operator()(Trace<cme_ilink::ExecutionReportStatus532> const &event) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &[trace_info, value] = event;
  log::info("DEBUG execution_report_status={}"sv, const_cast<value_type &>(value));
}

void OrderEntry::operator()(Trace<cme_ilink::ExecutionReportCancel534> const &event) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &[trace_info, value] = event;
  log::info("DEBUG execution_report_cancel={}"sv, const_cast<value_type &>(value));
}

void OrderEntry::operator()(Trace<cme_ilink::ExecutionReportPendingCancel564> const &event) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &[trace_info, value] = event;
  log::info("DEBUG execution_report_pending_cancel={}"sv, const_cast<value_type &>(value));
}

void OrderEntry::operator()(Trace<cme_ilink::ExecutionReportPendingReplace565> const &event) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &[trace_info, value] = event;
  log::info("DEBUG execution_report_pending_replace={}"sv, const_cast<value_type &>(value));
}

// order

void OrderEntry::operator()(Trace<cme_ilink::OrderCancelReject535> const &event) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &[trace_info, value] = event;
  log::info("order_cancel_reject={}"sv, const_cast<value_type &>(value));
}

void OrderEntry::operator()(Trace<cme_ilink::OrderCancelReplaceReject536> const &event) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &[trace_info, value] = event;
  log::info("DEBUG order_cancel_replace_reject={}"sv, const_cast<value_type &>(value));
}

// security definition

void OrderEntry::operator()(Trace<cme_ilink::SecurityDefinitionResponse561> const &event) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &[trace_info, value] = event;
  log::info("DEBUG security_definition_response={}"sv, const_cast<value_type &>(value));
}

void OrderEntry::operator()(io::net::ConnectionManager::Connected const &) {
  send_negotiate();
  (*this)(ConnectionStatus::LOGIN_SENT);
}

void OrderEntry::operator()(io::net::ConnectionManager::Disconnected const &) {
  outbound_ = {};
  inbound_ = {};
  next_heartbeat_ = {};
  (*this)(ConnectionStatus::DISCONNECTED);
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
        .account = account_.get_name(),
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

//

uint32_t OrderEntry::peek_next_seq_num() {
  return (outbound_.msg_seq_num + 1) % 1000000000;
}

uint32_t OrderEntry::fetch_next_seq_num() {
  auto result = peek_next_seq_num();
  outbound_.msg_seq_num = result;
  return result;
}

template <typename T>
void OrderEntry::send(T const &value) {
  auto message = value.encode(encode_buffer_2_);
  uint16_t length = utils::safe_cast{std::size(message) + 4};
  struct SOFH final {
    uint16_t message_length;
    uint8_t dummy_1 = 0xFE;
    uint8_t dummy_2 = 0xCA;
  } sofh = {
      .message_length = core::host_to_little_endian(length),
  };
  static_assert(sizeof(SOFH) == 4);
  auto data = std::array<std::span<std::byte const>, 2>{{
      {reinterpret_cast<std::byte const *>(&sofh), sizeof(sofh)},
      message,
  }};
  log::info(R"(DEBUG message="{}{}")"sv, debug::hex::Message{data[0]}, debug::hex::Message{data[1]});
  log::info<5>(R"(Sending message="{}{}")"sv, debug::hex::Message{data[0]}, debug::hex::Message{data[1]});
  (*connection_manager_).send(data);
  next_heartbeat_ = clock::get_system() + KEEP_ALIVE_INTERVAL;
}

void OrderEntry::send_negotiate() {
  auto now = clock::get_realtime();
  uuid_ = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::microseconds>(now).count());
  auto canonical_message = tools::CanonicalMessage{
      .request_timestamp = now,
      .uuid = uuid_,
      .session = account_.get_login(),
      .firm_id = shared_.settings.ilink.firm,
      .trading_system_name = {},     // note!
      .trading_system_version = {},  // note!
      .trading_system_vendor = {},   // note!
      .next_seq_no = {},             // note!
      .keep_alive_interval = {},     // note!
  };
  auto hmac_signature = account_.create_signature(canonical_message);
  auto negotiate = ilink::Negotiate{
      .hmac_signature = hmac_signature,
      .access_key_id = account_.get_password(),
      .uuid = canonical_message.uuid,
      .request_timestamp = canonical_message.request_timestamp,
      .session = canonical_message.session,
      .firm = canonical_message.firm_id,
  };
  log::info("DEBUG negotiate={}"sv, negotiate);
  send(negotiate);
}

void OrderEntry::send_establish() {
  auto now = clock::get_realtime();
  auto canonical_message = tools::CanonicalMessage{
      .request_timestamp = now,
      .uuid = uuid_,
      .session = account_.get_login(),
      .firm_id = shared_.settings.ilink.firm,
      .trading_system_name = ROQ_PACKAGE_NAME,
      .trading_system_version = ROQ_BUILD_VERSION,
      .trading_system_vendor = "ROQ"sv,
      .next_seq_no = peek_next_seq_num(),
      .keep_alive_interval = KEEP_ALIVE_INTERVAL,
  };
  auto hmac_signature = account_.create_signature(canonical_message);
  auto establish = ilink::Establish{
      .hmac_signature = hmac_signature,
      .access_key_id = account_.get_password(),
      .trading_system_name = canonical_message.trading_system_name,
      .trading_system_version = canonical_message.trading_system_version,
      .trading_system_vendor = canonical_message.trading_system_vendor,
      .uuid = canonical_message.uuid,
      .request_timestamp = canonical_message.request_timestamp,
      .next_seq_no = canonical_message.next_seq_no,
      .session = canonical_message.session,
      .firm = canonical_message.firm_id,
      .keep_alive_interval = canonical_message.keep_alive_interval,
  };
  log::info("DEBUG establish={}"sv, establish);
  send(establish);
}

void OrderEntry::send_sequence() {
  auto sequence = ilink::Sequence{
      .uuid = uuid_,
      .next_seq_no = peek_next_seq_num(),
      .fault_tolerance_indicator = cme_ilink::FTI::Primary,
      .keep_alive_interval_lapsed = cme_ilink::KeepAliveLapsed::NotLapsed,
  };
  log::info("DEBUG sequence={}"sv, sequence);
  send(sequence);
}

void OrderEntry::send_terminate() {
  auto terminate = ilink::Terminate{};
  log::info("DEBUG terminate={}"sv, terminate);
  send(terminate);
}

void OrderEntry::send_party_details_list_request() {
  auto now = clock::get_realtime();
  auto party_details_list_request = ilink::PartyDetailsListRequest{
      .party_details_list_req_id = 1,  // XXX
      .sending_time_epoch = now,
      .seq_num = fetch_next_seq_num(),
  };
  log::info("DEBUG party_details_list_request={}"sv, party_details_list_request);
  send(party_details_list_request);
}

void OrderEntry::send_party_details_definition_request() {
  auto now = clock::get_realtime();
  std::array<ilink::PartyDetailsDefinitionRequest::PartyDetails, 3> party_details{{
      {
          .party_detail_id = "ROQ"sv,
          .party_detail_role = cme_ilink::PartyDetailRole::ExecutingFirm,
      },
      {
          .party_detail_id = "HANS"sv,
          .party_detail_role = cme_ilink::PartyDetailRole::CustomerAccount,
      },
      {
          .party_detail_id = "CSET"sv,
          .party_detail_role = cme_ilink::PartyDetailRole::Operator,
      },
  }};
  auto party_details_definition_request = ilink::PartyDetailsDefinitionRequest{
      .party_details_list_req_id = {},  // note!
      .sending_time_epoch = now,
      .list_update_action = cme_ilink::ListUpdAct::Add,
      .seq_num = fetch_next_seq_num(),
      .memo = {},
      .avg_px_group_id = {},
      .self_match_prevention_id = {},
      .cmta_giveup_cd = cme_ilink::CmtaGiveUpCD::GiveUp,
      .cust_order_capacity = cme_ilink::CustOrderCapacity::Membertradingfortheirownaccount,
      .clearing_account_type = cme_ilink::ClearingAcctType::Firm,
      .self_match_prevention_instruction = cme_ilink::SMPI::CancelNewest,
      .avg_px_indicator = cme_ilink::AvgPxInd::NoAveragePricing,
      .clearing_trade_price_type = cme_ilink::SLEDS::TradeClearingatExecutionPrice,
      .cust_order_handling_inst = cme_ilink::CustOrdHandlInst::AlgoEngine,
      .executor = {},
      .idm_short_code = {},
      .no_party_details = party_details,
  };
  log::info("DEBUG party_details_definition_request={}"sv, party_details_definition_request);
  send(party_details_definition_request);
}

void OrderEntry::send_security_definition_request() {
  auto security_definition_request = ilink::SecurityDefinitionRequest{};
  log::info("DEBUG security_definition_request={}"sv, security_definition_request);
  send(security_definition_request);
}

void OrderEntry::send_order_mass_status_request() {
  auto order_mass_status_request = ilink::OrderMassStatusRequest{};
  log::info("DEBUG order_mass_status_request={}"sv, order_mass_status_request);
  send(order_mass_status_request);
}

void OrderEntry::send_new_order_single(CreateOrder const &create_order) {
  auto new_order_single = ilink::NewOrderSingle{};
  log::info("DEBUG new_order_single={}"sv, new_order_single);
  send(new_order_single);
}

void OrderEntry::send_order_cancel_request(CancelOrder const &cancel_order) {
  auto order_cancel_request = ilink::OrderCancelRequest{};
  log::info("DEBUG order_cancel_request={}"sv, order_cancel_request);
  send(order_cancel_request);
}

void OrderEntry::send_order_cancel_replace_request(ModifyOrder const &modify_order) {
  auto order_cancel_replace_request = ilink::OrderCancelReplaceRequest{};
  log::info("DEBUG order_cancel_replace_request={}"sv, order_cancel_replace_request);
  send(order_cancel_replace_request);
}

void OrderEntry::send_order_mass_action_request(CancelAllOrders const &cancel_all_orders) {
  auto order_mass_action_request = ilink::OrderMassActionRequest{};
  log::info("DEBUG order_mass_action_request={}"sv, order_mass_action_request);
  send(order_mass_action_request);
}

}  // namespace cme
}  // namespace roq
