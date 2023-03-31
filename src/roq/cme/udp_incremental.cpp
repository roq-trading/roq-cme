/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/cme/udp_incremental.hpp"

#include <fmt/ranges.h>

#include "roq/utils/common.hpp"
#include "roq/utils/safe_cast.hpp"
#include "roq/utils/update.hpp"

#include "roq/debug/hex/message.hpp"

#include "roq/core/metrics/factory.hpp"

#include "roq/io/network_address.hpp"

#include "roq/cme/flags/common.hpp"
#include "roq/cme/flags/config.hpp"
#include "roq/cme/flags/multicast.hpp"

#include "roq/cme/mdp/utils.hpp"

using namespace std::literals;

namespace roq {
namespace cme {

// === CONSTANTS ===

namespace {
auto const NAME = "I"sv;
}  // namespace

// === HELPERS ===

namespace {
auto create_name(auto stream_id, auto const &channel_id) {
  return fmt::format("{}:{}{}"sv, stream_id, NAME, channel_id);
}

auto create_receiver(auto &handler, auto &context, auto &shared, auto &channel_id, Priority priority) {
  log::info(R"(Create channel_id="{}, priority={}")"sv, channel_id, priority);
  auto [multicast_address, port] = shared.get_multicast_config(channel_id, mdp::ConnectionType::INCREMENTAL, priority);
  log::info("Create multicast receiver port={}"sv, port);
  auto network_address = io::NetworkAddress{port};
  auto socket_options = Mask{
      io::SocketOption::REUSE_ADDRESS,
  };
  auto receiver = context.create_udp_receiver(handler, network_address, socket_options);
  log::info(R"(Local interface is "{}")"sv, flags::Multicast::multicast_local_interface());
  auto local_interface = io::NetworkAddress::create_blocking(flags::Multicast::multicast_local_interface());
  log::info(R"(Add membership "{}")"sv, multicast_address);
  auto multicast_address_2 = io::NetworkAddress::create_blocking(multicast_address);
  (*receiver).add_membership(multicast_address_2, local_interface);
  return receiver;
}

struct create_metrics final : public core::metrics::Factory {
  explicit create_metrics(auto const &group, auto const &function)
      : core::metrics::Factory(server::Flags::name(), group, function) {}
};

auto get_supports() {
  auto result = Mask{
      SupportType::REFERENCE_DATA,
      SupportType::MARKET_STATUS,
      SupportType::TOP_OF_BOOK,
      SupportType::MARKET_BY_PRICE,
      SupportType::TRADE_SUMMARY,
      SupportType::STATISTICS,
  };
  if (flags::Common::enable_market_by_order())
    result |= SupportType::MARKET_BY_ORDER;
  return result;
}

struct SecurityIterator final {
  SecurityIterator(Shared &shared) : shared_(shared) {}

  template <typename T, typename Dispatch, typename Callback>
  void operator()(T &value, Dispatch dispatch, Callback callback) {
    value.forEach([&](auto &item) {
      auto security_id = item.securityID();
      if (security_id != security_id_) {
        if (security_)
          dispatch(security_id, *security_);
        security_id_ = security_id;
        if (shared_.get_security(security_id, [&](auto &security) { security_ = &security; })) {
        } else {
          security_ = nullptr;
        }
      }
      if (security_)
        callback(*security_, item);
    });
    if (security_)
      dispatch(security_id_, *security_);
  }

 private:
  Shared &shared_;
  int32_t security_id_ = {};
  tools::Security *security_ = nullptr;
};

// following are used from several places

template <typename T>
void mbp_emplace_back(auto &result, T const &item, auto &security) {
  auto price = mdp::get_double(const_cast<T &>(item).mDEntryPx());
  auto quantity = mdp::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue());
  auto number_of_orders = mdp::get_int(item.numberOfOrders(), item.numberOfOrdersNullValue());
  auto update_action = [&]() -> UpdateAction {
    constexpr bool has_md_update_action = requires(T const &t) { t.mDUpdateAction(); };
    if constexpr (has_md_update_action) {
      return mdp::map(item.mDUpdateAction());
    }
    return {};
  }();
  if (update_action == UpdateAction::DELETE)
    quantity = {};  // note! exchange gives us the *old* value / we need this to be zero
  auto md_price_level = mdp::get_int(item.mDPriceLevel(), item.mDPriceLevelNullValue());
  uint32_t price_level = md_price_level > 0 ? (md_price_level - 1) : 0;
  auto update = MBPUpdate{
      .price = price * security.display_factor,
      .quantity = utils::safe_cast(quantity),
      .implied_quantity = NaN,
      .number_of_orders = utils::safe_cast(number_of_orders),
      .update_action = update_action,
      .price_level = price_level,
  };
  result.emplace_back(std::move(update));
}

template <typename T>
void trades_emplace_back(auto &result, T const &item, auto &security) {
  auto side = mdp::map_side(item.aggressorSide());
  auto price = mdp::get_double(const_cast<T &>(item).mDEntryPx());
  auto quantity = mdp::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue());
  auto trade = Trade{
      .side = side,
      .price = price * security.display_factor,
      .quantity = utils::safe_cast(quantity),
      .trade_id = {},
      .taker_order_id = {},
      .maker_order_id = {},
  };
  auto trade_id = mdp::get_int(item.mDTradeEntryID(), item.mDTradeEntryIDNullValue());
  fmt::format_to(std::back_inserter(trade.trade_id), "{}"sv, trade_id);
  result.emplace_back(std::move(trade));
}

template <typename T>
void statistics_emplace_back_price(auto &result, auto type, T const &item, auto factor) {
  auto value = mdp::get_double(const_cast<T &>(item).mDEntryPx());
  auto statistics = Statistics{
      .type = type,
      .value = value * factor,
      .begin_time_utc = {},
      .end_time_utc = {},
  };
  result.emplace_back(std::move(statistics));
}

void statistics_emplace_back_size(auto &result, auto type, auto const &item) {
  auto value = mdp::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue());
  auto statistics = Statistics{
      .type = type,
      .value = utils::safe_cast(value),
      .begin_time_utc = {},
      .end_time_utc = {},
  };
  result.emplace_back(std::move(statistics));
}

template <typename T>
void statistics_emplace_back(auto &result, T const &item, auto &security) {
  auto statistics_type = mdp::map(item.mDEntryType());
  if (statistics_type == StatisticsType::OPEN_INTEREST) {
    statistics_emplace_back_size(result, statistics_type, item);
  } else {
    statistics_emplace_back_price(result, statistics_type, item, security.display_factor);
  }
}

void emplace_back(
    cme_mdp::MDIncrementalRefreshBook46::NoMDEntries const &item, auto &security, auto &layer, auto &bids, auto &asks) {
  using value_type = typename std::remove_cvref<decltype(item)>::type;
  switch (item.mDEntryType()) {
    using enum cme_mdp::MDEntryTypeBook::Value;
    case Bid:
      mbp_emplace_back(bids, item, security);
      break;
    case Offer:
      mbp_emplace_back(asks, item, security);
      break;
    case ImpliedBid:
      break;
    case ImpliedOffer:
      break;
    case BookReset:  // XXX ????????????????????????
      break;
    case MarketBestOffer: {
      auto price = mdp::get_double(const_cast<value_type &>(item).mDEntryPx());
      layer.ask_price = price * security.display_factor;
      break;
    }
    case MarketBestBid: {
      auto price = mdp::get_double(const_cast<value_type &>(item).mDEntryPx());
      layer.bid_price = price * security.display_factor;
      break;
    }
    case NULL_VALUE:
      break;
  }
}

void emplace_back(
    cme_mdp::MDIncrementalRefreshBook46::NoOrderIDEntries const &item,
    auto &security,
    auto side,
    auto price,
    auto &orders) {
  auto action = mdp::map(item.orderUpdateAction());
  auto quantity = mdp::get_int(item.mDDisplayQty(), item.mDDisplayQtyNullValue());
  auto priority = mdp::get_int(item.mDOrderPriority(), item.mDOrderPriorityNullValue());
  auto order_id = mdp::get_int(item.orderID(), item.orderIDNullValue());
  auto order = MBOUpdate{
      .price = price * security.display_factor,
      .quantity = static_cast<double>(quantity),
      .priority = priority,
      .order_id = {},
      .side = side,
      .action = action,
      .reason = {},
  };
  fmt::format_to(std::back_inserter(order.order_id), "{}"sv, order_id);
  if (action != UpdateAction::DELETE && !quantity) [[unlikely]]  // DEBUG
    log::warn("Unexpected: update={}"sv, order);
  orders.emplace_back(std::move(order));
}

void emplace_back(cme_mdp::MDIncrementalRefreshOrderBook47::NoMDEntries const &item, auto &security, auto &orders) {
  using value_type = typename std::remove_cvref<decltype(item)>::type;
  auto create_update = [&](auto side) {
    auto price = mdp::get_double(const_cast<value_type &>(item).mDEntryPx());
    auto quantity = mdp::get_int(item.mDDisplayQty(), item.mDDisplayQtyNullValue());
    auto priority = mdp::get_int(item.mDOrderPriority(), item.mDOrderPriorityNullValue());
    auto order_id = mdp::get_int(item.orderID(), item.orderIDNullValue());
    auto action = mdp::map(item.mDUpdateAction());
    auto result = MBOUpdate{
        .price = price * security.display_factor,
        .quantity = static_cast<double>(quantity),
        .priority = priority,
        .order_id = {},
        .side = side,
        .action = action,
        .reason = {},
    };
    fmt::format_to(std::back_inserter(result.order_id), "{}"sv, order_id);
    if (action != UpdateAction::DELETE && !quantity) [[unlikely]]  // DEBUG
      log::warn("Unexpected: update={}"sv, result);
    return result;
  };
  using value_type = typename std::remove_cvref<decltype(item)>::type;
  switch (item.mDEntryType()) {
    using enum cme_mdp::MDEntryTypeBook::Value;
    case Bid: {
      auto update = create_update(Side::BUY);
      orders.emplace_back(std::move(update));
      break;
    }
    case Offer: {
      auto update = create_update(Side::SELL);
      orders.emplace_back(std::move(update));
      break;
    }
    case ImpliedBid:
      break;
    case ImpliedOffer:
      break;
    case BookReset:  // XXX ????????????????????????
      break;
    case MarketBestOffer:
      break;
    case MarketBestBid:
      break;
    case NULL_VALUE:
      break;
  }
}
}  // namespace

// === IMPLEMENTATION ===

UDPIncremental::UDPIncremental(
    Handler &handler, io::Context &context, uint16_t stream_id, Shared &shared, Channel &channel, Priority priority)
    : handler_{handler}, stream_id_{stream_id}, name_{create_name(stream_id_, channel.channel_id)},
      market_by_order_{flags::Common::enable_market_by_order()},
      receiver_{create_receiver(*this, context, shared, channel.channel_id, priority)},
      counter_{
          .disconnect = create_metrics(name_, "disconnect"sv),
          .sequence_reset = create_metrics(name_, "sequence_reset"sv),
      },
      profile_{
          .parse = create_metrics(name_, "parse"sv),
          .admin_heartbeat = create_metrics(name_, "admin_heartbeat"sv),
          .channel_reset = create_metrics(name_, "channel_reset"sv),
          .security_status = create_metrics(name_, "security_status"sv),
          .md_instrument_definition_future = create_metrics(name_, "md_instrument_definition_future"sv),
          .md_instrument_definition_option = create_metrics(name_, "md_instrument_definition_option"sv),
          .md_instrument_definition_spread = create_metrics(name_, "md_instrument_definition_spread"sv),
          .md_instrument_definition_fixed_income = create_metrics(name_, "md_instrument_definition_fixed_income"sv),
          .md_instrument_definition_repo = create_metrics(name_, "md_instrument_definition_repo"sv),
          .md_instrument_definition_fx = create_metrics(name_, "md_instrument_definition_fx"sv),
          .md_incremental_refresh_book = create_metrics(name_, "md_incremental_refresh_book"sv),
          .md_incremental_refresh_book_long_qty = create_metrics(name_, "md_incremental_refresh_book_long_qty"sv),
          .md_incremental_refresh_order_book = create_metrics(name_, "md_incremental_refresh_order_book"sv),
          .md_incremental_refresh_trade_summary = create_metrics(name_, "md_incremental_refresh_trade_summary"sv),
          .md_incremental_refresh_trade_summary_long_qty =
              create_metrics(name_, "md_incremental_refresh_trade_summary_long_qty"sv),
          .md_incremental_refresh_daily_statistics = create_metrics(name_, "md_incremental_refresh_daily_statistics"sv),
          .md_incremental_refresh_session_statistics =
              create_metrics(name_, "md_incremental_refresh_session_statistics"sv),
          .md_incremental_refresh_session_statistics_long_qty =
              create_metrics(name_, "md_incremental_refresh_session_statistics_long_qty"sv),
          .md_incremental_refresh_volume = create_metrics(name_, "md_incremental_refresh_volume"sv),
          .md_incremental_refresh_volume_long_qty = create_metrics(name_, "md_incremental_refresh_volume_long_qty"sv),
          .md_incremental_refresh_limits_banding = create_metrics(name_, "md_incremental_refresh_limits_banding"sv),
      },
      shared_{shared}, channel_{channel} {
}

void UDPIncremental::operator()(Event<Start> const &) {
  TraceInfo trace_info;
  publish_stream_status(trace_info, ConnectionStatus::CONNECTING);
  last_update_time_ = trace_info.source_receive_time;
}

void UDPIncremental::operator()(Event<Stop> const &) {
}

void UDPIncremental::operator()(Event<Timer> const &event) {
  if (last_update_time_.count() && (last_update_time_ + flags::Multicast::multicast_timeout()) < event.value.now) {
    log::warn("*** DETECTED TIMEOUT ***"sv);
    last_update_time_ = {};
  }
}

namespace {
// this is general logic
void drain(auto &receiver, auto &channel, auto stream_id, auto parse, auto reset) {
  using value_type = typename decltype(channel.incremental.buffer)::value_type;
  for (auto stop = false; !stop;) {
    if (channel.incremental.buffer.next([&](auto buffer) -> std::pair<size_t, value_type> {
          // read into buffer
          auto bytes = receiver.recv(buffer);
          log::info<5>("Received {} byte(s) (stream_id={})"sv, bytes, stream_id);
          if (!bytes) {
            stop = true;
            return {};
          }
          // parse message
          std::span message{std::data(buffer), bytes};
          log::info<5>("{}"sv, debug::hex::Message{message});
          bool hold = false, drop = false;
          value_type sequence_number = {};
          if (mdp::Frame::parse(message, [&](auto &frame) {
                log::info<5>("frame={}, last_sequence_number={}"sv, frame, channel.sequence.last_sequence_number);
                // check sequence number
                sequence_number = frame.sequence_number;
                if (sequence_number < flags::Common::filter_snapshot_from_incremental()) {
                  drop = true;
                  return;
                }
                if (channel.sequence.first_sequence_number) {
                  auto next_sequence_number = channel.sequence.last_sequence_number + 1;
                  hold = sequence_number > next_sequence_number;
                  drop = sequence_number < next_sequence_number;
                }
                if (hold) {
                  auto delta = static_cast<int64_t>(sequence_number) -
                               static_cast<int64_t>(channel.sequence.last_sequence_number);
                  log::warn(
                      "DEBUG HOLD sequence_number={}, last_sequence_number={}, delta={}"sv,
                      sequence_number,
                      channel.sequence.last_sequence_number,
                      delta);
                }
                // TEST >>>
                if (flags::Common::test_drop() && !hold && !drop) {
                  if ((sequence_number % flags::Common::test_drop()) == 0) {
                    log::warn("DEBUG: *** SIMULATE DROP ***"sv);
                    drop = true;
                  }
                }
                if (flags::Common::test_reordering() && !hold && !drop) {
                  if ((sequence_number % flags::Common::test_reordering()) == 0) {
                    log::warn("DEBUG: *** SIMULATE REORDERING ***"sv);
                    hold = true;
                    stop = true;
                  }
                }
                // <<< TEST
              })) {
            log::info<5>("hold={}, drop={}"sv, hold, drop);
            if (drop)
              return {};
            if (hold)
              return {bytes, sequence_number};
            // parse this message
            channel.update_sequence_number(sequence_number);
            parse(message);
            return {};
          } else {
            // failed to parse frame
            log::warn("Unexpected"sv);
            return {};  // XXX not sure what to do here
          }
        })) {
      // successfully parsed a message
      // now process any withheld messages
      while (!stop) {
        if (channel.sequence.first_sequence_number) {
          // check next sequence number
          auto sequence_number = channel.sequence.last_sequence_number + 1;
          if (channel.incremental.buffer.get(sequence_number, [&](auto &message) {
                // parse this message
                channel.update_sequence_number(sequence_number);
                parse(message);
              })) {
          } else {
            // does not exist
            stop = true;
          }
        } else {
          // not ready
          stop = true;
        }
      }
    } else {
      // full: no available buffer
      log::warn<0>("*** BUFFER FULL ***"sv);  // XXX should be log level 1
      channel.incremental.buffer.clear();
      reset();
      // XXX resubscribe
    }
  }
}
}  // namespace

void UDPIncremental::operator()(io::net::udp::Receiver::Read const &) {
  TraceInfo trace_info;
  last_update_time_ = trace_info.source_receive_time;
  publish_stream_status(trace_info, ConnectionStatus::READY);  // first message will publish
  auto parse = [&](auto &message) {
    log_this_message_ = false;  // DEBUG
    if (!mdp::Parser::dispatch(*this, message, trace_info)) {
      log::warn("{}"sv, debug::hex::Message{message});
      log::fatal("Failed to parse message"sv);
    }
    if (log_this_message_) {  // DEBUG
      log::info("{}"sv, debug::hex::Message{message});
    }
  };
  drain(*receiver_, channel_, stream_id_, parse, [&]() { on_sequence_reset(); });
}

void UDPIncremental::on_sequence_reset() {
  log::warn<0>("*** SEQUENCE RESET ***"sv);  // XXX should be log level 1
  ++counter_.sequence_reset;
  channel_.sequence = {};
}

void UDPIncremental::operator()(io::net::udp::Receiver::Error const &error) {
  log::fatal("Error: what={}"sv, error.what);
}

// mdp::Parser::Handler

void UDPIncremental::operator()(mdp::Frame const &frame) {
  channel_.incremental.last_sequence = {true, frame.sequence_number};
}

void UDPIncremental::operator()(Trace<cme_mdp::AdminHeartbeat12> const &event, mdp::Frame const &frame) {
  profile_.admin_heartbeat([&]() {
    auto &[trace_info, value] = event;
    log::info<5>("admin_heartbeat_12={}, frame={}"sv, value, frame);
    auto external_latency = ExternalLatency{
        .stream_id = stream_id_,
        .account = {},
        .latency = trace_info.origin_create_time_utc - frame.sending_time,
    };
    create_trace_and_dispatch(handler_, trace_info, external_latency);
  });
}

void UDPIncremental::operator()(Trace<cme_mdp::ChannelReset4> const &event, mdp::Frame const &frame) {
  profile_.channel_reset([&]() {
    auto &[trace_info, value] = event;
    log::info<5>("channel_reset_4={}, frame={}"sv, value, frame);
  });
}

void UDPIncremental::operator()(Trace<cme_mdp::SecurityStatus30> const &event, mdp::Frame const &frame) {
  profile_.security_status([&]() {
    auto &trace_info = event.trace_info;
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("security_status_30={}, frame={}"sv, value, frame);
    value.sbeRewind();  // note!
    auto security_id = mdp::get_int(value.securityID(), value.securityIDNullValue());
    auto trading_status = mdp::map_security_trading_status(value.securityTradingStatus());
    auto dispatch = [&](auto security_id) {
      shared_.get_security(security_id, [&](auto &security) {
        auto market_status = MarketStatus{
            .stream_id = stream_id_,
            .exchange = security.exchange,
            .symbol = security.symbol,
            .trading_status = trading_status,
        };
        create_trace_and_dispatch(handler_, trace_info, market_status, true);
      });
    };
    if (security_id) {
      dispatch(security_id);
    } else {
      auto security_group = mdp::get_string_view(value.securityGroup(), value.securityGroupLength());
      shared_.get_security_group(security_group, [&](auto security_id) { dispatch(security_id); });
    }
  });
}

void UDPIncremental::operator()(Trace<cme_mdp::MDInstrumentDefinitionFuture54> const &event, mdp::Frame const &frame) {
  profile_.md_instrument_definition_future([&]() {
    auto &trace_info = event.trace_info;
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("md_instrument_definition_future_54={}, frame={}"sv, value, frame);
    value.sbeRewind();  // note!
    auto security_id = value.securityID();
    shared_.get_security_incl_discard(security_id, [&](auto &security) {
      auto reference_data = mdp::create_reference_data(value, stream_id_, security);
      create_trace_and_dispatch(handler_, trace_info, reference_data, true);
      if (security.discard)
        return;
      auto market_status = mdp::create_market_status(value, stream_id_, security);
      create_trace_and_dispatch(handler_, trace_info, market_status, true);
    });
  });
}

void UDPIncremental::operator()(Trace<cme_mdp::MDInstrumentDefinitionOption55> const &event, mdp::Frame const &frame) {
  profile_.md_instrument_definition_option([&]() {
    auto &trace_info = event.trace_info;
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("md_instrument_definition_option_55={}, frame={}"sv, value, frame);
    value.sbeRewind();  // note!
    auto security_id = value.securityID();
    shared_.get_security_incl_discard(security_id, [&](auto &security) {
      auto reference_data = mdp::create_reference_data(value, stream_id_, security);
      create_trace_and_dispatch(handler_, trace_info, reference_data, true);
      if (security.discard)
        return;
      auto market_status = mdp::create_market_status(value, stream_id_, security);
      create_trace_and_dispatch(handler_, trace_info, market_status, true);
    });
  });
}

void UDPIncremental::operator()(Trace<cme_mdp::MDInstrumentDefinitionSpread56> const &event, mdp::Frame const &frame) {
  profile_.md_instrument_definition_spread([&]() {
    auto &trace_info = event.trace_info;
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("md_instrument_definition_spread_56={}, frame={}"sv, value, frame);
    value.sbeRewind();  // note!
    auto security_id = value.securityID();
    shared_.get_security_incl_discard(security_id, [&](auto &security) {
      auto reference_data = mdp::create_reference_data(value, stream_id_, security);
      create_trace_and_dispatch(handler_, trace_info, reference_data, true);
      if (security.discard)
        return;
      auto market_status = mdp::create_market_status(value, stream_id_, security);
      create_trace_and_dispatch(handler_, trace_info, market_status, true);
    });
  });
}

void UDPIncremental::operator()(
    Trace<cme_mdp::MDInstrumentDefinitionFixedIncome57> const &event, mdp::Frame const &frame) {
  profile_.md_instrument_definition_fixed_income([&]() {
    auto &trace_info = event.trace_info;
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("md_instrument_definition_fixed_income_57={}, frame={}"sv, value, frame);
    value.sbeRewind();  // note!
    auto security_id = value.securityID();
    shared_.get_security_incl_discard(security_id, [&](auto &security) {
      auto reference_data = mdp::create_reference_data(value, stream_id_, security);
      create_trace_and_dispatch(handler_, trace_info, reference_data, true);
      if (security.discard)
        return;
      auto market_status = mdp::create_market_status(value, stream_id_, security);
      create_trace_and_dispatch(handler_, trace_info, market_status, true);
    });
  });
}

void UDPIncremental::operator()(Trace<cme_mdp::MDInstrumentDefinitionRepo58> const &event, mdp::Frame const &frame) {
  profile_.md_instrument_definition_repo([&]() {
    auto &trace_info = event.trace_info;
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("md_instrument_definition_repo_58={}, frame={}"sv, value, frame);
    value.sbeRewind();  // note!
    auto security_id = value.securityID();
    shared_.get_security_incl_discard(security_id, [&](auto &security) {
      auto reference_data = mdp::create_reference_data(value, stream_id_, security);
      create_trace_and_dispatch(handler_, trace_info, reference_data, true);
      if (security.discard)
        return;
      auto market_status = mdp::create_market_status(value, stream_id_, security);
      create_trace_and_dispatch(handler_, trace_info, market_status, true);
    });
  });
}

void UDPIncremental::operator()(Trace<cme_mdp::MDInstrumentDefinitionFX63> const &event, mdp::Frame const &frame) {
  profile_.md_instrument_definition_fx([&]() {
    auto &trace_info = event.trace_info;
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("md_instrument_definition_fx_63={}, frame={}"sv, value, frame);
    value.sbeRewind();  // note!
    auto security_id = value.securityID();
    shared_.get_security_incl_discard(security_id, [&](auto &security) {
      auto reference_data = mdp::create_reference_data(value, stream_id_, security);
      create_trace_and_dispatch(handler_, trace_info, reference_data, true);
      if (security.discard)
        return;
      auto market_status = mdp::create_market_status(value, stream_id_, security);
      create_trace_and_dispatch(handler_, trace_info, market_status, true);
    });
  });
}

void UDPIncremental::operator()(Trace<cme_mdp::SnapshotFullRefresh52> const &, mdp::Frame const &) {
}

void UDPIncremental::operator()(Trace<cme_mdp::SnapshotFullRefreshLongQty69> const &, mdp::Frame const &) {
}

void UDPIncremental::operator()(Trace<cme_mdp::QuoteRequest39> const &, mdp::Frame const &) {
}

void UDPIncremental::operator()(Trace<cme_mdp::MDIncrementalRefreshBook46> const &event, mdp::Frame const &frame) {
  profile_.md_incremental_refresh_book([&]() {
    auto &trace_info = event.trace_info;
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("md_incremental_refresh_book_46={}, frame={}"sv, value, frame);
    value.sbeRewind();  // note!
    auto exchange_sequence = frame.sequence_number;
    auto exchange_time_utc = std::chrono::nanoseconds{value.transactTime()};
    // note! MBO contains indexed references to MBP entries
    md_entries_.clear();
    {  // MBP
      Layer layer;
      auto &mbp = shared_.get_mbp();
      auto &mbo = shared_.get_mbo();
      auto dispatch = [&](auto security_id, auto &security, auto is_last) {
        if (!(std::isnan(layer.bid_price) && std::isnan(layer.ask_price))) {
          auto top_of_book = TopOfBook{
              .stream_id = stream_id_,
              .exchange = security.exchange,
              .symbol = security.symbol,
              .layer = layer,
              .exchange_time_utc = exchange_time_utc,
              .exchange_sequence = exchange_sequence,
          };
          create_trace_and_dispatch(handler_, trace_info, std::as_const(top_of_book), is_last);
          layer = {};
        }
        if (!std::empty(mbp)) {
          dispatch_market_by_price(
              trace_info,
              security_id,
              security,
              exchange_sequence,
              exchange_time_utc,
              frame.sending_time,
              mbp.bids,
              mbp.asks);
          mbp.clear();
        }
        if (!std::empty(mbo)) {
          dispatch_market_by_order(
              trace_info, security_id, security, exchange_sequence, exchange_time_utc, frame.sending_time, mbo.orders);
          mbo.clear();
        }
      };
      // HANS here we need to append every item regardless of security ==> can't use SecurityIterator
      auto security_id = int32_t{};
      tools::Security *security = nullptr;
      value.noMDEntries().forEach([&](auto const &item) {
        auto current_security_id = item.securityID();
        if (current_security_id != security_id) {
          if (security)
            dispatch(security_id, *security, true);
          security_id = current_security_id;
          if (shared_.get_security(security_id, [&security](auto &security_2) { security = &security_2; })) {
          } else {
            security = nullptr;
          }
        }
        if (security)
          check_report_sequence(*security, item, frame);
        using value_type = typename std::remove_cvref<decltype(item)>::type;
        auto &value = const_cast<value_type &>(item);  // note! not const-safe
        auto price = mdp::get_double(value.mDEntryPx());
        auto side = mdp::map(item.mDEntryType());
        auto action = mdp::map(item.mDUpdateAction());
        // ... need these for MBO referencing
        if (market_by_order_)
          md_entries_.emplace_back(security_id, side, price, action);
        if (security) {
          emplace_back(item, *security, layer, mbp.bids, mbp.asks);
          if (market_by_order_) {
            if (action == UpdateAction::DELETE && side != Side::UNDEFINED &&
                flags::Common::test_mbp_to_mbo_clear_price_level()) {
              auto update = MBOUpdate{
                  .price = price * (*security).display_factor,
                  .quantity = {},
                  .priority = {},
                  .order_id = {},
                  .side = side,
                  .action = action,
                  .reason = {},
              };
              mbo.orders.emplace_back(std::move(update));
            }
          }
        }
      });
      if (security)
        dispatch(security_id, *security, true);
    }
    if (market_by_order_) {
      auto security_id = int32_t{};
      tools::Security *security = nullptr;
      auto &mbo = shared_.get_mbo();
      auto dispatch = [&](auto security_id, auto &security) {
        if (!std::empty(mbo)) {
          dispatch_market_by_order(
              trace_info, security_id, security, exchange_sequence, exchange_time_utc, frame.sending_time, mbo.orders);
          mbo.clear();
        }
      };
      value.noOrderIDEntries().forEach([&](auto const &item) {
        auto reference_id = mdp::get_int(item.referenceID(), item.referenceIDNullValue());
        if (!reference_id)
          return;
        auto index = static_cast<size_t>(reference_id) - 1;  // indexing is 1-based
        if (!(index < std::size(md_entries_))) [[unlikely]] {
          log::warn("Unexpected: index={}, len={}"sv, index, std::size(md_entries_));
          return;
        }
        auto [current_security_id, side, price, action] = md_entries_[index];
        if (current_security_id != security_id) {
          if (security)
            dispatch(security_id, *security);
          security_id = current_security_id;
          if (shared_.get_security(security_id, [&security](auto &security_2) { security = &security_2; })) {
          } else {
            security = nullptr;
          }
        }
        if (security) {
          if (action != UpdateAction::DELETE)
            emplace_back(item, *security, side, price, mbo.orders);
        }
      });
      if (security)
        dispatch(security_id, *security);
    }
  });
}

void UDPIncremental::operator()(
    Trace<cme_mdp::MDIncrementalRefreshBookLongQty64> const &event, mdp::Frame const &frame) {
  profile_.md_incremental_refresh_book_long_qty([&]() {
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("md_incremental_refresh_book_long_qty_64={}, frame={}"sv, value, frame);
    auto dispatch = []([[maybe_unused]] auto security_id, [[maybe_unused]] auto &security) {};
    auto update = [&](auto &security, auto &item) { check_report_sequence(security, item, frame); };
    SecurityIterator{shared_}(value.noMDEntries(), dispatch, update);
  });
}

void UDPIncremental::operator()(Trace<cme_mdp::SnapshotFullRefreshOrderBook53> const &, mdp::Frame const &) {
}

void UDPIncremental::operator()(Trace<cme_mdp::MDIncrementalRefreshOrderBook47> const &event, mdp::Frame const &frame) {
  profile_.md_incremental_refresh_order_book([&]() {
    auto &trace_info = event.trace_info;
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("md_incremental_refresh_order_book_47={}, frame={}"sv, value, frame);
    if (!market_by_order_)
      return;
    value.sbeRewind();  // note!
    auto exchange_sequence = frame.sequence_number;
    auto exchange_time_utc = std::chrono::nanoseconds{value.transactTime()};
    auto &mbo = shared_.get_mbo();
    auto dispatch = [&](auto security_id, auto &security) {
      if (std::empty(mbo))
        return;
      dispatch_market_by_order(
          trace_info, security_id, security, exchange_sequence, exchange_time_utc, frame.sending_time, mbo.orders);
      mbo.clear();
    };
    auto update = [&](auto &security, auto &item) { emplace_back(item, security, mbo.orders); };
    SecurityIterator{shared_}(value.noMDEntries(), dispatch, update);
  });
}

void UDPIncremental::operator()(
    Trace<cme_mdp::MDIncrementalRefreshTradeSummary48> const &event, mdp::Frame const &frame) {
  profile_.md_incremental_refresh_trade_summary([&]() {
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("md_incremental_refresh_trade_summary_48={}, frame={}"sv, value, frame);
    dispatch_trade_summary(event, frame);
  });
}

void UDPIncremental::operator()(
    Trace<cme_mdp::MDIncrementalRefreshTradeSummaryLongQty65> const &event, mdp::Frame const &frame) {
  profile_.md_incremental_refresh_trade_summary_long_qty([&]() {
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("md_incremental_refresh_trade_summary_long_qty_65={}, frame={}"sv, value, frame);
    dispatch_trade_summary(event, frame);
  });
}

void UDPIncremental::operator()(
    Trace<cme_mdp::MDIncrementalRefreshDailyStatistics49> const &event, mdp::Frame const &frame) {
  profile_.md_incremental_refresh_daily_statistics([&]() {
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("md_incremental_refresh_daily_statistics_49={}, frame={}"sv, value, frame);
    dispatch_statistics(event, frame, [](auto &statistics, auto &item, auto &security) {
      statistics_emplace_back(statistics, item, security);
    });
  });
}

void UDPIncremental::operator()(
    Trace<cme_mdp::MDIncrementalRefreshSessionStatistics51> const &event, mdp::Frame const &frame) {
  profile_.md_incremental_refresh_session_statistics([&]() {
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("md_incremental_refresh_session_statistics_51={}, frame={}"sv, value, frame);
    dispatch_statistics(event, frame, [](auto &statistics, auto &item, auto &security) {
      statistics_emplace_back(statistics, item, security);
    });
  });
}

void UDPIncremental::operator()(
    Trace<cme_mdp::MDIncrementalRefreshSessionStatisticsLongQty67> const &event, mdp::Frame const &frame) {
  profile_.md_incremental_refresh_session_statistics_long_qty([&]() {
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("md_incremental_refresh_session_statistics_long_qty_67={}, frame={}"sv, value, frame);
    dispatch_statistics(event, frame, [](auto &statistics, auto &item, auto &security) {
      statistics_emplace_back(statistics, item, security);
    });
  });
}

void UDPIncremental::operator()(Trace<cme_mdp::MDIncrementalRefreshVolume37> const &event, mdp::Frame const &frame) {
  profile_.md_incremental_refresh_volume([&]() {
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("md_incremental_refresh_volume_37={}, frame={}"sv, value, frame);
    dispatch_statistics(event, frame, [](auto &statistics, auto &item, [[maybe_unused]] auto &security) {
      statistics_emplace_back_size(statistics, StatisticsType::TRADE_VOLUME, item);
    });
  });
}

void UDPIncremental::operator()(
    Trace<cme_mdp::MDIncrementalRefreshVolumeLongQty66> const &event, mdp::Frame const &frame) {
  profile_.md_incremental_refresh_volume_long_qty([&]() {
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("md_incremental_refresh_volume_long_qty_66={}, frame={}"sv, value, frame);
    auto dispatch = []([[maybe_unused]] auto security_id, [[maybe_unused]] auto &security) {};
    auto update = [&](auto &security, auto &item) { check_report_sequence(security, item, frame); };
    SecurityIterator{shared_}(value.noMDEntries(), dispatch, update);
  });
}

void UDPIncremental::operator()(
    Trace<cme_mdp::MDIncrementalRefreshLimitsBanding50> const &event, mdp::Frame const &frame) {
  profile_.md_incremental_refresh_limits_banding([&]() {
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("md_incremental_refresh_limits_banding_50={}, frame={}"sv, value, frame);
    auto dispatch = []([[maybe_unused]] auto security_id, [[maybe_unused]] auto &security) {};
    auto update = [&](auto &security, auto &item) { check_report_sequence(security, item, frame); };
    SecurityIterator{shared_}(value.noMDEntries(), dispatch, update);
  });
}

void UDPIncremental::dispatch_market_by_price(
    auto &trace_info,
    auto security_id,
    auto &security,
    auto exchange_sequence,
    auto exchange_time_utc,
    auto sending_time_utc,
    auto &bids,
    auto &asks) {
  auto &sequencer = security.mbp.sequencer;
  try {
    auto last_exchange_sequence = sequencer.last_sequence();  // note! the protocol doesn't tell us
    auto create_update = [&](auto &bids, auto &asks, auto update_type) -> MarketByPriceUpdate {
      return {
          .stream_id = stream_id_,
          .exchange = security.exchange,
          .symbol = security.symbol,
          .bids = bids,
          .asks = asks,
          .update_type = update_type,
          .exchange_time_utc = exchange_time_utc,
          .exchange_sequence = exchange_sequence,
          .sending_time_utc = sending_time_utc,
          .price_decimals = {},
          .quantity_decimals = {},
          .checksum = {},
      };
    };
    auto publish_update = [&](auto &bids, auto &asks) {
      auto market_by_price_update = create_update(bids, asks, UpdateType::INCREMENTAL);
      create_trace_and_dispatch(handler_, trace_info, market_by_price_update, true);
    };
    auto publish_snapshot = [&](auto &bids, auto &asks, [[maybe_unused]] auto exchange_sequence) {
      log::info(
          R"(PUBLISH MBP SNAPSHOT exchange="{}", symbol="{}", exchange_sequence={})"sv,
          security.exchange,
          security.symbol,
          exchange_sequence);
      auto market_by_price_update = create_update(bids, asks, UpdateType::SNAPSHOT);
      create_trace_and_dispatch(handler_, trace_info, market_by_price_update, true);
      security.mbp.resubscribe = {};
    };
    auto request_snapshot = [&]([[maybe_unused]] auto retries) {
      log::info(
          R"(REQUEST MBP SNAPSHOT exchange="{}", symbol="{}", exchange_sequence={})"sv,
          security.exchange,
          security.symbol,
          exchange_sequence);
      security.mbp.resubscribe = exchange_sequence;
    };
    sequencer(
        bids,
        asks,
        exchange_sequence,
        exchange_sequence,
        last_exchange_sequence,
        publish_update,
        publish_snapshot,
        request_snapshot);
  } catch (BadState &) {
    log::warn(
        R"(RESUBSCRIBE MBP exchange="{}", symbol="{}", security_id={}, exchange_sequene={})"sv,
        security.exchange,
        security.symbol,
        security_id,
        exchange_sequence);
    // XXX HANS publish stale
    sequencer.clear();
    security.mbp.resubscribe = exchange_sequence;
  }
}

void UDPIncremental::dispatch_market_by_order(
    auto &trace_info,
    [[maybe_unused]] auto security_id,
    auto &security,
    auto exchange_sequence,
    auto exchange_time_utc,
    auto sending_time_utc,
    auto &orders) {
  auto &sequencer = security.mbo.sequencer;
  try {
    auto last_exchange_sequence = sequencer.last_sequence();  // note! the protocol doesn't tell us
    auto create_update = [&](auto &orders, auto update_type) -> MarketByOrderUpdate {
      return {
          .stream_id = stream_id_,
          .exchange = security.exchange,
          .symbol = security.symbol,
          .orders = orders,
          .update_type = update_type,
          .exchange_time_utc = exchange_time_utc,
          .exchange_sequence = exchange_sequence,
          .sending_time_utc = sending_time_utc,
          .price_decimals = {},
          .quantity_decimals = {},
          .max_depth = {},
          .checksum = {},
      };
    };
    auto publish_update = [&](auto &orders) {
      auto market_by_order_update = create_update(orders, UpdateType::INCREMENTAL);
      create_trace_and_dispatch(handler_, trace_info, market_by_order_update, true);
    };
    auto publish_snapshot = [&](auto &orders, [[maybe_unused]] auto exchange_sequence) {
      log::info(
          R"(PUBLISH MBO SNAPSHOT exchange={}, symbol="{}", exchange_sequence={})"sv,
          security.exchange,
          security.symbol,
          exchange_sequence);
      auto market_by_order_update = create_update(orders, UpdateType::SNAPSHOT);
      create_trace_and_dispatch(handler_, trace_info, market_by_order_update, true);
      security.mbo.resubscribe = {};
    };
    auto request_snapshot = [&]([[maybe_unused]] auto retries) {
      log::info(
          R"(REQUEST MBO SNAPSHOT exchange="{}", symbol="{}", exchange_sequence={}, retries={})"sv,
          security.exchange,
          security.symbol,
          exchange_sequence,
          retries);
      security.mbo.resubscribe = exchange_sequence;
    };
    log::info<5>(
        R"(DEBUG UPDATE exchange="{}", symbol="{}", orders=[{}], exchange_sequence={}, last_exchange_sequence={})"sv,
        security.exchange,
        security.symbol,
        fmt::join(orders, ", "sv),
        exchange_sequence,
        last_exchange_sequence);
    sequencer(
        orders,
        exchange_sequence,
        exchange_sequence,
        last_exchange_sequence,
        publish_update,
        publish_snapshot,
        request_snapshot);
  } catch (BadState &) {
    log::warn(
        R"(RESUBSCRIBE MBO exchange="{}", symbol="{}",  exchange_sequence={}, security_id={})"sv,
        security.exchange,
        security.symbol,
        exchange_sequence,
        security_id);
    // XXX HANS publish stale
    sequencer.clear();
    security.mbo.resubscribe = exchange_sequence;
  }
}

template <typename T>
void UDPIncremental::dispatch_trade_summary(Trace<T> const &event, mdp::Frame const &frame) {
  auto &trace_info = event.trace_info;
  using value_type = typename std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  value.sbeRewind();                                    // note!
  auto exchange_time_utc = std::chrono::nanoseconds{value.transactTime()};
  auto exchange_sequence = frame.sequence_number;
  auto clear_state = [&]() {
    security_ids_.clear();
    trade_summary_.clear();
    orders_.clear();
    total_number_of_orders_ = 0;
  };
  auto fragmented = exchange_time_utc == transact_time_;
  if (!fragmented) {
    transact_time_ = exchange_time_utc;
    clear_state();
  }
  auto insert_security_id = [&](auto security_id) {
    for (auto iter : security_ids_) {
      if (iter == security_id)
        return;
    }
    security_ids_.emplace_back(security_id);
  };
  value.noMDEntries().forEach([&]<typename U>(U &item) {
    auto security_id = item.securityID();
    auto aggressor_side = item.aggressorSide();
    auto price = mdp::get_double(const_cast<U &>(item).mDEntryPx());
    auto size = item.mDEntrySize();
    auto number_of_orders = item.numberOfOrders();
    auto trade_id = mdp::get_int(item.mDTradeEntryID(), item.mDTradeEntryIDNullValue());
    insert_security_id(security_id);
    auto side = mdp::map_side(aggressor_side);
    trade_summary_.emplace_back(security_id, side, price, size, number_of_orders, trade_id);
    total_number_of_orders_ += number_of_orders;
    shared_.get_security(security_id, [&](auto &security) { check_report_sequence(security, item, frame); });
  });
  value.noOrderIDEntries().forEach([&](auto &item) {
    auto order_id = item.orderID();
    auto last_qty = item.lastQty();
    orders_.emplace_back(order_id, last_qty);
  });
  if (std::size(orders_) < total_number_of_orders_) {
    log::warn(
        "Message is fragmented: sequence={}, len(orders)={}, expected={}"sv,
        exchange_sequence,
        std::size(orders_),
        total_number_of_orders_);
    return;  // note!
  }
  if (std::size(orders_) > total_number_of_orders_) {
    log::warn(
        "Unexpected: sequence={}, len(orders)={}, expected={}"sv,
        exchange_sequence,
        std::size(orders_),
        total_number_of_orders_);
  } else if (fragmented) {
    log::warn(
        "Message was fragmented and now fully assembled: sequence={}, len(orders)={}, expected={}"sv,
        exchange_sequence,
        std::size(orders_),
        total_number_of_orders_);
  }
  // mbo
  auto &mbo = shared_.get_mbo();
  auto dispatch_market_by_order_2 = [&](auto security_id, auto &security) {
    if (std::empty(mbo))
      return;
    dispatch_market_by_order(
        trace_info, security_id, security, exchange_sequence, exchange_time_utc, frame.sending_time, mbo.orders);
    mbo.clear();
  };
  for (auto security_id : security_ids_) {
    shared_.get_security(security_id, [&](auto &security) {
      size_t offset = 0;
      for (auto [security_id_2, aggressor_side, price, size, number_of_orders, trade_id] : trade_summary_) {
        auto side = aggressor_side;
        if (security_id == security_id_2) {
          size_t offset_2 = 0;
          if (aggressor_side != Side::UNDEFINED) {
            auto &[order_id, last_qty] = orders_[offset + offset_2];
            if (last_qty == size)
              side = utils::invert(side);
            ++offset_2;
          }
          for (; offset_2 < number_of_orders; ++offset_2) {
            auto &[order_id, last_qty] = orders_[offset + offset_2];
            auto result = MBOUpdate{
                .price = price * security.display_factor,
                .quantity = static_cast<double>(last_qty),
                .priority = {},
                .order_id = {},
                .side = side,
                .action = UpdateAction::TRADE,
                .reason = {},
            };
            fmt::format_to(std::back_inserter(result.order_id), "{}"sv, order_id);
            if (!last_qty) [[unlikely]]  // DEBUG
              log::warn("Unexpected: update={}"sv, result);
            mbo.orders.emplace_back(std::move(result));
          }
        }
        offset += number_of_orders;
      }
      dispatch_market_by_order_2(security_id, security);
    });
  }
  // trades
  auto &trades = shared_.get_trades();
  auto dispatch_trade_summary = [&](auto &security) {
    if (std::empty(trades)) [[unlikely]] {  // DEBUG
      log::warn("DEBUG EMPTY TRADES"sv);
      return;
    }
    auto trade_summary = TradeSummary{
        .stream_id = stream_id_,
        .exchange = security.exchange,
        .symbol = security.symbol,
        .trades = trades,
        .exchange_time_utc = exchange_time_utc,
        .exchange_sequence = frame.sequence_number,
        .sending_time_utc = frame.sending_time,
    };
    create_trace_and_dispatch(handler_, trace_info, trade_summary, true);
    trades.clear();
  };
  for (auto security_id : security_ids_) {
    shared_.get_security(security_id, [&](auto &security) {
      size_t offset = 0;
      for (auto [security_id_2, aggressor_side, price, size, number_of_orders, trade_id] : trade_summary_) {
        if (security_id == security_id_2) {
          size_t offset_2 = 0;
          auto trade_id_2 = fmt::format("{}"sv, trade_id);
          std::string taker_order_id;
          if (aggressor_side != Side::UNDEFINED) {
            auto &[order_id, last_qty] = orders_[offset + offset_2];
            if (last_qty == size) {
              taker_order_id = fmt::format("{}"sv, order_id);
              if (number_of_orders == 1) {
                auto trade = Trade{
                    .side = aggressor_side,
                    .price = price * security.display_factor,
                    .quantity = utils::safe_cast(last_qty),
                    .trade_id = trade_id_2,
                    .taker_order_id = {},
                    .maker_order_id = {},
                };
                fmt::format_to(std::back_inserter(trade.taker_order_id), "{}"sv, order_id);
                trades.emplace_back(std::move(trade));
              }
            } else {  // join
              auto trade = Trade{
                  .side = aggressor_side,
                  .price = price * security.display_factor,
                  .quantity = utils::safe_cast(last_qty),
                  .trade_id = trade_id_2,
                  .taker_order_id = {},
                  .maker_order_id = {},
              };
              fmt::format_to(std::back_inserter(trade.taker_order_id), "{}"sv, order_id);
              trades.emplace_back(std::move(trade));
            }
            ++offset_2;
          }
          for (; offset_2 < number_of_orders; ++offset_2) {
            auto &[order_id, last_qty] = orders_[offset + offset_2];
            auto trade = Trade{
                .side = aggressor_side,
                .price = price * security.display_factor,
                .quantity = utils::safe_cast(last_qty),
                .trade_id = trade_id_2,
                .taker_order_id = taker_order_id,
                .maker_order_id = {},
            };
            fmt::format_to(std::back_inserter(trade.maker_order_id), "{}"sv, order_id);
            trades.emplace_back(std::move(trade));
          }
        }
        offset += number_of_orders;
      }
      dispatch_trade_summary(security);
    });
  }
  // done
  clear_state();
}

template <typename T, typename Callback>
void UDPIncremental::dispatch_statistics(Trace<T> const &event, mdp::Frame const &frame, Callback callback) {
  auto &trace_info = event.trace_info;
  using value_type = typename std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  value.sbeRewind();                                    // note!
  auto exchange_time_utc = std::chrono::nanoseconds{value.transactTime()};
  auto &statistics = shared_.get_statistics();
  auto dispatch = [&]([[maybe_unused]] auto security_id, auto &security) {
    if (std::empty(statistics))
      return;
    auto statistics_update = StatisticsUpdate{
        .stream_id = stream_id_,
        .exchange = security.exchange,
        .symbol = security.symbol,
        .statistics = statistics,
        .update_type = UpdateType::INCREMENTAL,
        .exchange_time_utc = exchange_time_utc,
        .exchange_sequence = frame.sequence_number,
        .sending_time_utc = frame.sending_time,
    };
    log::info<3>("statistics_update={}"sv, statistics_update);
    create_trace_and_dispatch(handler_, trace_info, statistics_update, true);
    statistics.clear();
  };
  auto update = [&](auto &security, auto &item) {
    check_report_sequence(security, item, frame);
    callback(statistics, item, security);
  };
  SecurityIterator{shared_}(value.noMDEntries(), dispatch, update);
}

void UDPIncremental::check_report_sequence(tools::Security &security, auto const &value, mdp::Frame const &frame) {
  auto rpt_seq = value.rptSeq();
  if (!security.update_rpt_seq(rpt_seq))
    return;
  log::warn(R"(RESUBSCRIBE exchange="{}", symbol="{}", rpt_seq={})"sv, security.exchange, security.symbol, rpt_seq);
  security.mbp.sequencer.clear();
  security.mbp.resubscribe = frame.sequence_number;
  security.mbo.sequencer.clear();
  security.mbo.resubscribe = frame.sequence_number;
}

void UDPIncremental::publish_stream_status(TraceInfo const &trace_info, ConnectionStatus connection_status) {
  if (!utils::update(connection_status_, connection_status))
    return;
  auto stream_status = StreamStatus{
      .stream_id = stream_id_,
      .account = {},
      .supports = get_supports(),
      .transport = Transport::UDP,
      .protocol = Protocol::SBE,
      .encoding = {Encoding::SBE},
      .priority = Priority::PRIMARY,
      .connection_status = connection_status_,
      .interface = {},
      .authority = {},
      .path = {},
      .proxy = {},
  };
  log::info("stream_status={}"sv, stream_status);
  create_trace_and_dispatch(handler_, trace_info, stream_status);
}

void UDPIncremental::operator()(metrics::Writer &writer) {
  writer  //
      .write(counter_.disconnect, metrics::COUNTER)
      .write(counter_.sequence_reset, metrics::COUNTER)
      .write(profile_.parse, metrics::PROFILE)
      .write(profile_.admin_heartbeat, metrics::PROFILE)
      .write(profile_.channel_reset, metrics::PROFILE)
      .write(profile_.security_status, metrics::PROFILE)
      .write(profile_.md_instrument_definition_future, metrics::PROFILE)
      .write(profile_.md_instrument_definition_option, metrics::PROFILE)
      .write(profile_.md_instrument_definition_spread, metrics::PROFILE)
      .write(profile_.md_instrument_definition_fixed_income, metrics::PROFILE)
      .write(profile_.md_instrument_definition_repo, metrics::PROFILE)
      .write(profile_.md_instrument_definition_fx, metrics::PROFILE)
      .write(profile_.md_incremental_refresh_book, metrics::PROFILE)
      .write(profile_.md_incremental_refresh_book_long_qty, metrics::PROFILE)
      .write(profile_.md_incremental_refresh_order_book, metrics::PROFILE)
      .write(profile_.md_incremental_refresh_trade_summary, metrics::PROFILE)
      .write(profile_.md_incremental_refresh_trade_summary_long_qty, metrics::PROFILE)
      .write(profile_.md_incremental_refresh_daily_statistics, metrics::PROFILE)
      .write(profile_.md_incremental_refresh_session_statistics, metrics::PROFILE)
      .write(profile_.md_incremental_refresh_session_statistics_long_qty, metrics::PROFILE)
      .write(profile_.md_incremental_refresh_volume, metrics::PROFILE)
      .write(profile_.md_incremental_refresh_volume_long_qty, metrics::PROFILE)
      .write(profile_.md_incremental_refresh_limits_banding, metrics::PROFILE);
}

}  // namespace cme
}  // namespace roq
