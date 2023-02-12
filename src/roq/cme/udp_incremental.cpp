/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/cme/udp_incremental.hpp"

#include <fmt/ranges.h>

#include "roq/utils/safe_cast.hpp"
#include "roq/utils/update.hpp"

#include "roq/debug/hex/message.hpp"

#include "roq/core/metrics/factory.hpp"

#include "roq/io/network_address.hpp"

#include "roq/cme/utils.hpp"

#include "roq/cme/flags/common.hpp"
#include "roq/cme/flags/config.hpp"
#include "roq/cme/flags/multicast.hpp"

#include "roq/cme/sbe/utils.hpp"

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
  auto [multicast_address, port] = shared.get_multicast_config(channel_id, multicast::Type::INCREMENTAL, priority);
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
  if (flags::Common::test_mbo())
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
  Security *security_ = nullptr;
};

// following are used from several places

template <typename T>
void mbp_emplace_back(auto &result, T const &item, auto &security) {
  auto price = sbe::get_double(const_cast<T &>(item).mDEntryPx());
  auto quantity = sbe::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue());
  auto number_of_orders = sbe::get_int(item.numberOfOrders(), item.numberOfOrdersNullValue());
  auto update_action = [&]() -> UpdateAction {
    constexpr bool has_md_update_action = requires(T const &t) { t.mDUpdateAction(); };
    if constexpr (has_md_update_action) {
      return sbe::map(item.mDUpdateAction());
    }
    return {};
  }();
  if (update_action == UpdateAction::DELETE)
    quantity = {};  // note! exchange gives us the *old* value / we need this to be zero
  auto md_price_level = sbe::get_int(item.mDPriceLevel(), item.mDPriceLevelNullValue());
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
  auto side = sbe::map_side(item.aggressorSide());
  auto price = sbe::get_double(const_cast<T &>(item).mDEntryPx());
  auto quantity = sbe::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue());
  auto trade = Trade{
      .side = side,
      .price = price * security.display_factor,
      .quantity = utils::safe_cast(quantity),
      .trade_id = {},
      .taker_order_id = {},
      .maker_order_id = {},
  };
  auto trade_id = sbe::get_int(item.mDTradeEntryID(), item.mDTradeEntryIDNullValue());
  fmt::format_to(std::back_inserter(trade.trade_id), "{}"sv, trade_id);
  result.emplace_back(std::move(trade));
}

template <typename T>
void statistics_emplace_back_price(auto &result, auto type, T const &item, auto factor) {
  auto value = sbe::get_double(const_cast<T &>(item).mDEntryPx());
  auto statistics = Statistics{
      .type = type,
      .value = value * factor,
      .begin_time_utc = {},
      .end_time_utc = {},
  };
  result.emplace_back(std::move(statistics));
}

void statistics_emplace_back_size(auto &result, auto type, auto const &item) {
  auto value = sbe::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue());
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
  auto statistics_type = sbe::map(item.mDEntryType());
  if (statistics_type == StatisticsType::OPEN_INTEREST) {
    statistics_emplace_back_size(result, statistics_type, item);
  } else {
    statistics_emplace_back_price(result, statistics_type, item, security.display_factor);
  }
}

void emplace_back(
    cme_mdp::SnapshotFullRefresh52::NoMDEntries const &item,
    auto &security,
    auto &layer,
    auto &bids,
    auto &asks,
    auto &statistics) {
  using value_type = typename std::remove_cvref<decltype(item)>::type;
  switch (item.mDEntryType()) {
    using enum cme_mdp::MDEntryType::Value;
    case Bid:
      mbp_emplace_back(bids, item, security);
      break;
    case Offer:
      mbp_emplace_back(asks, item, security);
      break;
    case Trade:
      break;
    case OpenPrice:
      statistics_emplace_back_price(statistics, StatisticsType::OPEN_PRICE, item, security.display_factor);
      break;
    case SettlementPrice:
      statistics_emplace_back_price(statistics, StatisticsType::SETTLEMENT_PRICE, item, security.display_factor);
      break;
    case TradingSessionHighPrice:
      statistics_emplace_back_price(statistics, StatisticsType::HIGHEST_TRADED_PRICE, item, security.display_factor);
      break;
    case TradingSessionLowPrice:
      statistics_emplace_back_price(statistics, StatisticsType::LOWEST_TRADED_PRICE, item, security.display_factor);
      break;
    case VWAP:
      break;
    case ClearedVolume:
      break;
    case OpenInterest:
      statistics_emplace_back_size(statistics, StatisticsType::OPEN_INTEREST, item);
      break;
    case ImpliedBid:
      break;
    case ImpliedOffer:
      break;
    case BookReset:  // XXX ????????????????????????
      break;
    case SessionHighBid:
      break;
    case SessionLowOffer:
      break;
    case FixingPrice:
      // XXX need a new type?
      statistics_emplace_back_price(statistics, StatisticsType::CLOSE_PRICE, item, security.display_factor);
      break;
    case ElectronicVolume:
      break;
    case ThresholdLimitsandPriceBandVariation:
      break;
    case MarketBestOffer: {
      auto price = sbe::get_double(const_cast<value_type &>(item).mDEntryPx());
      layer.ask_price = price * security.display_factor;
      break;
    }
    case MarketBestBid: {
      auto price = sbe::get_double(const_cast<value_type &>(item).mDEntryPx());
      layer.bid_price = price * security.display_factor;
      break;
    }
    case NULL_VALUE:
      break;
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
      auto price = sbe::get_double(const_cast<value_type &>(item).mDEntryPx());
      layer.ask_price = price * security.display_factor;
      break;
    }
    case MarketBestBid: {
      auto price = sbe::get_double(const_cast<value_type &>(item).mDEntryPx());
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
    auto &bids,
    auto &asks) {
  auto create_update = [&]() {
    auto action = sbe::map(item.orderUpdateAction());
    auto remaining_quantity = sbe::get_int(item.mDDisplayQty(), item.mDDisplayQtyNullValue());
    auto priority = sbe::get_int(item.mDOrderPriority(), item.mDOrderPriorityNullValue());
    auto order_id = sbe::get_int(item.orderID(), item.orderIDNullValue());
    auto result = MBOUpdate{
        .price = price * security.display_factor,
        .quantity = static_cast<double>(remaining_quantity),
        .priority = priority,
        .order_id = {},
        .action = action,
        .reason = {},
    };
    fmt::format_to(std::back_inserter(result.order_id), "{}"sv, order_id);
    return result;
  };
  switch (side) {
    using enum Side;
    case UNDEFINED:
      break;
    case BUY: {
      auto update = create_update();
      bids.emplace_back(std::move(update));
      break;
    }
    case SELL: {
      auto update = create_update();
      asks.emplace_back(std::move(update));
      break;
    }
  }
}

void emplace_back(
    cme_mdp::MDIncrementalRefreshOrderBook47::NoMDEntries const &item, auto &security, auto &bids, auto &asks) {
  using value_type = typename std::remove_cvref<decltype(item)>::type;
  auto create_update = [&]() {
    auto price = sbe::get_double(const_cast<value_type &>(item).mDEntryPx());
    auto remaining_quantity = sbe::get_int(item.mDDisplayQty(), item.mDDisplayQtyNullValue());
    auto priority = sbe::get_int(item.mDOrderPriority(), item.mDOrderPriorityNullValue());
    auto order_id = sbe::get_int(item.orderID(), item.orderIDNullValue());
    auto action = sbe::map(item.mDUpdateAction());
    auto result = MBOUpdate{
        .price = price * security.display_factor,
        .quantity = static_cast<double>(remaining_quantity),
        .priority = priority,
        .order_id = {},
        .action = action,
        .reason = {},
    };
    fmt::format_to(std::back_inserter(result.order_id), "{}"sv, order_id);
    return result;
  };
  using value_type = typename std::remove_cvref<decltype(item)>::type;
  switch (item.mDEntryType()) {
    using enum cme_mdp::MDEntryTypeBook::Value;
    case Bid: {
      auto update = create_update();
      bids.emplace_back(std::move(update));
      break;
    }
    case Offer: {
      auto update = create_update();
      asks.emplace_back(std::move(update));
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
          .snapshot_full_refresh = create_metrics(name_, "snapshot_full_refresh"sv),
          .snapshot_full_refresh_long_qty = create_metrics(name_, "snapshot_full_refresh_long_qty"sv),
          .md_incremental_refresh_book = create_metrics(name_, "md_incremental_refresh_book"sv),
          .md_incremental_refresh_book_long_qty = create_metrics(name_, "md_incremental_refresh_book_long_qty"sv),
          .snapshot_full_refresh_order_book = create_metrics(name_, "snapshot_full_refresh_order_book"sv),
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
          .quote_request = create_metrics(name_, "quote_request"sv),
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
  using value_type = typename decltype(channel.buffer)::value_type;
  for (auto stop = false; !stop;) {
    if (channel.buffer.next([&](auto buffer) -> std::pair<size_t, value_type> {
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
          if (sbe::Frame::parse(message, [&](auto &frame) {
                log::info<5>("frame={}, last_sequence_number={}"sv, frame, channel.last_sequence.second);
                // check sequence number
                sequence_number = frame.sequence_number;
                auto [ready, last_sequence_number] = channel.last_sequence;
                if (ready) {
                  auto next_sequence_number = last_sequence_number + 1;
                  hold = sequence_number > next_sequence_number;
                  drop = sequence_number < next_sequence_number;
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
            parse(message);
            channel.last_sequence = {true, sequence_number};
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
        auto [ready, last_sequence_number] = channel.last_sequence;
        if (ready) {
          // check next sequence number
          auto sequence_number = last_sequence_number + 1;
          if (channel.buffer.get(sequence_number, [&](auto &message) {
                // parse this message
                parse(message);
                channel.last_sequence = {true, sequence_number};
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
      log::warn<1>("*** RESET (BUFFER FULL) ***"sv);
      channel.buffer.clear();
      channel.last_sequence = {};
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
    if (!sbe::Parser::dispatch(*this, message, trace_info)) {
      log::warn("{}"sv, debug::hex::Message{message});
      log::fatal("Failed to parse message"sv);
    }
    if (log_this_message_) {  // DEBUG
      log::info("{}"sv, debug::hex::Message{message});
    }
  };
  drain(*receiver_, channel_, stream_id_, parse, [&]() { on_sequence_reset(trace_info); });
}

void UDPIncremental::on_sequence_reset(TraceInfo const &trace_info) {
  ++counter_.sequence_reset;
  log::warn<1>("*** RESUBSCRIBE ALL SYMBOLS ***"sv);
  for (auto &[security_id, collector] : channel_.mbp_collector) {
    if (collector.ready()) {
      shared_.get_security(security_id, [&](auto &security) {
        auto market_by_price_update = MarketByPriceUpdate{
            .stream_id = stream_id_,
            .exchange = security.exchange,
            .symbol = security.symbol,
            .bids = {},
            .asks = {},
            .update_type = UpdateType::STALE,
            .exchange_time_utc = {},
            .exchange_sequence = {},
            .price_decimals = {},
            .quantity_decimals = {},
            .checksum = {},
        };
        create_trace_and_dispatch(handler_, trace_info, market_by_price_update, true);
      });
    }
    collector.clear();
    channel_.mbp_last_sequence.erase(security_id);
  }
}

void UDPIncremental::operator()(io::net::udp::Receiver::Error const &error) {
  log::fatal("Error: what={}"sv, error.what);
}

// sbe::Parser::Handler

void UDPIncremental::operator()(sbe::Frame const &frame) {
  channel_.last_sequence = {true, frame.sequence_number};
}

void UDPIncremental::operator()(Trace<cme_mdp::AdminHeartbeat12> const &event, sbe::Frame const &frame) {
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

void UDPIncremental::operator()(Trace<cme_mdp::ChannelReset4> const &event, sbe::Frame const &frame) {
  profile_.channel_reset([&]() {
    auto &[trace_info, value] = event;
    log::info<5>("channel_reset_4={}, frame={}"sv, value, frame);
  });
}

void UDPIncremental::operator()(Trace<cme_mdp::SecurityStatus30> const &event, sbe::Frame const &frame) {
  profile_.security_status([&]() {
    auto &trace_info = event.trace_info;
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("security_status_30={}, frame={}"sv, value, frame);
    value.sbeRewind();  // note!
    auto security_id = sbe::get_int(value.securityID(), value.securityIDNullValue());
    auto trading_status = sbe::map_security_trading_status(value.securityTradingStatus());
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
      auto security_group = sbe::get_string_view(value.securityGroup(), value.securityGroupLength());
      shared_.get_security_group(security_group, [&](auto security_id) { dispatch(security_id); });
    }
  });
}

void UDPIncremental::operator()(Trace<cme_mdp::MDInstrumentDefinitionFuture54> const &event, sbe::Frame const &frame) {
  profile_.md_instrument_definition_future([&]() {
    auto &trace_info = event.trace_info;
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("md_instrument_definition_future_54={}, frame={}"sv, value, frame);
    value.sbeRewind();  // note!
    auto security_id = value.securityID();
    shared_.get_security_incl_discard(security_id, [&](auto &security) {
      auto quote_currency = sbe::get_string_view(value.currency(), value.currencyLength());
      auto min_price_increment = sbe::get_double(value.minPriceIncrement());
      auto contract_multiplier = sbe::get_int(value.contractMultiplier(), value.contractMultiplierNullValue());
      auto multiplier = contract_multiplier == 0 ? NaN : utils::safe_cast<double>(contract_multiplier);
      auto min_trade_vol = utils::safe_cast(value.minTradeVol());
      auto max_trade_vol = utils::safe_cast(value.maxTradeVol());
      auto reference_data = ReferenceData{
          .stream_id = stream_id_,
          .exchange = security.exchange,
          .symbol = security.symbol,
          .description = {},
          .security_type = SecurityType::FUTURES,
          .base_currency = {},
          .quote_currency = quote_currency,
          .margin_currency = {},
          .commission_currency = {},
          .tick_size = min_price_increment * security.display_factor,
          .multiplier = multiplier,
          .min_trade_vol = min_trade_vol,
          .max_trade_vol = max_trade_vol,
          .trade_vol_step_size = NaN,
          .option_type = {},
          .strike_currency = {},
          .strike_price = NaN,
          .underlying = {},
          .time_zone = {},
          .issue_date = {},
          .settlement_date = {},
          .expiry_datetime = {},  // MaturityMonthYear ???
          .expiry_datetime_utc = {},
          .discard = security.discard,
      };
      create_trace_and_dispatch(handler_, trace_info, reference_data, true);
      if (security.discard)
        return;
      auto trading_status = sbe::map_security_trading_status(value.mDSecurityTradingStatus());
      auto market_status = MarketStatus{
          .stream_id = stream_id_,
          .exchange = security.exchange,
          .symbol = security.symbol,
          .trading_status = trading_status,
      };
      create_trace_and_dispatch(handler_, trace_info, market_status, true);
    });
  });
}

void UDPIncremental::operator()(Trace<cme_mdp::MDInstrumentDefinitionOption55> const &event, sbe::Frame const &frame) {
  profile_.md_instrument_definition_option([&]() {
    auto &trace_info = event.trace_info;
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("md_instrument_definition_option_55={}, frame={}"sv, value, frame);
    value.sbeRewind();  // note!
    auto security_id = value.securityID();
    shared_.get_security_incl_discard(security_id, [&](auto &security) {
      auto quote_currency = sbe::get_string_view(value.currency(), value.currencyLength());
      auto min_price_increment = sbe::get_double(value.minPriceIncrement());
      auto min_trade_vol = utils::safe_cast(value.minTradeVol());
      auto max_trade_vol = utils::safe_cast(value.maxTradeVol());
      auto strike_currency = sbe::get_string_view(value.strikeCurrency(), value.strikeCurrencyLength());
      auto reference_data = ReferenceData{
          .stream_id = stream_id_,
          .exchange = security.exchange,
          .symbol = security.symbol,
          .description = {},
          .security_type = SecurityType::OPTION,
          .base_currency = {},
          .quote_currency = quote_currency,
          .margin_currency = {},
          .commission_currency = {},
          .tick_size = min_price_increment * security.display_factor,
          .multiplier = NaN,
          .min_notional = NaN,
          .min_trade_vol = min_trade_vol,
          .max_trade_vol = max_trade_vol,
          .trade_vol_step_size = NaN,
          .option_type = {},
          .strike_currency = strike_currency,
          .strike_price = NaN,
          .underlying = {},
          .time_zone = {},
          .issue_date = {},
          .settlement_date = {},
          .expiry_datetime = {},  // MaturityMonthYear ???
          .expiry_datetime_utc = {},
          .discard = security.discard,
      };
      create_trace_and_dispatch(handler_, trace_info, reference_data, true);
      if (security.discard)
        return;
      auto trading_status = sbe::map_security_trading_status(value.mDSecurityTradingStatus());
      auto market_status = MarketStatus{
          .stream_id = stream_id_,
          .exchange = security.exchange,
          .symbol = security.symbol,
          .trading_status = trading_status,
      };
      create_trace_and_dispatch(handler_, trace_info, market_status, true);
    });
  });
}

void UDPIncremental::operator()(Trace<cme_mdp::MDInstrumentDefinitionSpread56> const &event, sbe::Frame const &frame) {
  profile_.md_instrument_definition_spread([&]() {
    auto &trace_info = event.trace_info;
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("md_instrument_definition_spread_56={}, frame={}"sv, value, frame);
    value.sbeRewind();  // note!
    auto security_id = value.securityID();
    shared_.get_security_incl_discard(security_id, [&](auto &security) {
      auto quote_currency = sbe::get_string_view(value.currency(), value.currencyLength());
      auto tick_size = sbe::get_double(value.minPriceIncrement());
      auto min_trade_vol = utils::safe_cast(value.minTradeVol());
      auto max_trade_vol = utils::safe_cast(value.maxTradeVol());
      auto reference_data = ReferenceData{
          .stream_id = stream_id_,
          .exchange = security.exchange,
          .symbol = security.symbol,
          .description = {},
          .security_type = SecurityType::FUTURES,
          .base_currency = {},
          .quote_currency = quote_currency,
          .margin_currency = {},
          .commission_currency = {},
          .tick_size = tick_size,
          .multiplier = NaN,
          .min_trade_vol = min_trade_vol,
          .max_trade_vol = max_trade_vol,
          .trade_vol_step_size = NaN,
          .option_type = {},
          .strike_currency = {},
          .strike_price = NaN,
          .underlying = {},
          .time_zone = {},
          .issue_date = {},
          .settlement_date = {},
          .expiry_datetime = {},  // MaturityMonthYear ???
          .expiry_datetime_utc = {},
          .discard = security.discard,
      };
      create_trace_and_dispatch(handler_, trace_info, reference_data, true);
      if (security.discard)
        return;
      auto trading_status = sbe::map_security_trading_status(value.mDSecurityTradingStatus());
      auto market_status = MarketStatus{
          .stream_id = stream_id_,
          .exchange = security.exchange,
          .symbol = security.symbol,
          .trading_status = trading_status,
      };
      create_trace_and_dispatch(handler_, trace_info, market_status, true);
    });
  });
}

void UDPIncremental::operator()(
    Trace<cme_mdp::MDInstrumentDefinitionFixedIncome57> const &event, sbe::Frame const &frame) {
  profile_.md_instrument_definition_fixed_income([&]() {
    auto &trace_info = event.trace_info;
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("md_instrument_definition_fixed_income_57={}, frame={}"sv, value, frame);
    value.sbeRewind();  // note!
    auto security_id = value.securityID();
    shared_.get_security_incl_discard(security_id, [&](auto &security) {
      auto quote_currency = sbe::get_string_view(value.currency(), value.currencyLength());
      auto tick_size = sbe::get_double(value.minPriceIncrement());
      auto min_trade_vol = utils::safe_cast(value.minTradeVol());
      auto max_trade_vol = utils::safe_cast(value.maxTradeVol());
      auto reference_data = ReferenceData{
          .stream_id = stream_id_,
          .exchange = security.exchange,
          .symbol = security.symbol,
          .description = {},
          .security_type = SecurityType::FUTURES,
          .base_currency = {},
          .quote_currency = quote_currency,
          .margin_currency = {},
          .commission_currency = {},
          .tick_size = tick_size,
          .multiplier = NaN,
          .min_trade_vol = min_trade_vol,
          .max_trade_vol = max_trade_vol,
          .trade_vol_step_size = NaN,
          .option_type = {},
          .strike_currency = {},
          .strike_price = NaN,
          .underlying = {},
          .time_zone = {},
          .issue_date = {},
          .settlement_date = {},
          .expiry_datetime = {},  // MaturityMonthYear ???
          .expiry_datetime_utc = {},
          .discard = security.discard,
      };
      create_trace_and_dispatch(handler_, trace_info, reference_data, true);
      if (security.discard)
        return;
      auto trading_status = sbe::map_security_trading_status(value.mDSecurityTradingStatus());
      auto market_status = MarketStatus{
          .stream_id = stream_id_,
          .exchange = security.exchange,
          .symbol = security.symbol,
          .trading_status = trading_status,
      };
      create_trace_and_dispatch(handler_, trace_info, market_status, true);
    });
  });
}

void UDPIncremental::operator()(Trace<cme_mdp::MDInstrumentDefinitionRepo58> const &event, sbe::Frame const &frame) {
  profile_.md_instrument_definition_repo([&]() {
    auto &trace_info = event.trace_info;
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("md_instrument_definition_repo_58={}, frame={}"sv, value, frame);
    value.sbeRewind();  // note!
    auto security_id = value.securityID();
    shared_.get_security_incl_discard(security_id, [&](auto &security) {
      auto quote_currency = sbe::get_string_view(value.currency(), value.currencyLength());
      auto tick_size = sbe::get_double(value.minPriceIncrement());
      auto min_trade_vol = utils::safe_cast(value.minTradeVol());
      auto max_trade_vol = utils::safe_cast(value.maxTradeVol());
      auto reference_data = ReferenceData{
          .stream_id = stream_id_,
          .exchange = security.exchange,
          .symbol = security.symbol,
          .description = {},
          .security_type = SecurityType::FUTURES,
          .base_currency = {},
          .quote_currency = quote_currency,
          .margin_currency = {},
          .commission_currency = {},
          .tick_size = tick_size,
          .multiplier = NaN,
          .min_trade_vol = min_trade_vol,
          .max_trade_vol = max_trade_vol,
          .trade_vol_step_size = NaN,
          .option_type = {},
          .strike_currency = {},
          .strike_price = NaN,
          .underlying = {},
          .time_zone = {},
          .issue_date = {},
          .settlement_date = {},
          .expiry_datetime = {},  // MaturityMonthYear ???
          .expiry_datetime_utc = {},
          .discard = security.discard,
      };
      create_trace_and_dispatch(handler_, trace_info, reference_data, true);
      if (security.discard)
        return;
      auto trading_status = sbe::map_security_trading_status(value.mDSecurityTradingStatus());
      auto market_status = MarketStatus{
          .stream_id = stream_id_,
          .exchange = security.exchange,
          .symbol = security.symbol,
          .trading_status = trading_status,
      };
      create_trace_and_dispatch(handler_, trace_info, market_status, true);
    });
  });
}

void UDPIncremental::operator()(Trace<cme_mdp::MDInstrumentDefinitionFX63> const &event, sbe::Frame const &frame) {
  profile_.md_instrument_definition_fx([&]() {
    auto &trace_info = event.trace_info;
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("md_instrument_definition_fx_63={}, frame={}"sv, value, frame);
    value.sbeRewind();  // note!
    auto security_id = value.securityID();
    shared_.get_security_incl_discard(security_id, [&](auto &security) {
      auto quote_currency = sbe::get_string_view(value.currency(), value.currencyLength());
      auto tick_size = sbe::get_double(value.minPriceIncrement());
      auto min_trade_vol = utils::safe_cast(value.minTradeVol());
      auto max_trade_vol = utils::safe_cast(value.maxTradeVol());
      auto reference_data = ReferenceData{
          .stream_id = stream_id_,
          .exchange = security.exchange,
          .symbol = security.symbol,
          .description = {},
          .security_type = SecurityType::FUTURES,
          .base_currency = {},
          .quote_currency = quote_currency,
          .margin_currency = {},
          .commission_currency = {},
          .tick_size = tick_size,
          .multiplier = NaN,
          .min_trade_vol = min_trade_vol,
          .max_trade_vol = max_trade_vol,
          .trade_vol_step_size = NaN,
          .option_type = {},
          .strike_currency = {},
          .strike_price = NaN,
          .underlying = {},
          .time_zone = {},
          .issue_date = {},
          .settlement_date = {},
          .expiry_datetime = {},  // MaturityMonthYear ???
          .expiry_datetime_utc = {},
          .discard = security.discard,
      };
      create_trace_and_dispatch(handler_, trace_info, reference_data, true);
      if (security.discard)
        return;
      auto trading_status = sbe::map_security_trading_status(value.mDSecurityTradingStatus());
      auto market_status = MarketStatus{
          .stream_id = stream_id_,
          .exchange = security.exchange,
          .symbol = security.symbol,
          .trading_status = trading_status,
      };
      create_trace_and_dispatch(handler_, trace_info, market_status, true);
    });
  });
}

// XXX HANS why are we processing this here ???
void UDPIncremental::operator()(Trace<cme_mdp::SnapshotFullRefresh52> const &event, sbe::Frame const &frame) {
  profile_.snapshot_full_refresh([&]() {
    auto &trace_info = event.trace_info;
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("snapshot_full_refresh_52={}, frame={}"sv, value, frame);
    value.sbeRewind();  // note!
    auto security_id = value.securityID();
    shared_.get_security(security_id, [&](auto &security) {
      auto exchange_time_utc = std::chrono::nanoseconds{value.transactTime()};
      auto exchange_sequence = value.lastMsgSeqNumProcessed();
      Layer layer;
      auto &mbp = shared_.get_mbp();
      auto &statistics = shared_.get_statistics();
      value.noMDEntries().forEach(
          [&](auto const &item) { emplace_back(item, security, layer, mbp.bids, mbp.asks, statistics); });
      if (!(std::isnan(layer.bid_price) && std::isnan(layer.ask_price))) {
        auto top_of_book = TopOfBook{
            .stream_id = stream_id_,
            .exchange = security.exchange,
            .symbol = security.symbol,
            .layer = layer,
            .exchange_time_utc = exchange_time_utc,
            .exchange_sequence = exchange_sequence,
        };
        create_trace_and_dispatch(handler_, trace_info, std::as_const(top_of_book), true);
      }
      if (!std::empty(mbp)) {
        dispatch_market_by_price(
            trace_info, security_id, security, exchange_sequence, exchange_time_utc, mbp.bids, mbp.asks);
      }
      if (!std::empty(statistics)) {
        auto statistics_update = StatisticsUpdate{
            .stream_id = stream_id_,
            .exchange = security.exchange,
            .symbol = security.symbol,
            .statistics = statistics,
            .update_type = UpdateType::SNAPSHOT,
            .exchange_time_utc = exchange_time_utc,
        };
        create_trace_and_dispatch(handler_, trace_info, statistics_update, true);
      }
    });
  });
}

void UDPIncremental::operator()(Trace<cme_mdp::SnapshotFullRefreshLongQty69> const &event, sbe::Frame const &frame) {
  profile_.snapshot_full_refresh_long_qty([&]() {
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("snapshot_full_refresh_long_qty_69={}, frame={}"sv, value, frame);
  });
}

void UDPIncremental::operator()(Trace<cme_mdp::QuoteRequest39> const &event, sbe::Frame const &frame) {
  profile_.quote_request([&]() {
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("quote_request_39={}, frame={}"sv, value, frame);
  });
}

void UDPIncremental::operator()(Trace<cme_mdp::MDIncrementalRefreshBook46> const &event, sbe::Frame const &frame) {
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
              trace_info, security_id, security, exchange_sequence, exchange_time_utc, mbp.bids, mbp.asks);
          mbp.clear();
        }
        if (!std::empty(mbo)) {
          dispatch_market_by_order(
              trace_info, security_id, security, exchange_sequence, exchange_time_utc, mbo.bids, mbo.asks);
          mbo.clear();
        }
      };
      // HANS here we need to append every item regardless of security ==> can't use SecurityIterator
      auto security_id = int32_t{};
      Security *security = nullptr;
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
        if (security) {
          auto rpt_seq = item.rptSeq();
          (*security).update_rpt_seq(rpt_seq);
        }
        // ... need these for MBO referencing
        using value_type = typename std::remove_cvref<decltype(item)>::type;
        auto &value = const_cast<value_type &>(item);  // note! not const-safe
        auto price = sbe::get_double(value.mDEntryPx());
        auto side = sbe::map(item.mDEntryType());
        auto action = sbe::map(item.mDUpdateAction());
        md_entries_.emplace_back(security_id, side, price, action);
        if (security) {
          emplace_back(item, *security, layer, mbp.bids, mbp.asks);
          if (action == UpdateAction::DELETE) {
            auto update = MBOUpdate{
                .price = price * (*security).display_factor,
                .quantity = {},
                .priority = {},
                .order_id = {},
                .action = action,
                .reason = {},
            };
            switch (side) {
              using enum Side;
              case UNDEFINED:
                break;
              case BUY:
                mbo.bids.emplace_back(std::move(update));
                break;
              case SELL:
                mbo.asks.emplace_back(std::move(update));
                break;
            }
          }
        }
      });
      if (security)
        dispatch(security_id, *security, true);
    }
    {  // MBO
      auto security_id = int32_t{};
      Security *security = nullptr;
      auto &mbo = shared_.get_mbo();
      auto dispatch = [&](auto security_id, auto &security) {
        if (!std::empty(mbo)) {
          dispatch_market_by_order(
              trace_info, security_id, security, exchange_sequence, exchange_time_utc, mbo.bids, mbo.asks);
          mbo.clear();
        }
      };
      value.noOrderIDEntries().forEach([&](auto const &item) {
        auto reference_id = sbe::get_int(item.referenceID(), item.referenceIDNullValue());
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
            emplace_back(item, *security, side, price, mbo.bids, mbo.asks);
        }
      });
      if (security)
        dispatch(security_id, *security);
    }
  });
}

void UDPIncremental::operator()(
    Trace<cme_mdp::MDIncrementalRefreshBookLongQty64> const &event, sbe::Frame const &frame) {
  profile_.md_incremental_refresh_book_long_qty([&]() {
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("md_incremental_refresh_book_long_qty_64={}, frame={}"sv, value, frame);
    auto dispatch = []([[maybe_unused]] auto security_id, [[maybe_unused]] auto &security) {};
    auto update = [&](auto &security, auto &item) {
      auto rpt_seq = item.rptSeq();
      security.update_rpt_seq(rpt_seq);
    };
    SecurityIterator{shared_}(value.noMDEntries(), dispatch, update);
  });
}

void UDPIncremental::operator()(Trace<cme_mdp::SnapshotFullRefreshOrderBook53> const &event, sbe::Frame const &frame) {
  profile_.snapshot_full_refresh_order_book([&]() {
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("snapshot_full_refresh_order_book_53={}, frame={}"sv, value, frame);
  });
}

void UDPIncremental::operator()(Trace<cme_mdp::MDIncrementalRefreshOrderBook47> const &event, sbe::Frame const &frame) {
  profile_.md_incremental_refresh_order_book([&]() {
    auto &trace_info = event.trace_info;
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("md_incremental_refresh_order_book_47={}, frame={}"sv, value, frame);
    if (!flags::Common::test_mbo())
      return;
    value.sbeRewind();  // note!
    auto exchange_sequence = frame.sequence_number;
    auto exchange_time_utc = std::chrono::nanoseconds{value.transactTime()};
    auto &mbo = shared_.get_mbo();
    auto dispatch = [&](auto security_id, auto &security) {
      if (std::empty(mbo))
        return;
      dispatch_market_by_order(
          trace_info, security_id, security, exchange_sequence, exchange_time_utc, mbo.bids, mbo.asks);
      mbo.clear();
    };
    auto update = [&](auto &security, auto &item) { emplace_back(item, security, mbo.bids, mbo.asks); };
    SecurityIterator{shared_}(value.noMDEntries(), dispatch, update);
  });
}

void UDPIncremental::operator()(
    Trace<cme_mdp::MDIncrementalRefreshTradeSummary48> const &event, sbe::Frame const &frame) {
  profile_.md_incremental_refresh_trade_summary([&]() {
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("md_incremental_refresh_trade_summary_48={}, frame={}"sv, value, frame);
    dispatch_trade_summary(event);
  });
}

void UDPIncremental::operator()(
    Trace<cme_mdp::MDIncrementalRefreshTradeSummaryLongQty65> const &event, sbe::Frame const &frame) {
  profile_.md_incremental_refresh_trade_summary_long_qty([&]() {
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("md_incremental_refresh_trade_summary_long_qty_65={}, frame={}"sv, value, frame);
    dispatch_trade_summary(event);
  });
}

void UDPIncremental::operator()(
    Trace<cme_mdp::MDIncrementalRefreshDailyStatistics49> const &event, sbe::Frame const &frame) {
  profile_.md_incremental_refresh_daily_statistics([&]() {
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("md_incremental_refresh_daily_statistics_49={}, frame={}"sv, value, frame);
    dispatch_statistics(event, [](auto &statistics, auto &item, auto &security) {
      statistics_emplace_back(statistics, item, security);
    });
  });
}

void UDPIncremental::operator()(
    Trace<cme_mdp::MDIncrementalRefreshSessionStatistics51> const &event, sbe::Frame const &frame) {
  profile_.md_incremental_refresh_session_statistics([&]() {
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("md_incremental_refresh_session_statistics_51={}, frame={}"sv, value, frame);
    dispatch_statistics(event, [](auto &statistics, auto &item, auto &security) {
      statistics_emplace_back(statistics, item, security);
    });
  });
}

void UDPIncremental::operator()(
    Trace<cme_mdp::MDIncrementalRefreshSessionStatisticsLongQty67> const &event, sbe::Frame const &frame) {
  profile_.md_incremental_refresh_session_statistics_long_qty([&]() {
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("md_incremental_refresh_session_statistics_long_qty_67={}, frame={}"sv, value, frame);
    dispatch_statistics(event, [](auto &statistics, auto &item, auto &security) {
      statistics_emplace_back(statistics, item, security);
    });
  });
}

void UDPIncremental::operator()(Trace<cme_mdp::MDIncrementalRefreshVolume37> const &event, sbe::Frame const &frame) {
  profile_.md_incremental_refresh_volume([&]() {
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("md_incremental_refresh_volume_37={}, frame={}"sv, value, frame);
    dispatch_statistics(event, [](auto &statistics, auto &item, [[maybe_unused]] auto &security) {
      statistics_emplace_back_size(statistics, StatisticsType::TRADE_VOLUME, item);
    });
  });
}

void UDPIncremental::operator()(
    Trace<cme_mdp::MDIncrementalRefreshVolumeLongQty66> const &event, sbe::Frame const &frame) {
  profile_.md_incremental_refresh_volume_long_qty([&]() {
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("md_incremental_refresh_volume_long_qty_66={}, frame={}"sv, value, frame);
    auto dispatch = []([[maybe_unused]] auto security_id, [[maybe_unused]] auto &security) {};
    auto update = [&](auto &security, auto &item) {
      auto rpt_seq = item.rptSeq();
      security.update_rpt_seq(rpt_seq);
    };
    SecurityIterator{shared_}(value.noMDEntries(), dispatch, update);
  });
}

void UDPIncremental::operator()(
    Trace<cme_mdp::MDIncrementalRefreshLimitsBanding50> const &event, sbe::Frame const &frame) {
  profile_.md_incremental_refresh_limits_banding([&]() {
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("md_incremental_refresh_limits_banding_50={}, frame={}"sv, value, frame);
    auto dispatch = []([[maybe_unused]] auto security_id, [[maybe_unused]] auto &security) {};
    auto update = [&](auto &security, auto &item) {
      auto rpt_seq = item.rptSeq();
      security.update_rpt_seq(rpt_seq);
    };
    SecurityIterator{shared_}(value.noMDEntries(), dispatch, update);
  });
}

void UDPIncremental::dispatch_market_by_price(
    auto &trace_info,
    auto security_id,
    auto &security,
    auto exchange_sequence,
    auto exchange_time_utc,
    auto &bids,
    auto &asks) {
  channel_.mbp_last_sequence[security_id] = exchange_sequence;
  auto &collector = channel_.mbp_collector[security_id];
  try {
    auto last_exchange_sequence = collector.last_sequence();  // note! the protocol doesn't tell us
    auto create_update = [&](auto &bids, auto &asks, auto update_type) -> MarketByPriceUpdate {
      if (flags::Common::test_inversion()) [[unlikely]] {  // DEBUG
        if (!std::empty(bids) && !std::empty(asks)) {
          auto &bid = bids[0];
          auto &ask = asks[0];
          if (bid.update_action != UpdateAction::DELETE && ask.update_action != UpdateAction::DELETE &&
              utils::compare(ask.price, bid.price) <= 0) {
            log::info("*** INVERSION *** {} {}"sv, bid.price, ask.price);
            log_this_message_ = true;  // DEBUG
          }
        }
        if (!std::empty(asks) && utils::compare(asks[0].price, 100.0) < 0) {
          log::info("*** INVERSION ***  {}"sv, asks[0].price);
          log_this_message_ = true;  // DEBUG
        }
      }
      return {
          .stream_id = stream_id_,
          .exchange = security.exchange,
          .symbol = security.symbol,
          .bids = bids,
          .asks = asks,
          .update_type = update_type,
          .exchange_time_utc = exchange_time_utc,
          .exchange_sequence = exchange_sequence,
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
      auto market_by_price_update = create_update(bids, asks, UpdateType::SNAPSHOT);
      create_trace_and_dispatch(handler_, trace_info, market_by_price_update, true);
    };
    auto request_snapshot = [&]([[maybe_unused]] auto retries) {
      channel_.mbp_resubscribe.emplace(security_id, exchange_sequence);
      channel_.mbp_last_sequence.erase(security_id);
    };
    collector(
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
        R"(RESUBSCRIBE exchange="{}", symbol="{}", security_id={})"sv, security.exchange, security.symbol, security_id);
    // XXX HANS publish stale
    collector.clear();
    channel_.mbp_resubscribe.emplace(security_id, exchange_sequence);
    channel_.mbp_last_sequence.erase(security_id);
  }
}

void UDPIncremental::dispatch_market_by_order(
    auto &trace_info,
    [[maybe_unused]] auto security_id,
    auto &security,
    auto exchange_sequence,
    auto exchange_time_utc,
    auto &bids,
    auto &asks) {
  auto market_by_order_update = MarketByOrderUpdate{
      .stream_id = stream_id_,
      .exchange = security.exchange,
      .symbol = security.symbol,
      .bids = bids,
      .asks = asks,
      .update_type = UpdateType::INCREMENTAL,
      .exchange_time_utc = exchange_time_utc,
      .exchange_sequence = exchange_sequence,
      .price_decimals = {},
      .quantity_decimals = {},
      .checksum = {},
  };
  create_trace_and_dispatch(handler_, trace_info, market_by_order_update, true);
}

template <typename T>
void UDPIncremental::dispatch_trade_summary(Trace<T> const &event) {
  auto &trace_info = event.trace_info;
  using value_type = typename std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  value.sbeRewind();                                    // note!
  auto exchange_time_utc = std::chrono::nanoseconds{value.transactTime()};
  auto &trades = shared_.get_trades();
  auto dispatch = [&]([[maybe_unused]] auto security_id, auto &security) {
    if (std::empty(trades))
      return;
    auto trade_summary = TradeSummary{
        .stream_id = stream_id_,
        .exchange = security.exchange,
        .symbol = security.symbol,
        .trades = trades,
        .exchange_time_utc = exchange_time_utc,
        .exchange_sequence = {},  // note! NoMDEntries.RptSeq is the MD seqno
    };
    create_trace_and_dispatch(handler_, trace_info, trade_summary, true);
    trades.clear();
  };
  auto update = [&](auto &security, auto &item) {
    auto rpt_seq = item.rptSeq();
    security.update_rpt_seq(rpt_seq);
    trades_emplace_back(trades, item, security);
  };
  SecurityIterator{shared_}(value.noMDEntries(), dispatch, update);
}

template <typename T, typename Callback>
void UDPIncremental::dispatch_statistics(Trace<T> const &event, Callback callback) {
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
    };
    log::info<3>("statistics_update={}"sv, statistics_update);
    create_trace_and_dispatch(handler_, trace_info, statistics_update, true);
    statistics.clear();
  };
  auto update = [&](auto &security, auto &item) {
    auto rpt_seq = item.rptSeq();
    security.update_rpt_seq(rpt_seq);
    callback(statistics, item, security);
  };
  SecurityIterator{shared_}(value.noMDEntries(), dispatch, update);
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
      .write(profile_.snapshot_full_refresh, metrics::PROFILE)
      .write(profile_.snapshot_full_refresh_long_qty, metrics::PROFILE)
      .write(profile_.md_incremental_refresh_book, metrics::PROFILE)
      .write(profile_.md_incremental_refresh_book_long_qty, metrics::PROFILE)
      .write(profile_.snapshot_full_refresh_order_book, metrics::PROFILE)
      .write(profile_.md_incremental_refresh_order_book, metrics::PROFILE)
      .write(profile_.md_incremental_refresh_trade_summary, metrics::PROFILE)
      .write(profile_.md_incremental_refresh_trade_summary_long_qty, metrics::PROFILE)
      .write(profile_.md_incremental_refresh_daily_statistics, metrics::PROFILE)
      .write(profile_.md_incremental_refresh_session_statistics, metrics::PROFILE)
      .write(profile_.md_incremental_refresh_session_statistics_long_qty, metrics::PROFILE)
      .write(profile_.md_incremental_refresh_volume, metrics::PROFILE)
      .write(profile_.md_incremental_refresh_volume_long_qty, metrics::PROFILE)
      .write(profile_.md_incremental_refresh_limits_banding, metrics::PROFILE)
      .write(profile_.quote_request, metrics::PROFILE);
}

}  // namespace cme
}  // namespace roq
