/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/cme/order_entry.hpp"

#include "roq/mask.hpp"

#include "roq/oms/order.hpp"

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

auto const TRADING_SYSTEM_VENDOR = "ROQ GMBH"sv;
auto const KEEP_ALIVE_INTERVAL = 30s;

auto const REQUEST_TIMEOUT = 5s;
auto const MAX_SEQ_NUM = uint64_t{1000000000};

auto const MANUAL_ORDER_INDICATOR = cme_ilink::ManualOrdIndReq::Automated;  // XXX flag?
}  // namespace

// === HELPERS ===

namespace {
auto create_name(auto stream_id, auto const &account) {
  return fmt::format("{}:{}:{}"_cf, stream_id, NAME, account);
}

auto create_connection_factory(auto &settings, auto &context, auto &uri) {
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

auto get_quantity(double value) -> uint32_t {
  int32_t result = utils::safe_cast{value};
  if (result >= 0)
    return static_cast<uint32_t>(value);
  throw RuntimeError{"Unexpected: value={} < 0"sv, value};
}

// order_request_id
// note! we include request type because ExectuionReportReject523 doesn't have that information

constexpr auto encode_order_request_id(RequestType type, uint8_t user_id, uint64_t order_id) -> uint64_t {
  return (static_cast<uint64_t>(static_cast<uint8_t>(type)) << 56) | (static_cast<uint64_t>(user_id) << 48) | order_id;
}

constexpr auto decode_order_request_id(uint64_t value) -> std::tuple<RequestType, uint8_t, uint64_t> {
  auto request_type = static_cast<RequestType>(static_cast<uint8_t>((value >> 56) & ((uint64_t{1} << 8) - 1)));
  auto user_id = static_cast<uint8_t>((value >> 48) & ((uint64_t{1} << 8) - 1));
  auto order_id = (value & ((uint64_t{1} << 48) - 1));
  return {request_type, user_id, order_id};
}

static_assert(encode_order_request_id(RequestType::CREATE_ORDER, 1, 1) == 0x0101000000000001);
static_assert(encode_order_request_id(RequestType::CANCEL_ORDER, SOURCE_SELF, ORDER_ID_MAX) == 0x03ffffffffffffff);

static_assert(
    decode_order_request_id(0x0101000000000001) ==
    std::tuple<RequestType, uint8_t, uint64_t>{RequestType::CREATE_ORDER, 1, 1});
static_assert(
    decode_order_request_id(0x03ffffffffffffff) ==
    std::tuple<RequestType, uint8_t, uint64_t>{RequestType::CANCEL_ORDER, SOURCE_SELF, ORDER_ID_MAX});

// side

auto map(Side side) -> cme_ilink::SideReq::Value {
  switch (side) {
    using enum Side;
    case UNDEFINED:
      break;
    case BUY:
      return cme_ilink::SideReq::Buy;
    case SELL:
      return cme_ilink::SideReq::Sell;
  }
  return cme_ilink::SideReq::NULL_VALUE;
}

auto map(cme_ilink::SideReq::Value value) -> Side {
  switch (value) {
    using enum cme_ilink::SideReq::Value;
    case Buy:
      return Side::BUY;
    case Sell:
      return Side::SELL;
    case Undisclosed:
    case NULL_VALUE:
      break;
  }
  return {};
}

// order type

// XXX need some more complexity here
auto map(OrderType order_type) -> cme_ilink::OrderTypeReq::Value {
  switch (order_type) {
    using enum OrderType;
    case UNDEFINED:
      break;
    case MARKET:
      return cme_ilink::OrderTypeReq::MarketwithProtection;  // ???
    case LIMIT:
      return cme_ilink::OrderTypeReq::Limit;
  }
  return cme_ilink::OrderTypeReq::NULL_VALUE;
}

auto map(cme_ilink::OrderType::Value value) -> OrderType {
  switch (value) {
    using enum cme_ilink::OrderType::Value;
    case MarketWithProtection:
      return OrderType::MARKET;
    case Limit:
      return OrderType::LIMIT;
    case StopLimit:
      return OrderType::LIMIT;
    case MarketWithLeftoverAsLimit:
      return OrderType::MARKET;  // ???
    case NULL_VALUE:
      break;
  }
  return {};
}

// time in force

auto map(TimeInForce time_in_force) -> cme_ilink::TimeInForce::Value {
  switch (time_in_force) {
    using enum TimeInForce;
    case UNDEFINED:
      break;
    case GFD:
      return cme_ilink::TimeInForce::Day;
    case GTC:
      return cme_ilink::TimeInForce::GoodTillCancel;
    case OPG:
      break;
    case IOC:
      return cme_ilink::TimeInForce::FillAndKill;
    case FOK:
      return cme_ilink::TimeInForce::FillOrKill;  // btec and ebs
    case GTX:
      break;
    case GTD:
      return cme_ilink::TimeInForce::GoodTillDate;
    case AT_THE_CLOSE:
      break;
    case GOOD_THROUGH_CROSSING:
      break;
    case AT_CROSSING:
      break;
    case GOOD_FOR_TIME:
      break;
    case GFA:
      break;
    case GFM:
      break;
  }
  return cme_ilink::TimeInForce::NULL_VALUE;
}

auto map(cme_ilink::TimeInForce::Value value) -> TimeInForce {
  switch (value) {
    using enum cme_ilink::TimeInForce::Value;
    case Day:
      return TimeInForce::GFD;
    case GoodTillCancel:
      return TimeInForce::GTC;
    case FillAndKill:
      return TimeInForce::IOC;
    case FillOrKill:
      return TimeInForce::FOK;
    case GoodTillDate:
      return TimeInForce::GTD;
    case GoodForSession:
      break;  // ???
    case NULL_VALUE:
      break;
  }
  return {};
}

// execution instructions

auto map(Mask<ExecutionInstruction> execution_instructions) -> uint8_t {
  uint8_t result = {};
  auto error = false;
  using value_type = std::remove_cvref<decltype(execution_instructions)>::type;
  using iterator = value_type::iterator;
  using sentinel = value_type::sentinel;
  std::ranges::for_each(iterator{execution_instructions}, sentinel{}, [&](auto item) {
    switch (item) {
      using enum ExecutionInstruction;
      case UNDEFINED:
        break;
      case PARTICIPATE_DO_NOT_INITIATE:
        error = true;
        break;
      case CANCEL_IF_NOT_BEST:
        // result = cme_ilink::ExecInst::oB(result, true); // note! not for futures and options!
        error = true;
        break;
      case DO_NOT_INCREASE:
      case DO_NOT_REDUCE:
        error = true;
        break;
    }
  });
  if (error) [[unlikely]] {
    throw oms::Rejected{Origin::GATEWAY, Error::INVALID_EXECUTION_INSTRUCTION, "unsupported"sv};
  }
  return result;
}

// order status

auto map(cme_ilink::OrderStatus::Value value) -> OrderStatus {
  switch (value) {
    using enum cme_ilink::OrderStatus::Value;
    case New:
      return OrderStatus::WORKING;  // ???
    case PartiallyFilled:
      return OrderStatus::WORKING;
    case Filled:
      return OrderStatus::COMPLETED;
    case Cancelled:
      return OrderStatus::CANCELED;
    case Replaced:
      return OrderStatus::WORKING;
    case PendingCancel:
      return OrderStatus::WORKING;
    case Rejected:
      return OrderStatus::REJECTED;
    case Expired:
      return OrderStatus::EXPIRED;
    case PendingReplace:
      return OrderStatus::ACCEPTED;
    case Undefined:
      break;  // ???
    case NULL_VALUE:
      break;
  }
  return {};
}

auto map(cme_ilink::OrdStatusTrd::Value value) -> OrderStatus {
  switch (value) {
    using enum cme_ilink::OrdStatusTrd::Value;
    case PartiallyFilled:
      return OrderStatus::WORKING;
    case Filled:
      return OrderStatus::COMPLETED;
    case NULL_VALUE:
      break;
  }
  return {};
}

// get

auto get_business_reject_ref_id(auto &value) -> uint64_t {
  auto result = value.businessRejectRefID();
  if (result == value.businessRejectRefIDNullValue())
    return {};
  return result;
}

auto get_order_id(auto &value) -> uint64_t {
  auto result = value.orderID();
  if (result == value.orderIDNullValue())
    return {};
  return result;
}

auto get_security_id(auto &value) -> int32_t {
  auto result = value.securityID();
  if (result == value.securityIDNullValue())
    return {};
  return result;
}

auto get_transact_time(auto &value) -> std::chrono::nanoseconds {
  auto result = value.transactTime();
  if (result == value.transactTimeNullValue())
    return {};
  return std::chrono::nanoseconds{result};
}

auto get_ord_status(auto &value) -> OrderStatus {
  auto ord_status = value.ordStatus();
  if constexpr (std::is_same<decltype(ord_status), char const *>::value) {
    return map(static_cast<cme_ilink::OrderStatus::Value>(ord_status[0]));
  } else if constexpr (std::is_same<decltype(ord_status), cme_ilink::OrdStatusTrd::Value>::value) {
    return map(ord_status);
  } else {
    return map(ord_status);
  }
}

template <typename T>
auto get_leaves_qty(T &value) -> double {
  constexpr bool has_leaves_qty = requires(T const &t) { t.leavesQty(); };
  if constexpr (has_leaves_qty) {
    auto result = value.leavesQty();
    if (result == value.leavesQtyNullValue())
      return NaN;
    return result;
  }
  return NaN;
}

template <typename T>
auto get_cum_qty(T &value) -> double {
  constexpr bool has_cum_qty = requires(T const &t) { t.cumQty(); };
  if constexpr (has_cum_qty) {
    auto result = value.cumQty();
    if (result == value.cumQtyNullValue())
      return NaN;
    return result;
  }
  return NaN;
}

template <typename T>
auto get_last_qty(T &value) -> double {
  constexpr bool has_last_qty = requires(T const &t) { t.lastQty(); };
  constexpr bool has_last_qty_null_value = requires(T const &t) { t.lastQtyNullValue(); };
  if constexpr (has_last_qty) {
    auto result = value.lastQty();
    if constexpr (has_last_qty_null_value) {
      if (result == value.lastQtyNullValue())
        return NaN;
    }
    return result;
  }
  return NaN;
}

template <typename T>
auto get_last_px(T &value) -> double {
  constexpr bool has_last_px = requires(T const &t) { t.lastPx(); };
  if constexpr (has_last_px) {
    return ilink::get_double(const_cast<T &>(value).lastPx());
  }
  return NaN;
}

template <typename T>
auto get_aggressor_indicator(T &value) -> Liquidity {
  constexpr bool has_aggressor_indicator = requires(T const &t) { t.aggressorIndicator(); };
  if constexpr (has_aggressor_indicator) {
    auto tmp = value.aggressorIndicator();
    switch (tmp) {
      using enum cme_ilink::BooleanFlag::Value;
      case False:
        return Liquidity::MAKER;
      case True:
        return Liquidity::TAKER;
      case NULL_VALUE:
        return {};
    }
  }
  return {};
}

// execution report -> order update

template <typename T>
auto order_update_from_execution_report(T &value, auto &security, auto &external_order_id) -> oms::OrderUpdate {
  using value_type = std::remove_cvref<T>::type;
  auto account = value.getSenderIDAsStringView();
  auto side = map(value.side());
  auto order_type = map(value.ordType());
  auto time_in_force = map(value.timeInForce());
  auto create_time_utc = get_transact_time(value);
  auto update_time_utc = create_time_utc;
  auto client_order_id = value.getClOrdIDAsStringView();
  auto order_status = get_ord_status(value);
  auto quantity = static_cast<double>(value.orderQty());
  auto price = ilink::get_double(const_cast<value_type &>(value).price());
  auto stop_price = ilink::get_double(const_cast<value_type &>(value).stopPx());
  auto remaining_quantity = get_leaves_qty(value);
  auto traded_quantity = get_cum_qty(value);
  auto last_traded_quantity = get_last_qty(value);
  auto last_traded_price = get_last_px(value);
  auto liquidity = get_aggressor_indicator(value);
  auto update_type = [&]() {
    if constexpr (std::is_same<value_type, cme_ilink::ExecutionReportStatus532>::value) {
      return UpdateType::SNAPSHOT;
    } else {
      return UpdateType::INCREMENTAL;
    }
  }();
  return {
      .account = account,
      .exchange = security.exchange,
      .symbol = security.symbol,
      .side = side,
      .position_effect = {},
      .max_show_quantity = NaN,
      .order_type = order_type,
      .time_in_force = time_in_force,
      .execution_instructions = {},
      .create_time_utc = create_time_utc,
      .update_time_utc = update_time_utc,
      .external_account = {},
      .external_order_id = external_order_id,
      .client_order_id = client_order_id,
      .status = order_status,
      .quantity = quantity,
      .price = price,
      .stop_price = stop_price,
      .remaining_quantity = remaining_quantity,
      .traded_quantity = traded_quantity,
      .average_traded_price = NaN,
      .last_traded_quantity = last_traded_quantity,
      .last_traded_price = last_traded_price,
      .last_liquidity = liquidity,
      .routing_id = {},
      .max_request_version = {},
      .max_response_version = {},
      .max_accepted_version = {},
      .update_type = update_type,
      .sending_time_utc = {},
  };
}
}  // namespace

// === IMPLEMENTATION ===

OrderEntry::OrderEntry(
    Handler &handler,
    io::Context &context,
    uint16_t stream_id,
    Account &account,
    Shared &shared,
    uint8_t market_segment_id,
    io::web::URI const &uri)
    : handler_{handler}, stream_id_{stream_id}, name_{create_name(stream_id_, account.get_name())},
      market_segment_id_{market_segment_id},
      connection_factory_{create_connection_factory(shared.settings, context, uri)},
      connection_manager_{create_connection_manager(*this, shared.settings, *connection_factory_)},
      decode_buffer_(shared.settings.common.decode_buffer_size),
      encode_buffer_2_(shared.settings.common.encode_buffer_size),
      counter_{
          .disconnect = create_metrics(shared.settings, name_, "disconnect"sv),
      },
      profile_{
          .parse = create_metrics(shared.settings, name_, "parse"sv),
          .negotiation_response = create_metrics(shared.settings, name_, "negotiation_response"sv),
          .negotiation_reject = create_metrics(shared.settings, name_, "negotiation_reject"sv),
          .establishment_ack = create_metrics(shared.settings, name_, "establishment_ack"sv),
          .establishment_reject = create_metrics(shared.settings, name_, "establishment_reject"sv),
          .sequence = create_metrics(shared.settings, name_, "sequence"sv),
          .terminate = create_metrics(shared.settings, name_, "terminate"sv),
          .retransmission = create_metrics(shared.settings, name_, "retransmission"sv),
          .retransmission_reject = create_metrics(shared.settings, name_, "retransmission_reject"sv),
          .not_applied = create_metrics(shared.settings, name_, "not_applied"sv),
          .party_details_definition_request_ack =
              create_metrics(shared.settings, name_, "party_details_definition_request_ack"sv),
          .business_reject = create_metrics(shared.settings, name_, "business_reject"sv),
          .execution_report_new = create_metrics(shared.settings, name_, "execution_report_new"sv),
          .execution_report_reject = create_metrics(shared.settings, name_, "execution_report_reject"sv),
          .execution_report_trade_outright =
              create_metrics(shared.settings, name_, "execution_report_trade_outright"sv),
          .execution_report_trade_spread = create_metrics(shared.settings, name_, "execution_report_trade_spread"sv),
          .execution_report_trade_spread_leg =
              create_metrics(shared.settings, name_, "execution_report_trade_spread_leg"sv),
          .execution_report_modify = create_metrics(shared.settings, name_, "execution_report_modify"sv),
          .execution_report_status = create_metrics(shared.settings, name_, "execution_report_status"sv),
          .execution_report_cancel = create_metrics(shared.settings, name_, "execution_report_cancel"sv),
          .execution_report_pending_cancel =
              create_metrics(shared.settings, name_, "execution_report_pending_cancel"sv),
          .execution_report_pending_replace =
              create_metrics(shared.settings, name_, "execution_report_pending_replace"sv),
          .order_cancel_reject = create_metrics(shared.settings, name_, "order_cancel_reject"sv),
          .order_cancel_replace_reject = create_metrics(shared.settings, name_, "order_cancel_replace_reject"sv),
          .order_mass_action_report = create_metrics(shared.settings, name_, "order_mass_action_report"sv),
          .security_definition_response = create_metrics(shared.settings, name_, "security_definition_response"sv),
      },
      latency_{
          .ping = create_metrics(shared.settings, name_, "ping"sv),
      },
      account_{account}, shared_{shared}, download_{REQUEST_TIMEOUT, [this](auto state) { return download(state); }} {
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
    Event<CreateOrder> const &event, oms::Order const &order, std::string_view const &request_id) {
  if (!ready()) [[unlikely]]
    throw oms::NotReady{"not ready"sv};
  send_new_order_single(event.value, order, request_id);
  return stream_id_;
}

uint16_t OrderEntry::operator()(
    Event<ModifyOrder> const &event,
    oms::Order const &order,
    [[maybe_unused]] std::string_view const &request_id,
    [[maybe_unused]] std::string_view const &previous_request_id) {
  if (!ready()) [[unlikely]]
    throw oms::NotReady{"not ready"sv};
  send_order_cancel_replace_request(event.value, order);
  return stream_id_;
}

uint16_t OrderEntry::operator()(
    Event<CancelOrder> const &event,
    oms::Order const &order,
    [[maybe_unused]] std::string_view const &request_id,
    [[maybe_unused]] std::string_view const &previous_request_id) {
  if (!ready()) [[unlikely]]
    throw oms::NotReady{"not ready"sv};
  send_order_cancel_request(event.value, order);
  return stream_id_;
}

uint16_t OrderEntry::operator()(
    Event<CancelAllOrders> const &event, [[maybe_unused]] std::string_view const &request_id) {
  if (ready()) {
    send_order_mass_action_request(event.value);
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
      .write(profile_.negotiation_response, metrics::PROFILE)
      .write(profile_.negotiation_reject, metrics::PROFILE)
      .write(profile_.establishment_ack, metrics::PROFILE)
      .write(profile_.establishment_reject, metrics::PROFILE)
      .write(profile_.sequence, metrics::PROFILE)
      .write(profile_.terminate, metrics::PROFILE)
      .write(profile_.retransmission, metrics::PROFILE)
      .write(profile_.retransmission_reject, metrics::PROFILE)
      .write(profile_.not_applied, metrics::PROFILE)
      .write(profile_.party_details_definition_request_ack, metrics::PROFILE)
      .write(profile_.business_reject, metrics::PROFILE)
      .write(profile_.execution_report_new, metrics::PROFILE)
      .write(profile_.execution_report_reject, metrics::PROFILE)
      .write(profile_.execution_report_trade_outright, metrics::PROFILE)
      .write(profile_.execution_report_trade_spread, metrics::PROFILE)
      .write(profile_.execution_report_trade_spread_leg, metrics::PROFILE)
      .write(profile_.execution_report_modify, metrics::PROFILE)
      .write(profile_.execution_report_status, metrics::PROFILE)
      .write(profile_.execution_report_cancel, metrics::PROFILE)
      .write(profile_.execution_report_pending_cancel, metrics::PROFILE)
      .write(profile_.execution_report_pending_replace, metrics::PROFILE)
      .write(profile_.order_cancel_reject, metrics::PROFILE)
      .write(profile_.order_cancel_replace_reject, metrics::PROFILE)
      .write(profile_.order_mass_action_report, metrics::PROFILE)
      .write(profile_.security_definition_response, metrics::PROFILE)
      .write(latency_.ping, metrics::LATENCY);
}

// session

void OrderEntry::operator()(Trace<cme_ilink::NegotiationResponse501> const &event) {
  profile_.negotiation_response([&]() {
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &[trace_info, value] = event;
    log::info("DEBUG negotiation_response={}"sv, const_cast<value_type &>(value));
    send_establish();
  });
}

void OrderEntry::operator()(Trace<cme_ilink::NegotiationReject502> const &event) {
  log::error("negotiation_reject="sv);
  profile_.negotiation_reject([&]() {
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &[trace_info, value] = event;
    log::error("negotiation_reject={}"sv, const_cast<value_type &>(value));
    // XXX now what?
    (*connection_manager_).close();
  });
}

void OrderEntry::operator()(Trace<cme_ilink::EstablishmentAck504> const &event) {
  profile_.establishment_ack([&]() {
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &[trace_info, value] = event;
    log::info("DEBUG establishment_ack={}"sv, const_cast<value_type &>(value));
    (*this)(ConnectionStatus::DOWNLOADING);
    download_.begin();
  });
}

void OrderEntry::operator()(Trace<cme_ilink::EstablishmentReject505> const &event) {
  log::error("establishment_reject"sv);
  profile_.establishment_reject([&]() {
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &[trace_info, value] = event;
    log::error("establishment_reject={}"sv, const_cast<value_type &>(value));
    // XXX now what?
    (*connection_manager_).close();
  });
}

void OrderEntry::operator()(Trace<cme_ilink::Sequence506> const &event) {
  profile_.sequence([&]() {
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &[trace_info, value] = event;
    // log::info("DEBUG sequence={}"sv, const_cast<value_type &>(value));
  });
}

void OrderEntry::operator()(Trace<cme_ilink::Terminate507> const &event) {
  log::error("terminate"sv);
  profile_.terminate([&]() {
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &[trace_info, value] = event;
    log::warn("terminate={}"sv, const_cast<value_type &>(value));
    (*connection_manager_).close();
  });
}

void OrderEntry::operator()(Trace<cme_ilink::Retransmission509> const &event) {
  profile_.retransmission([&]() {
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &[trace_info, value] = event;
    log::warn("UNSUPPORTED retransmission={}"sv, const_cast<value_type &>(value));
  });
}

void OrderEntry::operator()(Trace<cme_ilink::RetransmitReject510> const &event) {
  profile_.retransmission_reject([&]() {
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &[trace_info, value] = event;
    log::warn("UNSUPPORTED retransmit_reject={}"sv, const_cast<value_type &>(value));
  });
}

void OrderEntry::operator()(Trace<cme_ilink::NotApplied513> const &event) {
  profile_.not_applied([&]() {
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &[trace_info, value] = event;
    log::warn("UNSUPPORTED not_applied={}"sv, const_cast<value_type &>(value));
  });
}

// business

void OrderEntry::operator()(Trace<cme_ilink::PartyDetailsDefinitionRequestAck519> const &event) {
  profile_.party_details_definition_request_ack([&]() {
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &[trace_info, value] = event;
    log::info("DEBUG party_defails_definition_request_ack={}"sv, const_cast<value_type &>(value));
  });
}

void OrderEntry::operator()(Trace<cme_ilink::BusinessReject521> const &event) {
  log::error("business_reject"sv);
  profile_.business_reject([&]() {
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &[trace_info, value] = event;
    log::error("business_reject={}"sv, const_cast<value_type &>(value));
    auto ref_msg_type = value.getRefMsgTypeAsStringView();
    if (ref_msg_type == "D"sv || ref_msg_type == "G"sv || ref_msg_type == "F"sv) {
      auto [type, user_id, order_id] = decode_order_request_id(get_business_reject_ref_id(value));
      log::info("DEBUG type={}, user_id={}, order_id={}"sv, type, user_id, order_id);
      auto text = value.getTextAsStringView();
      auto response = oms::Response{
          .type = type,
          .origin = Origin::EXCHANGE,
          .status = RequestStatus::REJECTED,
          .error = {},
          .text = text,
          .version = {},
          .request_id = {},
          .quantity = NaN,
          .price = NaN,
      };
      Trace event_2{trace_info, response};
      (*this)(event_2, user_id, order_id);
    }
  });
}

// execution report
void OrderEntry::operator()(Trace<cme_ilink::ExecutionReportNew522> const &event) {
  profile_.execution_report_new([&]() {
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &trace_info = event.trace_info;
    auto &value = event.value;
    log::info("DEBUG execution_report_new={}"sv, const_cast<value_type &>(value));
    auto order_id = get_order_id(value);
    if (order_id) {
      auto external_order_id = fmt::format("{}"sv, order_id);
      auto security_id = get_security_id(value);
      if (shared_.get_security(security_id, [&](auto &security) {
            auto [type, user_id, order_id] = decode_order_request_id(value.orderRequestID());
            log::info("DEBUG type={}, user_id={}, order_id={}"sv, type, user_id, order_id);
            if (type != RequestType::CREATE_ORDER) [[unlikely]]
              log::warn("Unexpected: type={}"sv, type);
            auto order_update = order_update_from_execution_report(value, security, external_order_id);
            auto response = oms::Response{
                .type = type,
                .origin = Origin::EXCHANGE,
                .status = RequestStatus::ACCEPTED,
                .error = {},
                .text = {},
                .version = {},
                .request_id = {},
                .quantity = order_update.quantity,
                .price = order_update.price,
            };
            Trace event_2{trace_info, response};
            (*this)(event_2, order_update.client_order_id, order_update);
          })) {
      } else {
        log::warn("Unexpected: security_id={}"sv, security_id);
      }
    } else {
      log::warn("Unexpected: order_id={}"sv, order_id);
    }
  });
}

void OrderEntry::operator()(Trace<cme_ilink::ExecutionReportReject523> const &event) {
  profile_.execution_report_reject([&]() {
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &trace_info = event.trace_info;
    auto &value = event.value;
    log::info("DEBUG execution_report_reject={}"sv, const_cast<value_type &>(value));
    auto cl_ord_id = value.getClOrdIDAsStringView();
    auto text = value.getTextAsStringView();
    auto [type, user_id, order_id] = decode_order_request_id(value.orderRequestID());
    log::info("DEBUG type={}, user_id={}, order_id={}"sv, type, user_id, order_id);
    if (type != RequestType::CREATE_ORDER) [[unlikely]]
      log::warn("Unexpected: type={}"sv, type);
    auto response = oms::Response{
        .type = type,
        .origin = Origin::EXCHANGE,
        .status = RequestStatus::REJECTED,
        .error = {},
        .text = text,
        .version = {},
        .request_id = {},
        .quantity = NaN,
        .price = NaN,
    };
    Trace event_2{trace_info, response};
    (*this)(event_2, cl_ord_id);
  });
}

void OrderEntry::operator()(Trace<cme_ilink::ExecutionReportTradeOutright525> const &event) {
  profile_.execution_report_trade_outright([&]() {
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &trace_info = event.trace_info;
    auto &value = event.value;
    log::info("DEBUG execution_report_trade_outright={}"sv, const_cast<value_type &>(value));
    auto order_id = get_order_id(value);
    if (order_id) {
      auto external_order_id = fmt::format("{}"sv, order_id);
      auto security_id = get_security_id(value);
      if (shared_.get_security(security_id, [&](auto &security) {
            auto order_update = order_update_from_execution_report(value, security, external_order_id);
            auto user_id = SOURCE_NONE;
            auto order_id = ORDER_ID_NONE;
            auto strategy_id = STRATEGY_ID_NONE;
            auto callback = [&](auto &order) {
              user_id = order.user_id;
              order_id = order.order_id;
              strategy_id = order.strategy_id;
            };
            Trace event_2{trace_info, order_update};
            (*this)(callback, event_2, order_update.client_order_id);
            // XXX TODO make generic
            auto &fills = shared_.get_fills();
            std::vector<std::string> external_trade_ids;  // alloc
            auto exec_id = value.getExecIDAsStringView();
            auto liquidity = get_aggressor_indicator(value);
            const_cast<value_type &>(value).sbeRewind();  // note!
            const_cast<value_type &>(value).noFills().forEach([&](auto &item) {
              auto fill_exec_id = item.getFillExecIDAsStringView();
              auto tmp = fmt::format("{}{}"sv, exec_id, fill_exec_id);  // alloc
              external_trade_ids.emplace_back(std::move(tmp));
              auto &external_trade_id = external_trade_ids.back();
              auto price = ilink::get_double(item.fillPx());
              auto fill = Fill{
                  .external_trade_id = external_trade_id,
                  .quantity = static_cast<double>(item.fillQty()),
                  .price = price,
                  .liquidity = liquidity,
              };
              fills.emplace_back(std::move(fill));
            });
            if (!std::empty(fills)) {
              auto side = map(value.side());
              auto create_time_utc = get_transact_time(value);
              auto update_time_utc = create_time_utc;
              auto trade_update = TradeUpdate{
                  .stream_id = stream_id_,
                  .account = account_.get_name(),
                  .order_id = order_id,
                  .exchange = security.exchange,
                  .symbol = security.symbol,
                  .side = side,
                  .position_effect = {},
                  .create_time_utc = create_time_utc,
                  .update_time_utc = update_time_utc,
                  .external_account = {},
                  .external_order_id = external_order_id,
                  .client_order_id = {},
                  .fills = fills,
                  .routing_id = {},
                  .update_type = UpdateType::INCREMENTAL,
                  .sending_time_utc = {},
                  .user = {},
                  .strategy_id = strategy_id,
              };
              create_trace_and_dispatch(shared_, trace_info, trade_update, true, user_id, order_update.client_order_id);
            }
          })) {
      } else {
        log::warn("Unexpected: security_id={}"sv, security_id);
      }
    } else {
      log::warn("Unexpected: order_id={}"sv, order_id);
    }
  });
}

void OrderEntry::operator()(Trace<cme_ilink::ExecutionReportTradeSpread526> const &event) {
  profile_.execution_report_trade_spread([&]() {
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &[trace_info, value] = event;
    log::warn("UNSUPPORTED execution_report_trade_spread={}"sv, const_cast<value_type &>(value));
  });
}

void OrderEntry::operator()(Trace<cme_ilink::ExecutionReportTradeSpreadLeg527> const &event) {
  profile_.execution_report_trade_spread_leg([&]() {
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &[trace_info, value] = event;
    log::warn("UNSUPPORTED execution_report_trade_spread_leg={}"sv, const_cast<value_type &>(value));
  });
}

void OrderEntry::operator()(Trace<cme_ilink::ExecutionReportModify531> const &event) {
  profile_.execution_report_modify([&]() {
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &trace_info = event.trace_info;
    auto &value = event.value;
    log::info("DEBUG execution_report_modify={}"sv, const_cast<value_type &>(value));
    auto order_id = get_order_id(value);
    if (order_id) {
      auto external_order_id = fmt::format("{}"sv, order_id);
      auto security_id = get_security_id(value);
      if (shared_.get_security(security_id, [&](auto &security) {
            auto [type, user_id, order_id] = decode_order_request_id(value.orderRequestID());
            log::info("DEBUG type={}, user_id={}, order_id={}"sv, type, user_id, order_id);
            if (type != RequestType::MODIFY_ORDER) [[unlikely]]
              log::warn("Unexpected: type={}"sv, type);
            auto order_update = order_update_from_execution_report(value, security, external_order_id);
            auto response = oms::Response{
                .type = type,
                .origin = Origin::EXCHANGE,
                .status = RequestStatus::ACCEPTED,
                .error = {},
                .text = {},
                .version = {},
                .request_id = {},
                .quantity = order_update.quantity,
                .price = order_update.price,
            };
            Trace event_2{trace_info, response};
            (*this)(event_2, order_update.client_order_id, order_update);
          })) {
      } else {
        log::warn("Unexpected: security_id={}"sv, security_id);
      }
    } else {
      log::warn("Unexpected: order_id={}"sv, order_id);
    }
  });
}

void OrderEntry::operator()(Trace<cme_ilink::ExecutionReportStatus532> const &event) {
  profile_.execution_report_status([&]() {
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &trace_info = event.trace_info;
    auto &value = event.value;
    log::info("DEBUG execution_report_status={}"sv, const_cast<value_type &>(value));
    auto order_id = get_order_id(value);
    if (order_id) {
      auto external_order_id = fmt::format("{}"sv, order_id);
      auto security_id = get_security_id(value);
      if (shared_.get_security(security_id, [&](auto &security) {
            auto order_update = order_update_from_execution_report(value, security, external_order_id);
            auto callback = []([[maybe_unused]] auto &order) {};
            Trace event_2{trace_info, order_update};
            (*this)(callback, event_2, order_update.client_order_id);
          })) {
      } else {
        log::warn("Unexpected: security_id={}"sv, security_id);
      }
    } else {
      log::info(R"(No working orders (text="{}"))"sv, value.getTextAsStringView());
    }
    if (value.lastRptRequested() == cme_ilink::BooleanNULL::True)
      download_.check_relaxed(OrderEntryState::ORDERS);
  });
}

void OrderEntry::operator()(Trace<cme_ilink::ExecutionReportCancel534> const &event) {
  profile_.execution_report_cancel([&]() {
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &trace_info = event.trace_info;
    auto &value = event.value;
    log::info("DEBUG execution_report_cancel={}"sv, const_cast<value_type &>(value));
    auto order_id = get_order_id(value);
    if (order_id) {
      auto external_order_id = fmt::format("{}"sv, order_id);
      auto security_id = get_security_id(value);
      if (shared_.get_security(security_id, [&](auto &security) {
            auto [type, user_id, order_id] = decode_order_request_id(value.orderRequestID());
            log::info("DEBUG type={}, user_id={}, order_id={}"sv, type, user_id, order_id);
            if (type != RequestType::CANCEL_ORDER) [[unlikely]]
              log::warn("Unexpected: type={}"sv, type);
            auto order_update = order_update_from_execution_report(value, security, external_order_id);
            auto response = oms::Response{
                .type = RequestType::CANCEL_ORDER,
                .origin = Origin::EXCHANGE,
                .status = RequestStatus::ACCEPTED,
                .error = {},
                .text = {},
                .version = {},
                .request_id = {},
                .quantity = order_update.quantity,
                .price = order_update.price,
            };
            Trace event_2{trace_info, response};
            (*this)(event_2, order_update.client_order_id, order_update);
          })) {
      } else {
        log::warn("Unexpected: security_id={}"sv, security_id);
      }
    } else {
      log::warn("Unexpected: order_id={}"sv, order_id);
    }
  });
}

void OrderEntry::operator()(Trace<cme_ilink::ExecutionReportPendingCancel564> const &event) {
  profile_.execution_report_pending_cancel([&]() {
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &[trace_info, value] = event;
    log::warn("UNSUPPORTED execution_report_pending_cancel={}"sv, const_cast<value_type &>(value));
  });
}

void OrderEntry::operator()(Trace<cme_ilink::ExecutionReportPendingReplace565> const &event) {
  profile_.execution_report_pending_replace([&]() {
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &[trace_info, value] = event;
    log::warn("UNSUPPORTED execution_report_pending_replace={}"sv, const_cast<value_type &>(value));
  });
}

// order mass action

void OrderEntry::operator()(Trace<cme_ilink::OrderMassActionReport562> const &event) {
  profile_.order_mass_action_report([&]() {
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &trace_info = event.trace_info;
    auto &value = event.value;
    log::info("DEBUG order_mass_action_report={}"sv, const_cast<value_type &>(value));
    switch (value.massActionResponse()) {
      using enum cme_ilink::MassActionResponse::Value;
      case Rejected: {
        auto mass_action_reject_reason = value.massActionRejectReason();
        log::info("*** CANCEL ALL ORDERS FAILED, REASON={} ***"sv, mass_action_reject_reason);
        break;
      }
      case Accepted: {
        auto account = value.getSenderIDAsStringView();
        auto update_time_utc = get_transact_time(value);
        size_t count = 0;
        const_cast<value_type &>(value).sbeRewind();  // note!
        const_cast<value_type &>(value).noAffectedOrders().forEach([&](auto &item) {
          ++count;
          auto orig_cl_ord_id = item.getOrigCIOrdIDAsStringView();
          auto affected_order_id = item.affectedOrderID();
          auto cxl_quantity = item.cxlQuantity();
          log::info(
              R"(DEBUG orig_cl_ord_id="{}", affected_order_id={}, cxl_quantity={})"sv,
              orig_cl_ord_id,
              affected_order_id,
              cxl_quantity);
          auto external_order_id = fmt::format("{}"sv, affected_order_id);
          auto order_update = oms::OrderUpdate{
              .account = account,
              .exchange = {},
              .symbol = {},
              .side = {},
              .position_effect = {},
              .max_show_quantity = NaN,
              .order_type = {},
              .time_in_force = {},
              .execution_instructions = {},
              .create_time_utc = {},
              .update_time_utc = update_time_utc,
              .external_account = {},
              .external_order_id = external_order_id,
              .client_order_id = orig_cl_ord_id,
              .status = OrderStatus::CANCELED,
              .quantity = {},
              .price = NaN,
              .stop_price = NaN,
              .remaining_quantity = NaN,
              .traded_quantity = NaN,
              .average_traded_price = NaN,
              .last_traded_quantity = NaN,
              .last_traded_price = NaN,
              .last_liquidity = {},
              .routing_id = {},
              .max_request_version = {},
              .max_response_version = {},
              .max_accepted_version = {},
              .update_type = UpdateType::INCREMENTAL,
              .sending_time_utc = {},
          };
          auto callback = []([[maybe_unused]] auto &order) {};
          Trace event_2{trace_info, order_update};
          (*this)(callback, event_2, orig_cl_ord_id);
        });
        log::info("*** CANCEL ALL ORDERS SUCCEEDED, TOTAL_AFFECTED_ORDERS={} ***"sv, count);
        break;
      }
      case NULL_VALUE:
        break;
    }
  });
}

// order

void OrderEntry::operator()(Trace<cme_ilink::OrderCancelReject535> const &event) {
  profile_.order_cancel_reject([&]() {
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &[trace_info, value] = event;
    log::info("order_cancel_reject={}"sv, const_cast<value_type &>(value));
    auto cl_ord_id = value.getClOrdIDAsStringView();
    auto text = value.getTextAsStringView();
    auto [type, user_id, order_id] = decode_order_request_id(value.orderRequestID());
    log::info("DEBUG type={}, user_id={}, order_id={}"sv, type, user_id, order_id);
    if (type != RequestType::CANCEL_ORDER) [[unlikely]]
      log::warn("Unexpected: type={}"sv, type);
    auto response = oms::Response{
        .type = type,
        .origin = Origin::EXCHANGE,
        .status = RequestStatus::REJECTED,
        .error = {},
        .text = text,
        .version = {},
        .request_id = {},
        .quantity = NaN,
        .price = NaN,
    };
    Trace event_2{trace_info, response};
    (*this)(event_2, cl_ord_id);
  });
}

// note!
//   maybe OrderRequestID can be used to correlate version?
void OrderEntry::operator()(Trace<cme_ilink::OrderCancelReplaceReject536> const &event) {
  profile_.order_cancel_replace_reject([&]() {
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &[trace_info, value] = event;
    log::info("DEBUG order_cancel_replace_reject={}"sv, const_cast<value_type &>(value));
    auto cl_ord_id = value.getClOrdIDAsStringView();
    auto text = value.getTextAsStringView();
    auto [type, user_id, order_id] = decode_order_request_id(value.orderRequestID());
    log::info("DEBUG type={}, user_id={}, order_id={}"sv, type, user_id, order_id);
    if (type != RequestType::MODIFY_ORDER) [[unlikely]]
      log::warn("Unexpected: type={}"sv, type);
    auto response = oms::Response{
        .type = type,
        .origin = Origin::EXCHANGE,
        .status = RequestStatus::REJECTED,
        .error = {},
        .text = text,
        .version = {},
        .request_id = {},
        .quantity = NaN,
        .price = NaN,
    };
    Trace event_2{trace_info, response};
    (*this)(event_2, cl_ord_id);
  });
}

// security definition

void OrderEntry::operator()(Trace<cme_ilink::SecurityDefinitionResponse561> const &event) {
  profile_.security_definition_response([&]() {
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &[trace_info, value] = event;
    log::info("DEBUG security_definition_response={}"sv, const_cast<value_type &>(value));
  });
}

void OrderEntry::operator()(io::net::ConnectionManager::Connected const &) {
  send_negotiate();
  (*this)(ConnectionStatus::LOGIN_SENT);
}

void OrderEntry::operator()(io::net::ConnectionManager::Disconnected const &) {
  outbound_ = {};
  inbound_ = {};
  download_.reset();
  next_heartbeat_ = {};
  (*this)(ConnectionStatus::DISCONNECTED);
  ++counter_.disconnect;
}

void OrderEntry::operator()(io::net::ConnectionManager::Read const &) {
  auto buffer = (*connection_manager_).buffer();
  size_t total_bytes = 0;
  while (!std::empty(buffer)) {
    auto bytes = parse(buffer);
    if (bytes == 0) {  // note! we didn't receive the full message
      log::error("!!! MESSAGE WAS NOT RECEIVED IN FULL !!!"sv);
      break;
    }
    assert(bytes <= std::size(buffer));
    total_bytes += bytes;
    buffer = buffer.subspan(bytes);
  }
  (*connection_manager_).drain(total_bytes);
}

size_t OrderEntry::parse(std::span<std::byte const> const &buffer) {
  size_t result = 0;
  profile_.parse([&]() {
    TraceInfo trace_info;
    result = ilink::Parser::dispatch(*this, buffer, trace_info);
  });
  return result;
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

uint32_t OrderEntry::download(OrderEntryState state) {
  switch (state) {
    using enum OrderEntryState;
    case UNDEFINED:
      assert(false);
      break;
    case ORDERS:
      if (!party_details_list_req_id_)
        send_party_details_definition_request();
      send_order_mass_status_request();
      return 1;
    case DONE:
      (*this)(ConnectionStatus::READY);
      return {};
  }
  assert(false);
  return {};
}

//

uint32_t OrderEntry::peek_next_seq_num() {
  return (outbound_.msg_seq_num + 1) % MAX_SEQ_NUM;
}

uint32_t OrderEntry::fetch_next_seq_num() {
  auto result = peek_next_seq_num();
  outbound_.msg_seq_num = result;
  return result;
}

uint64_t OrderEntry::fetch_next_request_id() {
  return ++request_id_;
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
  // log::info(R"(DEBUG message="{}{}")"sv, debug::hex::Message{data[0]}, debug::hex::Message{data[1]});
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
      .firm_id = shared_.settings.ilink.firm_id,
      // note! following must be empty for the negotiate request
      .trading_system_name = {},
      .trading_system_version = {},
      .trading_system_vendor = {},
      .next_seq_no = {},
      .keep_alive_interval = {},
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
      .firm_id = shared_.settings.ilink.firm_id,
      .trading_system_name = ROQ_PACKAGE_NAME,
      .trading_system_version = ROQ_BUILD_VERSION,
      .trading_system_vendor = TRADING_SYSTEM_VENDOR,
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
  // log::info("DEBUG sequence={}"sv, sequence);
  send(sequence);
}

void OrderEntry::send_terminate() {
  auto terminate = ilink::Terminate{};
  log::info("DEBUG terminate={}"sv, terminate);
  send(terminate);
}

// XXX requires access to a service gateway
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
          .party_detail_id = shared_.settings.ilink.firm_id,
          .party_detail_role = cme_ilink::PartyDetailRole::ExecutingFirm,
      },
      {
          .party_detail_id = account_.get_name(),
          .party_detail_role = cme_ilink::PartyDetailRole::CustomerAccount,
      },
      {
          .party_detail_id = account_.get_name(),
          .party_detail_role = cme_ilink::PartyDetailRole::Operator,
      },
  }};
  auto party_details_definition_request = ilink::PartyDetailsDefinitionRequest{
      .party_details_list_req_id = {},  // note! must be 0
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

// XXX ???
void OrderEntry::send_security_definition_request() {
  auto now = clock::get_realtime();
  auto security_definition_request = ilink::SecurityDefinitionRequest{
      .party_details_list_req_id = party_details_list_req_id_,
      .security_req_id = {},
      .manual_order_indicator = MANUAL_ORDER_INDICATOR,
      .seq_num = fetch_next_seq_num(),
      .sender_id = account_.get_name(),
      .sending_time_epoch = now,
      .security_sub_type = {},
      .location = shared_.settings.ilink.location,
      .start_date = {},
      .end_date = {},
      .max_no_of_substitutions = {},
      .source_repo_id = {},
      .broken_date_term_type = {},
  };
  log::info("DEBUG security_definition_request={}"sv, security_definition_request);
  send(security_definition_request);
}

void OrderEntry::send_order_mass_status_request() {
  auto now = clock::get_realtime();
  auto order_mass_status_request = ilink::OrderMassStatusRequest{
      .party_details_list_req_id = party_details_list_req_id_,
      .mass_status_req_id = fetch_next_request_id(),
      .manual_order_indicator = MANUAL_ORDER_INDICATOR,
      .seq_num = fetch_next_seq_num(),
      .sender_id = account_.get_name(),
      .sending_time_epoch = now,
      .security_group = {},
      .location = shared_.settings.ilink.location,
      .security_id = {},
      .mass_status_req_type = cme_ilink::MassStatusReqTyp::MarketSegment,
      .ord_status_req_type = cme_ilink::MassStatusOrdTyp::Account,
      .time_in_force = cme_ilink::MassStatusTIF::NULL_VALUE,
      .market_segment_id = market_segment_id_,
  };
  log::info("DEBUG order_mass_status_request={}"sv, order_mass_status_request);
  send(order_mass_status_request);
}

// note!
//   undoc: execution mode must be x0 (NULL results in reject reason 109)
void OrderEntry::send_new_order_single(
    CreateOrder const &create_order, oms::Order const &order, std::string_view const &request_id) {
  if (shared_.find_security_id(market_segment_id_, order.symbol, [&](auto security_id) {
        log::info("DEBUG found security_id={}"sv, security_id);
        if (shared_.get_security(security_id, [&](auto &security) {
              if (create_order.exchange != security.exchange) [[unlikely]] {
                throw oms::Rejected{Origin::GATEWAY, Error::INVALID_EXCHANGE, "Expected: {}"sv, security.exchange};
              }
              auto order_qty = get_quantity(create_order.quantity);
              auto side = map(create_order.side);
              auto order_request_id = encode_order_request_id(RequestType::CREATE_ORDER, order.user_id, order.order_id);
              auto ord_type = map(create_order.order_type);
              auto time_in_force = map(create_order.time_in_force);
              auto exec_inst = map(create_order.execution_instructions);
              if (!party_details_list_req_id_)
                send_party_details_definition_request();
              auto now = clock::get_realtime();
              auto new_order_single = ilink::NewOrderSingle{
                  .price = create_order.price,
                  .order_qty = order_qty,
                  .security_id = security_id,
                  .side = side,
                  .seq_num = fetch_next_seq_num(),
                  .sender_id = account_.get_name(),
                  .cl_ord_id = request_id,
                  .party_details_list_req_id = party_details_list_req_id_,
                  .order_request_id = order_request_id,
                  .sending_time_epoch = now,
                  .stop_px = create_order.stop_price,
                  .location = shared_.settings.ilink.location,
                  .min_qty = {},
                  .display_qty = {},
                  .expire_date = {},
                  .ord_type = ord_type,
                  .time_in_force = time_in_force,
                  .manual_order_indicator = MANUAL_ORDER_INDICATOR,
                  .exec_inst = exec_inst,
                  .execution_mode = static_cast<cme_ilink::ExecMode::Value>(0),  // note!
                  .liquidity_flag = {},
                  .managed_order = {},
                  .short_sale_type = cme_ilink::ShortSaleType::NULL_VALUE,
                  .discretion_price = NaN,
                  .reservation_price = NaN,
              };
              log::info("DEBUG new_order_single={}"sv, new_order_single);
              send(new_order_single);
            })) {
        } else {
          log::fatal("Unexpected: didn't find security for security_id={}"sv, security_id);
        }
      })) {
  } else {
    throw oms::Rejected{
        Origin::GATEWAY,
        Error::INVALID_SYMBOL,
        R"(No mapping for market_segment_id={} and symbol="{}")"sv,
        market_segment_id_,
        order.symbol};
  }
}

// note!
//   undoc: execution mode must be x0 (NULL results in reject reason 109)
void OrderEntry::send_order_cancel_replace_request(ModifyOrder const &modify_order, oms::Order const &order) {
  log::info("DEBUG modify_order={}"sv, modify_order);
  log::info("DEBUG order={}"sv, order);
  if (shared_.find_security_id(market_segment_id_, order.symbol, [&](auto security_id) {
        log::info("DEBUG found security_id={}"sv, security_id);
        auto order_qty = get_quantity(modify_order.quantity);
        auto side = map(order.side);
        auto order_request_id = encode_order_request_id(RequestType::MODIFY_ORDER, order.user_id, order.order_id);
        auto ord_type = map(order.order_type);
        auto time_in_force = map(order.time_in_force);
        if (!party_details_list_req_id_)
          send_party_details_definition_request();
        auto now = clock::get_realtime();
        auto order_cancel_replace_request = ilink::OrderCancelReplaceRequest{
            .price = modify_order.price,
            .order_qty = order_qty,
            .security_id = security_id,
            .side = side,
            .seq_num = fetch_next_seq_num(),
            .sender_id = account_.get_name(),
            .cl_ord_id = order.client_order_id,
            .party_details_list_req_id = party_details_list_req_id_,
            .order_id = {},
            .stop_px = NaN,
            .order_request_id = order_request_id,
            .sending_time_epoch = now,
            .location = shared_.settings.ilink.location,
            .min_qty = {},
            .display_qty = {},
            .expire_date = {},
            .ord_type = ord_type,
            .time_in_force = time_in_force,
            .manual_order_indicator = MANUAL_ORDER_INDICATOR,
            .ofm_override = cme_ilink::OFMOverrideReq::Disabled,
            .exec_inst = {},                                               // XXX
            .execution_mode = static_cast<cme_ilink::ExecMode::Value>(0),  // note!
            .liquidity_flag = {},
            .managed_order = {},
            .short_sale_type = cme_ilink::ShortSaleType::NULL_VALUE,
            .discretion_price = NaN,
        };
        log::info("DEBUG order_cancel_replace_request={}"sv, order_cancel_replace_request);
        send(order_cancel_replace_request);
      })) {
  } else {
    throw oms::Rejected{
        Origin::GATEWAY,
        Error::INVALID_SYMBOL,
        R"(No mapping for market_segment_id={} and symbol="{}")"sv,
        market_segment_id_,
        order.symbol};
  }
}

void OrderEntry::send_order_cancel_request(CancelOrder const &cancel_order, oms::Order const &order) {
  log::info("DEBUG cancel_order={}"sv, cancel_order);
  log::info("DEBUG order={}"sv, order);
  if (shared_.find_security_id(market_segment_id_, order.symbol, [&](auto security_id) {
        log::info("DEBUG found security_id={}"sv, security_id);
        auto order_request_id = encode_order_request_id(RequestType::CANCEL_ORDER, order.user_id, order.order_id);
        auto side = map(order.side);
        if (!party_details_list_req_id_)
          send_party_details_definition_request();
        auto now = clock::get_realtime();
        auto order_cancel_request = ilink::OrderCancelRequest{
            .order_id = {},
            .party_details_list_req_id = party_details_list_req_id_,
            .manual_order_indicator = MANUAL_ORDER_INDICATOR,
            .seq_num = fetch_next_seq_num(),
            .sender_id = account_.get_name(),
            .cl_ord_id = order.client_order_id,
            .order_request_id = order_request_id,
            .sending_time_epoch = now,
            .location = shared_.settings.ilink.location,
            .security_id = security_id,
            .side = side,
            .liquidity_flag = {},
            .orig_order_user = {},
        };
        log::info("DEBUG order_cancel_request={}"sv, order_cancel_request);
        send(order_cancel_request);
      })) {
  } else {
    throw oms::Rejected{
        Origin::GATEWAY,
        Error::INVALID_SYMBOL,
        R"(No mapping for market_segment_id={} and symbol="{}")"sv,
        market_segment_id_,
        order.symbol};
  }
}

// note!
//   undoc: ord type must be x0 (NULL results in reject reason 109)
void OrderEntry::send_order_mass_action_request(CancelAllOrders const &) {
  if (!party_details_list_req_id_)
    send_party_details_definition_request();
  auto now = clock::get_realtime();
  auto order_mass_action_request = ilink::OrderMassActionRequest{
      .party_details_list_req_id = party_details_list_req_id_,
      .order_request_id = fetch_next_request_id(),  // XXX for what ???
      .manual_order_indicator = MANUAL_ORDER_INDICATOR,
      .seq_num = fetch_next_seq_num(),
      .sender_id = account_.get_name(),
      .sending_time_epoch = now,
      .security_group = {},
      .location = shared_.settings.ilink.location,
      .security_id = {},
      .mass_action_scope = cme_ilink::MassActionScope::MarketSegmentID,
      .market_segment_id = market_segment_id_,
      .mass_cancel_request_type = cme_ilink::MassCxlReqTyp::Account,
      .side = cme_ilink::SideNULL::NULL_VALUE,
      .ord_type = static_cast<cme_ilink::MassActionOrdTyp::Value>(0),  // note!
      .time_in_force = cme_ilink::MassCancelTIF::NULL_VALUE,
      .liquidity_flag = cme_ilink::BooleanNULL::NULL_VALUE,
      .orig_order_user = {},
  };
  log::info("DEBUG order_mass_action_request={}"sv, order_mass_action_request);
  send(order_mass_action_request);
}

template <typename Callback, typename... Args>
void OrderEntry::operator()(
    Callback callback, Trace<oms::OrderUpdate> const &event, std::string_view const &client_order_id, Args &&...args) {
  auto &[trace_info, order_update] = event;
  if (shared_.update_order(
          client_order_id, stream_id_, trace_info, order_update, std::forward<Args>(args)..., [&](auto &order) {
            callback(order);
          })) {
  } else {
    log::warn("*** EXTERNAL ORDER ***"sv);
  }
}

template <typename... Args>
void OrderEntry::operator()(
    Trace<oms::Response> const &event, std::string_view const &client_order_id, Args &&...args) {
  auto &[trace_info, response] = event;
  if (shared_.update_order(
          client_order_id,
          stream_id_,
          trace_info,
          response,
          std::forward<Args>(args)...,
          [&]([[maybe_unused]] auto &order) {})) {
  } else {
    log::warn("*** EXTERNAL ORDER ***"sv);
  }
}

template <typename... Args>
void OrderEntry::operator()(Trace<oms::Response> const &event, uint8_t user_id, uint64_t order_id, Args &&...args) {
  auto &[trace_info, response] = event;
  if (shared_.update_order(
          user_id,
          order_id,
          stream_id_,
          trace_info,
          response,
          std::forward<Args>(args)...,
          []([[maybe_unused]] auto &order) {})) {
  } else {
    log::warn("Did not find order: user_id={}, order_id={}"sv, user_id, order_id);
  }
}

}  // namespace cme
}  // namespace roq
