/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/deribit/order_entry.hpp"

#include <algorithm>
#include <utility>

#include "roq/mask.hpp"

#include "roq/utils/safe_cast.hpp"
#include "roq/utils/update.hpp"

#include "roq/debug/fix/message.hpp"
#include "roq/debug/hex/message.hpp"

#include "roq/core/back_emplacer.hpp"

#include "roq/core/metrics/factory.hpp"

#include "roq/core/fix/utils.hpp"

#include "roq/deribit/common.hpp"

#include "roq/deribit/flags/common.hpp"
#include "roq/deribit/flags/config.hpp"
#include "roq/deribit/flags/fix.hpp"

#include "roq/deribit/fix/utils.hpp"

// business (outbound)
#include "roq/deribit/fix/new_order_single.hpp"
#include "roq/deribit/fix/order_cancel_replace_request.hpp"
#include "roq/deribit/fix/order_cancel_request.hpp"
#include "roq/deribit/fix/order_mass_cancel_request.hpp"
#include "roq/deribit/fix/order_mass_status_request.hpp"
#include "roq/deribit/fix/request_for_positions.hpp"

using namespace std::literals;

namespace roq {
namespace deribit {

namespace {
auto const LOGOUT_RESPONSE = "LOGOUT"sv;

const Mask SUPPORTS{
    SupportType::CREATE_ORDER,
    SupportType::MODIFY_ORDER,
    SupportType::CANCEL_ORDER,
    SupportType::ORDER_ACK,
    SupportType::ORDER,
    SupportType::TRADE,
    SupportType::POSITION,
};

auto create_name(auto const &stream_id, auto const &security) {
  auto name = "om"sv;
  return fmt::format("{}:{}:{}"sv, stream_id, name, security.get_account());
}

struct create_metrics final : public core::metrics::Factory {
  explicit create_metrics(std::string_view const &group, std::string_view const &function)
      : core::metrics::Factory(server::Flags::name(), group, function) {}
};

auto create_connection_factory(auto &context) {
  auto uri = flags::FIX::fix_uri();
  io::net::ConnectionFactory::Config config{
      .uris = {&uri, 1},
      .validate_certificate = server::Flags::net_tls_validate_certificate(),
  };
  return io::net::ConnectionFactory::create(context, config);
}

auto create_connection_manager(auto &handler, auto &connection_factory) {
  io::net::ConnectionManager::Config config{
      .always_reconnect = true,
      .connection_timeout = server::Flags::net_connection_timeout(),
      .disconnect_on_idle_timeout = {},
  };
  return io::net::ConnectionManager::create(handler, connection_factory, config);
}

template <typename T>
void emplace(Fill &result, const T &value) {
  new (&result) Fill{
      .external_trade_id = value.fill_exec_id,
      .quantity = value.fill_qty,
      .price = value.fill_px,
      .liquidity = {},
  };
}
}  // namespace

OrderEntry::OrderEntry(Handler &handler, io::Context &context, uint16_t stream_id, Security &security, Shared &shared)
    : handler_(handler), stream_id_(stream_id), name_(create_name(stream_id_, security)),
      connection_factory_(create_connection_factory(context)),
      connection_manager_(create_connection_manager(*this, *connection_factory_)),
      encode_buffer_(flags::Common::encode_buffer_size()), decode_buffer_(flags::Common::decode_buffer_size()),
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
      security_(security), shared_(shared),
      download_(flags::FIX::fix_request_timeout(), [this](auto state) { return download(state); }) {
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
  if (last_logon_or_heartbeat_.count() && flags::FIX::fix_request_timeout().count() &&
      (event.value.now - last_logon_or_heartbeat_) > flags::FIX::fix_request_timeout()) {
    log::warn("*** DETECTED TIMEOUT ***"sv);
    log::info("closing connection"sv);
    (*connection_manager_).close();
  } else {
    if (ready_) {
      if (test_disconnect_time_.count() && test_disconnect_time_ < event.value.now) [[unlikely]] {
        if (flags::FIX::fix_test_order_disconnect().count()) {
          log::warn("*** TEST: DISCONNECT ***"sv);
          log::info("closing connection"sv);
          (*connection_manager_).close();
        }
      } else {
        if (next_heartbeat_ <= event.value.now) {
          assert(flags::FIX::fix_ping_freq().count() > 0);
          next_heartbeat_ = event.value.now + flags::FIX::fix_ping_freq();
          send_test_request(core::clock::GetSystem());
        }
      }
    } else {
      if (test_logon_time_.count() && test_logon_time_ < event.value.now) {
        if (flags::FIX::fix_test_order_logon().count())
          log::warn("*** TEST: LOGON ***"sv);
        test_logon_time_ = {};
        send_logon();
        (*this)(ConnectionStatus::LOGIN_SENT);
      }
    }
  }
}

uint16_t OrderEntry::operator()(
    Event<CreateOrder> const &event, oms::Order const &order, std::string_view const &request_id) {
  if (!ready())
    throw oms::NotReady("not ready"sv);
  auto &[message_info, create_order] = event;
  if (std::isfinite(create_order.stop_price))
    throw RuntimeError("stop_price not supported"sv);
  if (std::isfinite(create_order.max_show_quantity))
    throw RuntimeError("max_show_quantity not supported"sv);
  auto side = core::fix::map(create_order.side);
  auto exec_inst = fix::map(create_order.execution_instructions);
  auto ord_type = core::fix::map(create_order.order_type);
  auto time_in_force = core::fix::map(create_order.time_in_force);
  core::stack::Buffer<char, sizeof(RequestId)> buffer;
  fmt::format_to(std::back_inserter(buffer), "roq-{}-{}"sv, message_info.source, create_order.order_id);
  std::string_view deribit_label(std::data(buffer), std::size(buffer));
  fix::NewOrderSingle new_order_single{
      .cl_ord_id = request_id,
      .side = side,
      .order_qty = {create_order.quantity, order.quantity_decimals},
      .price = {create_order.price, order.price_decimals},
      .symbol = create_order.symbol,
      .exec_inst = exec_inst,
      .ord_type = ord_type,
      .time_in_force = time_in_force,
      .deribit_label = deribit_label,
      .deribit_adv_order_type = '\0',
  };
  auto msg_seq_num = send(new_order_single);
  // XXX HANS EXPERIMENTAL -- it's a leak / currently no way to clean up
  log::info(R"(DEBUG: msg_seq_num={} --> request_id="{}")"sv, msg_seq_num, request_id);
  msg_seq_num_to_request_id_.emplace(msg_seq_num, request_id);
  return stream_id_;
}

uint16_t OrderEntry::operator()(
    Event<ModifyOrder> const &event,
    oms::Order const &order,
    std::string_view const &request_id,
    [[maybe_unused]] std::string_view const &previous_request_id) {
  if (!ready())
    throw oms::NotReady("not ready"sv);
  auto const &modify_order = event.value;
  auto side = core::fix::map(order.side);
  auto ord_type = core::fix::map(order.order_type);
  fix::OrderCancelReplaceRequest order_cancel_replace_request{
      .orig_cl_ord_id = order.external_order_id,
      .cl_ord_id = request_id,
      .transact_time = utils::safe_cast(order.update_time_utc),
      .side = side,
      .order_qty = {modify_order.quantity, order.quantity_decimals},
      .ord_type = ord_type,
      .price = {modify_order.price, order.price_decimals},
      .symbol = order.symbol,
      .exec_inst = {},
  };
  auto msg_seq_num = send(order_cancel_replace_request);
  // XXX HANS EXPERIMENTAL -- it's a leak / currently no way to clean up
  log::info(R"(DEBUG: msg_seq_num={} --> request_id="{}")"sv, msg_seq_num, request_id);
  msg_seq_num_to_request_id_.emplace(msg_seq_num, request_id);
  return stream_id_;
}

uint16_t OrderEntry::operator()(
    Event<CancelOrder> const &,
    oms::Order const &order,
    std::string_view const &request_id,
    [[maybe_unused]] std::string_view const &previous_request_id) {
  if (!ready())
    throw oms::NotReady("not ready"sv);
  fix::OrderCancelRequest order_cancel_request{
      .cl_ord_id = request_id,
      .orig_cl_ord_id = order.external_order_id,
  };
  auto msg_seq_num = send(order_cancel_request);
  // XXX HANS EXPERIMENTAL -- it's a leak / currently no way to clean up
  log::info(R"(DEBUG: msg_seq_num={} --> request_id="{}")"sv, msg_seq_num, request_id);
  msg_seq_num_to_request_id_.emplace(msg_seq_num, request_id);
  return stream_id_;
}

uint16_t OrderEntry::operator()(Event<CancelAllOrders> const &event, std::string_view const &request_id) {
  if (ready()) {
    fix::OrderMassCancelRequest order_mass_cancel_request{
        .cl_ord_id = request_id,
        .mass_cancel_request_type = core::fix::MassCancelRequestType::CANCEL_ALL_ORDERS,
        .security_type = {},
        .symbol = {},
        .currency = {},
    };
    send(order_mass_cancel_request);
  } else {
    auto &[message_info, cancel_all_orders] = event;
    log::warn(R"(*** NOT CONNECTED! UNABLE TO CANCEL ALL ORDERS FOR ACCOUNT="{}")"sv, cancel_all_orders.account);
  }
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

void OrderEntry::operator()(io::net::ConnectionManager::Connected const &) {
  assert(test_logon_time_.count() == 0);
  auto now = core::clock::GetSystem();
  test_logon_time_ = now + flags::FIX::fix_test_order_logon();
  if (flags::FIX::fix_test_order_disconnect().count())
    test_disconnect_time_ = now + flags::FIX::fix_test_order_disconnect();
}

void OrderEntry::operator()(io::net::ConnectionManager::Disconnected const &) {
  ++counter_.disconnect;
  outbound_ = {};
  inbound_ = {};
  ready_ = false;
  next_heartbeat_ = {};
  (*this)(ConnectionStatus::DISCONNECTED);
  download_.reset();
  // test
  test_logon_time_ = {};
  test_disconnect_time_ = {};
}

void OrderEntry::operator()(io::net::ConnectionManager::Read const &) {
  auto buffer = (*connection_manager_).buffer();
  size_t total_bytes = 0;
  while (!std::empty(buffer)) {
    auto trace_info = server::create_trace_info();
    auto bytes = core::fix::Reader<FIX_VERSION>::dispatch(
        [&](core::fix::Message const &message) {
          try {
            check(message.header);
            Trace event{trace_info, message};
            parse(event);
          } catch (std::exception &) {
            log::warn("{}"sv, debug::fix::Message{buffer});
#ifndef NDEBUG
            log::warn("{}"sv, debug::hex::Message{buffer});
#endif
            if (!flags::FIX::fix_continue_from_parse_exception()) [[likely]] {
              throw;
            } else {
              log::error("Message could not be parsed. PLEASE REPORT!"sv);
            }
          }
        },
        buffer,
        [](auto &message) {
          if (flags::FIX::fix_debug())
            log::info("{}"sv, debug::fix::Message{message});
        });
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
    auto trace_info = server::create_trace_info();
    const StreamStatus stream_status{
        .stream_id = stream_id_,
        .account = security_.get_account(),
        .supports = SUPPORTS,
        .transport = Transport::TCP,
        .protocol = Protocol::FIX,
        .encoding = {Encoding::FIX},
        .priority = Priority::PRIMARY,
        .connection_status = status_,
    };
    log::info("stream_status={}"sv, stream_status);
    create_trace_and_dispatch(handler_, trace_info, stream_status);
  }
}

void OrderEntry::send_logon() {
  auto ping_freq = std::chrono::duration_cast<std::chrono::seconds>(flags::FIX::fix_ping_freq());
  auto now = core::clock::GetRealTime<std::chrono::milliseconds>();
  auto raw_data = security_.create_raw_data(now);
  auto password = security_.create_password(raw_data);
  fix::Logon logon{
      .heart_bt_int = static_cast<uint16_t>(ping_freq.count()),
      .raw_data_length = static_cast<uint32_t>(std::size(raw_data)),
      .raw_data = raw_data,
      .username = security_.get_access_key(),
      .password = password,
      .use_wordsafe_tags = false,
      .cancel_on_disconnect = flags::FIX::fix_cancel_on_disconnect(),
      .deribit_app_id = {},
      .deribit_app_sig = {},
      .deribit_sequential = false,
      .unsubscribe_execution_reports = false,
  };
  send(logon);
  last_logon_or_heartbeat_ = core::clock::GetSystem();
}

void OrderEntry::send_logout(std::string_view const &text) {
  fix::Logout logout{
      .text = text,
  };
  send(logout);
}

void OrderEntry::send_heartbeat(std::string_view const &test_req_id) {
  fix::Heartbeat heartbeat{
      .test_req_id = test_req_id,
  };
  send(heartbeat);
}

void OrderEntry::send_test_request(std::chrono::nanoseconds now) {
  // request_id is current time
  stack_buffer_.clear();
  core::charconv::to_string(std::back_inserter(stack_buffer_), now.count());
  auto request_id = std::string_view(std::data(stack_buffer_), std::size(stack_buffer_));
  fix::TestRequest test_request{
      .test_req_id = request_id,
  };
  send(test_request);
  if (!last_logon_or_heartbeat_.count())
    last_logon_or_heartbeat_ = now;
}

uint32_t OrderEntry::download(OrderEntryState state) {
  switch (state) {
    using enum OrderEntryState;
    case UNDEFINED:
      assert(false);
      break;
    case POSITIONS:
      subscribe_positions();
      return 1;
    case ORDERS:
      download_orders();
      return 1;  // note! first report includes the true number of reports
    case DONE:
      (*this)(ConnectionStatus::READY);
      assert(!ready_);
      ready_ = true;
      return {};
  }
  assert(false);
  return {};
}

void OrderEntry::subscribe_positions() {
  auto request_id = shared_.next_request_id();
  fix::RequestForPositions request_for_positions{
      .pos_req_id = request_id,
      .pos_req_type = roq::core::fix::PosReqType::POSITIONS,
      .subscription_request_type = roq::core::fix::SubscriptionRequestType::SNAPSHOT_UPDATES,
      .currency = {},
  };
  send(request_for_positions);
}

void OrderEntry::download_orders() {
  auto request_id = shared_.next_request_id();
  fix::OrderMassStatusRequest order_mass_status_request{
      .mass_status_req_id = request_id,
      .mass_status_req_type = core::fix::MassStatusReqType::ORDERS,
  };
  send(order_mass_status_request);
}

void OrderEntry::parse(Trace<core::fix::Message const> const &event) {
  profile_.parse([&]() { parse_helper(event); });
}

void OrderEntry::parse_helper(Trace<core::fix::Message const> const &event) {
  // auto &[trace_info, message] = event;
  auto &trace_info = event.trace_info;
  auto &message = event.value;
  core::fix::Buffer buffer(decode_buffer_);
  switch (message.header.msg_type) {
    using enum core::fix::MsgType;
    // session
    case HEARTBEAT: {
      auto const heartbeat = fix::Heartbeat::create(message);
      create_trace_and_dispatch(*this, trace_info, heartbeat, message.header);
      return;
    }
    case LOGON: {
      auto const logon = fix::Logon::create(message);
      create_trace_and_dispatch(*this, trace_info, logon, message.header);
      return;
    }
    case LOGOUT: {
      auto const logout = fix::Logout::create(message);
      create_trace_and_dispatch(*this, trace_info, logout, message.header);
      return;
    }
    case RESEND_REQUEST: {
      auto const resend_request = fix::ResendRequest::create(message);
      create_trace_and_dispatch(*this, trace_info, resend_request, message.header);
      return;
    }
    case TEST_REQUEST: {
      auto const test_request = fix::TestRequest::create(message);
      create_trace_and_dispatch(*this, trace_info, test_request, message.header);
      return;
    }
    // ...
    case POSITION_REPORT: {
      profile_.position_report([&]() {
        const auto position_report = fix::PositionReport::create(message, buffer);
        create_trace_and_dispatch(*this, trace_info, position_report, message.header);
      });
      return;
    }
    case EXECUTION_REPORT: {
      profile_.execution_report([&]() {
        const auto execution_report = fix::ExecutionReport::create(message, buffer);
        create_trace_and_dispatch(*this, trace_info, execution_report, message.header);
      });
      return;
    }
    case ORDER_CANCEL_REJECT: {
      profile_.order_cancel_reject([&]() {
        const auto order_cancel_reject = fix::OrderCancelReject::create(message);
        create_trace_and_dispatch(*this, trace_info, order_cancel_reject, message.header);
      });
      return;
    }
    case REJECT: {
      profile_.reject([&]() {
        const auto reject = fix::Reject::create(message);
        create_trace_and_dispatch(*this, trace_info, reject, message.header);
      });
      return;
    }
    case ORDER_MASS_CANCEL_REPORT: {
      profile_.order_mass_cancel_report([&]() {
        const auto order_mass_cancel_report = fix::OrderMassCancelReport::create(message, buffer);
        create_trace_and_dispatch(*this, trace_info, order_mass_cancel_report, message.header);
      });
      return;
    }
    default:
      break;
  }
  log::warn("Unexpected msg_type={}"sv, message.header.msg_type);
}

void OrderEntry::operator()(Trace<fix::Heartbeat const> const &event, core::fix::Header const &header) {
  auto now = core::clock::GetSystem();
  auto &[trace_info, heartbeat] = event;
  log::info<3>("event={{header={}, heartbeat={}}}"sv, header, heartbeat);
  last_logon_or_heartbeat_ = {};
  if (!std::empty(heartbeat.test_req_id)) {
    auto send_time = std::chrono::nanoseconds{core::from_chars<uint64_t>(heartbeat.test_req_id)};
    auto latency = (now - send_time) / 2;  // 1-way
    const ExternalLatency external_latency{
        .stream_id = stream_id_,
        .account = security_.get_account(),
        .latency = latency,
    };
    create_trace_and_dispatch(handler_, trace_info, external_latency);
    latency_.ping.update(latency);
  }
}

void OrderEntry::operator()(Trace<fix::Logon const> const &event, core::fix::Header const &header) {
  auto &[trace_info, logon] = event;
  log::info<2>("event={{header={}, logon={}}}"sv, header, logon);
  last_logon_or_heartbeat_ = {};
  (*this)(ConnectionStatus::DOWNLOADING);
  download_.begin();
}

void OrderEntry::operator()(Trace<fix::Logout const> const &event, core::fix::Header const &header) {
  auto &[trace_info, logout] = event;
  log::warn("event={{header={}, logout={}}}"sv, header, logout);
  ready_ = false;
  // note! mandated, must send a logout response
  send_logout(LOGOUT_RESPONSE);
  log::info("closing connection"sv);
  (*connection_manager_).close();
}

void OrderEntry::operator()(Trace<fix::ResendRequest const> const &event, core::fix::Header const &header) {
  auto &[trace_info, resend_request] = event;
  log::warn("event={{header={}, resend_request={}}}"sv, header, resend_request);
  log::info("closing connection"sv);
  (*connection_manager_).close();
}

void OrderEntry::operator()(Trace<fix::TestRequest const> const &event, core::fix::Header const &header) {
  auto &[trace_info, test_request] = event;
  log::info<1>("event={{header={}, test_request={}}}"sv, header, test_request);
  send_heartbeat(test_request.test_req_id);
}

void OrderEntry::operator()(Trace<fix::PositionReport const> const &event, core::fix::Header const &header) {
  auto &[trace_info, position_report] = event;
  log::info<2>("event={{header={}, position_report={}}}"sv, header, position_report);
  for (size_t i = 0; i < std::size(position_report.no_positions); ++i) {
    auto is_last = std::size(position_report.no_positions) == (i + 1);
    auto &position_qty = position_report.no_positions[i];
    auto long_quantity = std::max(0.0, position_qty.long_qty);
    auto short_quantity = std::max(0.0, position_qty.short_qty);
    const PositionUpdate position_update{
        .stream_id = stream_id_,
        .account = security_.get_account(),
        .exchange = flags::Config::exchange(),
        .symbol = position_qty.symbol,
        .external_account = {},
        .long_quantity = long_quantity,
        .short_quantity = short_quantity,
        .long_quantity_begin = NaN,
        .short_quantity_begin = NaN,
    };
    create_trace_and_dispatch(handler_, trace_info, position_update, is_last);
  }
  download_.check_relaxed(OrderEntryState::POSITIONS);
}

namespace {
// execution_report:
//
// mass_status_req_type  what
// ----------------------------------------
//   ORDERS                begin download
//   *                     order update
//
// exec_type       ord_status          what
// ------------------------------------------------------------------
//   REJECTED        *                   ack failure
//   CANCELED        *                   ack success + order update
//   ORDER_STATUS    NEW                 ack success + order update (create + modify)
//   ORDER_STATUS    PARTIALLY_FILLED    order update
//   ORDER_STATUS    FILLED              order update
//   ORDER_STATUS    CANCELED            ack success

RequestType compute_request_type(auto const exec_type, auto const ord_status) {
  switch (exec_type) {
    using enum core::fix::ExecType;
    case REJECTED:
      return {};  // any
    case CANCELED:
      return RequestType::CANCEL_ORDER;
    case ORDER_STATUS:
      switch (ord_status) {
        using enum core::fix::OrdStatus;
        case NEW:
        case PARTIALLY_FILLED:
          return {};  // create or modify
        case CANCELED:
          return RequestType::CANCEL_ORDER;
          break;
        default:
          break;
      }
    default:
      break;
  }
  return {};
}

RequestStatus compute_request_status(auto const exec_type, auto const ord_status) {
  switch (exec_type) {
    using enum core::fix::ExecType;
    case REJECTED:
      return RequestStatus::REJECTED;
    case CANCELED:
      return RequestStatus::ACCEPTED;
    case ORDER_STATUS:
      switch (ord_status) {
        using enum core::fix::OrdStatus;
        case NEW:
        case PARTIALLY_FILLED:
        case FILLED:
        case CANCELED:
          return RequestStatus::ACCEPTED;
        default:
          break;
      }
      break;
    default:
      break;
  }
  return {};
}

auto find_liquidity_ind(auto const &fills) {
  auto result = core::fix::FillLiquidityInd::UNDEFINED;
  auto found = false;
  for (auto &item : fills) {
    if (item.fill_liquidity_ind != core::fix::FillLiquidityInd::UNDEFINED) {
      if (!found) {
        result = item.fill_liquidity_ind;
        found = true;
      } else if (item.fill_liquidity_ind != result) {
        result = core::fix::FillLiquidityInd::UNDEFINED;
        break;
      }
    }
  }
  return result;
}

// note!
//   last traded is expected (downstream) to be the sum of all fills for this update
//   Deribit reports only the *last* fill, but includes all fills as well
//   we will therefore replace these values, when possible
std::pair<double, double> compute_last_traded(
    auto const last_traded_quantity, auto const last_traded_price, auto const &fills) {
  if (std::empty(fills))
    return {last_traded_quantity, last_traded_price};
  double sum_quantity = 0.0, sum_quantity_price = 0.0;
  for (auto &item : fills) {
    sum_quantity += item.fill_qty;
    sum_quantity_price += item.fill_qty * item.fill_px;
  }
  auto average_price = utils::is_zero(sum_quantity) ? NaN : sum_quantity_price / sum_quantity;
  return {sum_quantity, average_price};
}

UpdateType compute_update_type(auto const &download) {
  if (download.state() != OrderEntryState::ORDERS)
    return UpdateType::INCREMENTAL;
  return UpdateType::SNAPSHOT;
}
}  // namespace

void OrderEntry::operator()(Trace<fix::ExecutionReport const> const &event, core::fix::Header const &header) {
  // auto &[trace_info, execution_report] = event;  // XXX clang13
  auto &trace_info = event.trace_info;
  auto &execution_report = event.value;
  log::info<2>("event={{header={}, execution_report={}}}"sv, header, execution_report);
  // log::debug("execution_report={}"sv, execution_report);
  // download begin?
  switch (execution_report.mass_status_req_type) {
    using enum core::fix::MassStatusReqType;
    case UNDEFINED:
      assert(std::empty(execution_report.mass_status_req_id));
      break;
    case ORDERS: {
      auto count = execution_report.tot_num_reports;
      log::info<1>(
          R"(Downloading {} execution reports (request_id="{}")"sv, count, execution_report.mass_status_req_id);
      download_.update(OrderEntryState::ORDERS, count);
      return;  // this is not an ordinary execution report
    }
    default:
      log::fatal(
          R"(Unexpected: mass_status_req_type={}, mass_status_req_id="{}")"sv,
          execution_report.mass_status_req_type,
          execution_report.mass_status_req_id);
      break;
  }
  // convenience
  auto exec_type = execution_report.exec_type;
  auto ord_status = execution_report.ord_status;
  // special case: partial fill can overlap cancel request (#143)
  if (!flags::Common::disable_deribit_143()) {
    if (exec_type == core::fix::ExecType::CANCELED && ord_status == core::fix::OrdStatus::CANCELED) {
      log::warn<1>("Drop execution report due to FIX compliance"sv);
      return;
    }
  }
  auto side = core::fix::map(execution_report.side);
  auto order_status = core::fix::map(execution_report.ord_status);
  auto order_type = core::fix::map(execution_report.ord_type);
  auto liquidity_ind = find_liquidity_ind(execution_report.no_fills);
  auto last_liquidity = core::fix::map(liquidity_ind);
  auto request_type = compute_request_type(exec_type, ord_status);
  auto request_status = compute_request_status(exec_type, ord_status);
  auto error = fix::map_error(execution_report.text);
  auto [last_traded_quantity, last_traded_price] =
      compute_last_traded(execution_report.last_qty, execution_report.last_px, execution_report.no_fills);
  auto update_type = compute_update_type(download_);
  // note!
  // we have very little information to match requests as we can't rewrite ClOrdID
  // - create and modify both have exec_type=ORDER_STATUS and ord_status=NEW
  // - reject has nothing
  oms::Response response{
      .type = request_type,
      .origin = Origin::EXCHANGE,
      .status = request_status,
      .error = error,
      .text = execution_report.text,
      .version = {},
      .request_id = {},
      .quantity = execution_report.order_qty,
      .price = execution_report.price,
  };
  oms::OrderUpdate order_update{
      .account = security_.get_account(),
      .exchange = flags::Config::exchange(),
      .symbol = execution_report.symbol,
      .side = side,
      .position_effect = {},
      .max_show_quantity = execution_report.max_show,
      .order_type = order_type,
      .time_in_force = {},
      .execution_instructions = {},
      .order_template = {},
      .create_time_utc = {},
      .update_time_utc = execution_report.transact_time,
      .external_account = {},
      .external_order_id = execution_report.order_id,
      .status = order_status,
      .quantity = execution_report.order_qty,
      .price = execution_report.price,
      .stop_price = execution_report.stop_px,
      .remaining_quantity = execution_report.leaves_qty,
      .traded_quantity = execution_report.cum_qty,
      .average_traded_price = execution_report.avg_px,
      .last_traded_quantity = last_traded_quantity,
      .last_traded_price = last_traded_price,
      .last_liquidity = last_liquidity,
      .update_type = update_type,
  };
  if (shared_.update_order(
          execution_report.orig_cl_ord_id,  // note! *always* from create order (can't rewrite)
          stream_id_,
          trace_info,
          response,
          order_update,
          [&](auto &order) {
            // log::debug("found order={}"sv, order);
            core::back_emplacer fills(shared_.fills);
            for (auto &item : execution_report.no_fills) {
              fills.emplace_back([&](auto &result) { emplace(result, item); });
            }
            if (!std::empty(fills)) {
              const TradeUpdate trade_update{
                  .stream_id = stream_id_,
                  .account = order.account,
                  .order_id = order.order_id,
                  .exchange = order.exchange,
                  .symbol = order.symbol,
                  .side = order.side,
                  .position_effect = order.position_effect,
                  .create_time_utc = execution_report.transact_time,
                  .update_time_utc = execution_report.transact_time,
                  .external_account = order.external_account,
                  .external_order_id = order.external_order_id,
                  .fills = fills,
                  .routing_id = order.routing_id,
                  .update_type = update_type,
              };
              create_trace_and_dispatch(handler_, trace_info, trade_update, true, order.user_id);
            }
          })) {
  } else {
    auto external = std::empty(execution_report.deribit_label);
    if (external)
      log::warn("*** EXTERNAL ORDER ***"sv);
    else
      log::warn("*** UNKNOWN INTERNAL ORDER ***"sv);
    log::warn("execution_report={}"sv, execution_report);
  }
  // download end?
  download_.check_relaxed(OrderEntryState::ORDERS);
}

void OrderEntry::operator()(Trace<fix::OrderCancelReject const> const &event, core::fix::Header const &header) {
  // auto &[trace_info, order_cancel_reject] = event;  // XXX clang13
  auto &trace_info = event.trace_info;
  auto &order_cancel_reject = event.value;
  log::warn<1>("event={{header={}, order_cancel_reject={}}}"sv, header, order_cancel_reject);
  auto error = fix::map_error(order_cancel_reject.text);
  oms::Response response{
      .type = {},  // modify or cancel
      .origin = Origin::EXCHANGE,
      .status = RequestStatus::REJECTED,
      .error = error,
      .text = order_cancel_reject.text,
      .version = {},
      .request_id = {},
      .quantity = NaN,
      .price = NaN,
  };
  if (shared_.update_order(order_cancel_reject.orig_cl_ord_id, stream_id_, trace_info, response, [&](auto &order) {
        // log::debug("found order={}"sv, order);
        auto status = core::fix::map(order_cancel_reject.ord_status);
        if (status != order.status) {
          log::warn("Unexpected: order status received={}, expected={}"sv, status, order.status);
        }
      })) {
  } else {
    log::warn("*** EXTERNAL ORDER ***"sv);
    log::warn("order_cancel_reject={}"sv, order_cancel_reject);
  }
}

namespace {
RequestType message_type_to_request_type(auto const msg_type) {
  switch (msg_type) {
    using enum core::fix::MsgType;
    case NEW_ORDER_SINGLE:
      return RequestType::CREATE_ORDER;
    case ORDER_CANCEL_REPLACE_REQUEST:
      return RequestType::MODIFY_ORDER;
    case ORDER_CANCEL_REQUEST:
      return RequestType::CANCEL_ORDER;
    default:
      return {};
  }
}
}  // namespace

void OrderEntry::operator()(Trace<fix::Reject const> const &event, core::fix::Header const &header) {
  auto &[trace_info, reject] = event;
  log::warn<1>("event={{header={}, reject={}}}"sv, header, reject);
  auto request_type = message_type_to_request_type(reject.ref_msg_type);
  if (request_type != RequestType{}) {
    auto iter = msg_seq_num_to_request_id_.find(reject.ref_seq_num);
    if (iter != std::end(msg_seq_num_to_request_id_)) {
      auto &request_id = (*iter).second;
      auto error = fix::reject_to_error(reject.session_reject_reason, reject.text);
      oms::Response response{
          .type = request_type,
          .origin = Origin::EXCHANGE,
          .status = RequestStatus::REJECTED,
          .error = error,
          .text = reject.text,
          .version = {},
          .request_id = request_id,
          .quantity = NaN,
          .price = NaN,
      };
      if (shared_.update_order(request_id, stream_id_, trace_info, response, []([[maybe_unused]] auto &order) {})) {
      } else {
        log::warn<1>(R"(*** NO ORDER WITH REQUEST_ID="{}" ***)"sv, request_id);
      }
    } else {
      log::warn<1>(R"(*** NO REQUEST FOR MSG_SEQ_NUM="{}" ***)"sv, reject.ref_seq_num);
    }
  } else if (reject.session_reject_reason.compare("99"sv) == 0 && reject.text.compare("connection_too_slow"sv) == 0) {
    log::info("closing connection"sv);
    (*connection_manager_).close();
  } else {
    log::fatal("Unexpected"sv);
  }
}

void OrderEntry::operator()(Trace<fix::OrderMassCancelReport const> const &event, core::fix::Header const &header) {
  auto &[trace_info, order_mass_cancel_report] = event;
  log::info<1>("event={{header={}, order_mass_cancel_report={}}}"sv, header, order_mass_cancel_report);
  switch (order_mass_cancel_report.mass_cancel_response) {
    using enum core::fix::MassCancelResponse;
    case CANCEL_REQUEST_REJECTED:
      log::warn(
          R"(*** CANCEL ALL ORDERS FAILED, REASON="{}" ***)"sv, order_mass_cancel_report.mass_cancel_reject_reason);
      break;
    default:
      log::info(
          "*** CANCEL ALL ORDERS SUCCEEDED, TOTAL_AFFECTED_ORDERS={} ***"sv,
          order_mass_cancel_report.total_affected_orders);
  }
}

template <typename T>
uint64_t OrderEntry::send(const T &event) {
  auto now = core::clock::GetRealTime();
  return send(event, now);
}

template <typename T>
uint64_t OrderEntry::send(const T &event, std::chrono::nanoseconds sending_time) {
  core::fix::Writer writer(
      encode_buffer_, FIX_VERSION, T::msg_type, SENDER_COMP_ID, TARGET_COMP_ID, outbound_.msg_seq_num, sending_time);
  auto message = event.encode(writer);
  if (flags::FIX::fix_debug())
    log::info("{}"sv, debug::fix::Message{message});
  (*connection_manager_).send(message);
  return outbound_.msg_seq_num;
}

void OrderEntry::check(core::fix::Header const &header) {
  auto current = header.msg_seq_num;
  auto expected = inbound_.msg_seq_num + 1;
  if (current != expected) [[unlikely]] {
    if (expected < current) {
      log::warn(
          "*** SEQUENCE GAP *** "
          "current={} previous={} distance={}"sv,
          current,
          inbound_.msg_seq_num,
          current - inbound_.msg_seq_num);
    } else {
      log::warn(
          "*** SEQUENCE REPLAY *** "
          "current={} previous={} distance={}"sv,
          current,
          inbound_.msg_seq_num,
          inbound_.msg_seq_num - current);
    }
  }
  inbound_.msg_seq_num = current;
}

}  // namespace deribit
}  // namespace roq
