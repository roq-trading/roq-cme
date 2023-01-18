/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/cme/udp_incremental.hpp"

#include <fmt/ranges.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "roq/utils/safe_cast.hpp"
#include "roq/utils/update.hpp"

#include "roq/core/back_emplacer.hpp"

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

auto const SUPPORTS = Mask{
    SupportType::TOP_OF_BOOK,
    SupportType::MARKET_BY_PRICE,
    SupportType::STATISTICS,
};
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
  std::string local_interface{flags::Multicast::multicast_local_interface()};
  struct in_addr local = {};
  local.s_addr = inet_addr(local_interface.c_str());
  log::info(R"(Add membership "{}")"sv, multicast_address);
  struct in_addr multicast = {};
  multicast.s_addr = inet_addr(multicast_address.c_str());
  (*receiver).add_membership(io::NetworkAddress{0, multicast}, io::NetworkAddress{0, local});
  return receiver;
}

struct create_metrics final : public core::metrics::Factory {
  explicit create_metrics(auto const &group, auto const &function)
      : core::metrics::Factory(server::Flags::name(), group, function) {}
};

// following are used from several places

template <typename Callback>
bool get_security(auto &shared, auto security_id, Callback callback) {
  auto iter = shared.securities.find(security_id);
  if (iter == std::end(shared.securities))
    return false;
  auto &security = (*iter).second;
  if (security.discard)
    return false;
  callback(security);
  return true;
}

template <typename T, typename U, typename std::enable_if<std::is_same<T, MBPUpdate>::value, int>::type = 0>
void emplace(T &result, U const &item, auto &security) {
  auto price = sbe::get_double(const_cast<U &>(item).mDEntryPx());
  auto quantity = sbe::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue());
  auto number_of_orders = sbe::get_int(item.numberOfOrders(), item.numberOfOrdersNullValue());
  auto update_action = [&]() {
    constexpr bool has_md_update_action = requires(U const &t) { t.mDUpdateAction(); };
    if constexpr (has_md_update_action) {
      return sbe::map(item.mDUpdateAction());
    }
    return UpdateAction{};
  }();
  if (update_action == UpdateAction::DELETE)
    quantity = {};  // note! exchange gives us the *old* value / we need this to be zero
  auto md_price_level = sbe::get_int(item.mDPriceLevel(), item.mDPriceLevelNullValue());
  uint32_t price_level = md_price_level > 0 ? (md_price_level - 1) : 0;
  new (&result) T{
      .price = price * security.display_factor,
      .quantity = utils::safe_cast(quantity),
      .implied_quantity = NaN,
      .number_of_orders = utils::safe_cast(number_of_orders),
      .update_action = update_action,
      .price_level = price_level,
  };
}

template <typename T, typename U, typename std::enable_if<std::is_same<T, Trade>::value, int>::type = 0>
void emplace(T &result, U const &item, auto &security) {
  auto price = sbe::get_double(const_cast<U &>(item).mDEntryPx());
  auto quantity = sbe::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue());
  new (&result) T{
      .side = sbe::map_side(item.aggressorSide()),
      .price = price * security.display_factor,
      .quantity = utils::safe_cast(quantity),
      .trade_id = {},
      .taker_order_id = {},
      .maker_order_id = {},
  };
  auto trade_id = sbe::get_int(item.mDTradeEntryID(), item.mDTradeEntryIDNullValue());
  fmt::format_to(std::back_inserter(result.trade_id), "{}"sv, trade_id);
}

template <typename T>
void trade_summary_emplace_back(auto &result, T const &item, auto &security) {
  result.emplace_back([&item, &security](auto &result) { emplace(result, item, security); });
}

template <typename T, typename U, typename std::enable_if<std::is_same<T, Statistics>::value, int>::type = 0>
void emplace_price(T &result, auto type, U const &item, auto factor) {
  auto value = sbe::get_double(const_cast<U &>(item).mDEntryPx());
  new (&result) T{
      .type = type,
      .value = value * factor,
      .begin_time_utc = {},
      .end_time_utc = {},
  };
}

template <typename T, typename U, typename std::enable_if<std::is_same<T, Statistics>::value, int>::type = 0>
void emplace_size(T &result, auto type, U const &item) {
  auto value = sbe::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue());
  new (&result) T{
      .type = type,
      .value = utils::safe_cast(value),
      .begin_time_utc = {},
      .end_time_utc = {},
  };
}

template <typename T>
void statistics_emplace_back(auto &result, T const &item, auto &security) {
  auto statistics_type = sbe::map(item.mDEntryType());
  if (statistics_type == StatisticsType::OPEN_INTEREST) {
    result.emplace_back([&](auto &result) { emplace_size(result, statistics_type, item); });
  } else {
    result.emplace_back([&](auto &result) { emplace_price(result, statistics_type, item, security.display_factor); });
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
      bids.emplace_back([&item, &security](auto &result) { emplace(result, item, security); });
      break;
    case Offer:
      asks.emplace_back([&item, &security](auto &result) { emplace(result, item, security); });
      break;
    case Trade:
      break;
    case OpenPrice:
      statistics.emplace_back([&item, &security](auto &result) {
        emplace_price(result, StatisticsType::OPEN_PRICE, item, security.display_factor);
      });
      break;
    case SettlementPrice:
      statistics.emplace_back([&item, &security](auto &result) {
        emplace_price(result, StatisticsType::SETTLEMENT_PRICE, item, security.display_factor);
      });
      break;
    case TradingSessionHighPrice:
      statistics.emplace_back([&item, &security](auto &result) {
        emplace_price(result, StatisticsType::HIGHEST_TRADED_PRICE, item, security.display_factor);
      });
      break;
    case TradingSessionLowPrice:
      statistics.emplace_back([&item, &security](auto &result) {
        emplace_price(result, StatisticsType::LOWEST_TRADED_PRICE, item, security.display_factor);
      });
      break;
    case VWAP:
      break;
    case ClearedVolume:
      break;
    case OpenInterest:
      statistics.emplace_back([&item](auto &result) { emplace_size(result, StatisticsType::OPEN_INTEREST, item); });
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
      statistics.emplace_back([&item, &security](auto &result) {
        emplace_price(result, StatisticsType::CLOSE_PRICE, item, security.display_factor);
      });
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
      bids.emplace_back([&item, &security](auto &result) { emplace(result, item, security); });
      break;
    case Offer:
      asks.emplace_back([&item, &security](auto &result) { emplace(result, item, security); });
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
void drain(auto &receiver, auto &channel, auto parse, auto reset) {
  using value_type = typename decltype(channel.buffer)::value_type;
  for (auto stop = false; !stop;) {
    if (channel.buffer.next([&](auto buffer) -> std::pair<size_t, value_type> {
          // read into buffer
          auto bytes = receiver.recv(buffer);
          log::info<5>("Received {} byte(s)"sv, bytes);
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
                // check sequence number
                sequence_number = frame.sequence_number;
                log::info<5>("sequence_number={}"sv, sequence_number);
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
    if (!sbe::Parser::dispatch(*this, message, trace_info)) {
      log::warn("{}"sv, debug::hex::Message{message});
      log::fatal("Failed to parse message"sv);
    }
  };
  auto reset = [&]() {
    ++counter_.sequence_reset;
    log::warn<1>("*** RESUBSCRIBE ALL SYMBOLS ***"sv);
    for (auto &[security_id, collector] : channel_.mbp_collector) {
      if (collector.ready()) {
        get_security(shared_, security_id, [&](auto &security) {
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
          create_trace_and_dispatch(handler_, trace_info, market_by_price_update, true, false);
        });
      }
      collector.clear();
      channel_.mbp_last_sequence.erase(security_id);
    }
  };
  drain(*receiver_, channel_, parse, reset);
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
    log::info<5>("admin_heartbeat={}, frame={}"sv, value, frame);
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
    log::info<5>("channel_reset={}, frame={}"sv, value, frame);
  });
}

void UDPIncremental::operator()(Trace<cme_mdp::SecurityStatus30> const &event, sbe::Frame const &frame) {
  profile_.security_status([&]() {
    auto &trace_info = event.trace_info;
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("security_status={}, frame={}"sv, value, frame);
    value.sbeRewind();  // note!
    auto security_id = value.securityID();
    get_security(shared_, security_id, [&](auto &security) {
      auto market_status = MarketStatus{
          .stream_id = stream_id_,
          .exchange = security.exchange,
          .symbol = security.symbol,
          .trading_status = sbe::map_security_trading_status(value.securityTradingStatus()),
      };
      create_trace_and_dispatch(handler_, trace_info, std::as_const(market_status), true);
    });
  });
}

void UDPIncremental::operator()(Trace<cme_mdp::MDInstrumentDefinitionFuture54> const &event, sbe::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_instrument_definition_future={}, frame={}"sv, value, frame);
}

void UDPIncremental::operator()(Trace<cme_mdp::MDInstrumentDefinitionOption55> const &event, sbe::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_instrument_definition_option={}, frame={}"sv, value, frame);
}

void UDPIncremental::operator()(Trace<cme_mdp::MDInstrumentDefinitionSpread56> const &event, sbe::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_instrument_definition_spread={}, frame={}"sv, value, frame);
}

void UDPIncremental::operator()(
    Trace<cme_mdp::MDInstrumentDefinitionFixedIncome57> const &event, sbe::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_instrument_definition_fixed_income={}, frame={}"sv, value, frame);
}

void UDPIncremental::operator()(Trace<cme_mdp::MDInstrumentDefinitionRepo58> const &event, sbe::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_instrument_definition_repo={}, frame={}"sv, value, frame);
}

void UDPIncremental::operator()(Trace<cme_mdp::MDInstrumentDefinitionFX63> const &event, sbe::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_instrument_definition_fx={}, frame={}"sv, value, frame);
}

void UDPIncremental::operator()(Trace<cme_mdp::SnapshotFullRefresh52> const &event, sbe::Frame const &frame) {
  profile_.snapshot_full_refresh([&]() {
    auto &trace_info = event.trace_info;
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("snapshot_full_refresh={}, frame={}"sv, value, frame);
    value.sbeRewind();  // note!
    auto security_id = value.securityID();
    get_security(shared_, security_id, [&](auto &security) {
      std::chrono::nanoseconds exchange_time_utc{value.transactTime()};
      auto exchange_sequence = value.lastMsgSeqNumProcessed();
      Layer layer = {};
      core::back_emplacer bids{shared_.bids}, asks{shared_.asks};
      core::back_emplacer statistics{shared_.statistics};
      value.noMDEntries().forEach(
          [&](auto const &item) { emplace_back(item, security, layer, bids, asks, statistics); });
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
      if (!(std::empty(bids) && std::empty(bids))) {
        dispatch_market_by_price(trace_info, security_id, security, exchange_sequence, exchange_time_utc, bids, asks);
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
    log::info<5>("snapshot_full_refresh_long_qty={}, frame={}"sv, value, frame);
  });
}

void UDPIncremental::operator()(Trace<cme_mdp::MDIncrementalRefreshBook46> const &event, sbe::Frame const &frame) {
  profile_.md_incremental_refresh_book([&]() {
    auto &trace_info = event.trace_info;
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("md_incremental_refresh_book={}, frame={}"sv, value, frame);
    value.sbeRewind();  // note!
    uint32_t exchange_sequence = frame.sequence_number;
    std::chrono::nanoseconds exchange_time_utc{value.transactTime()};
    Layer layer = {};
    core::back_emplacer bids{shared_.bids}, asks{shared_.asks};
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
      if (!(std::empty(bids) && std::empty(bids))) {
        dispatch_market_by_price(trace_info, security_id, security, exchange_sequence, exchange_time_utc, bids, asks);
        bids.clear();
        asks.clear();
      }
    };
    int32_t security_id = {};
    Shared::Security *security = nullptr;
    value.noMDEntries().forEach([&](auto const &item) {
      auto current_security_id = item.securityID();
      if (current_security_id != security_id) {
        if (security)
          dispatch(security_id, *security, true);
        security_id = current_security_id;
        if (get_security(shared_, security_id, [&security](auto &security_) { security = &security_; })) {
        } else {
          security = nullptr;
        }
      }
      if (security)
        emplace_back(item, *security, layer, bids, asks);
    });
    if (security)
      dispatch(security_id, *security, true);
  });
}

void UDPIncremental::operator()(
    Trace<cme_mdp::MDIncrementalRefreshBookLongQty64> const &event, sbe::Frame const &frame) {
  profile_.md_incremental_refresh_book_long_qty([&]() {
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("md_incremental_refresh_book_long_qty={}, frame={}"sv, value, frame);
  });
}

void UDPIncremental::operator()(Trace<cme_mdp::SnapshotFullRefreshOrderBook53> const &event, sbe::Frame const &frame) {
  profile_.snapshot_full_refresh_order_book([&]() {
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("snapshot_full_refresh_order_book={}, frame={}"sv, value, frame);
  });
}

void UDPIncremental::operator()(Trace<cme_mdp::MDIncrementalRefreshOrderBook47> const &event, sbe::Frame const &frame) {
  profile_.md_incremental_refresh_order_book([&]() {
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("md_incremental_refresh_order_book={}, frame={}"sv, value, frame);
  });
}

void UDPIncremental::operator()(
    Trace<cme_mdp::MDIncrementalRefreshTradeSummary48> const &event, sbe::Frame const &frame) {
  profile_.md_incremental_refresh_trade_summary([&]() {
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("md_incremental_refresh_trade_summary={}, frame={}"sv, value, frame);
    dispatch_trade_summary(event);
  });
}

void UDPIncremental::operator()(
    Trace<cme_mdp::MDIncrementalRefreshTradeSummaryLongQty65> const &event, sbe::Frame const &frame) {
  profile_.md_incremental_refresh_trade_summary_long_qty([&]() {
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("md_incremental_refresh_trade_summary_long_qty={}, frame={}"sv, value, frame);
    dispatch_trade_summary(event);
  });
}

void UDPIncremental::operator()(
    Trace<cme_mdp::MDIncrementalRefreshDailyStatistics49> const &event, sbe::Frame const &frame) {
  profile_.md_incremental_refresh_daily_statistics([&]() {
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("md_incremental_refresh_daily_statistics={}, frame={}"sv, value, frame);
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
    log::info<5>("md_incremental_refresh_session_statistics={}, frame={}"sv, value, frame);
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
    log::info<5>("md_incremental_refresh_session_statistics_long_qty={}, frame={}"sv, value, frame);
    dispatch_statistics(event, [](auto &statistics, auto &item, auto &security) {
      statistics_emplace_back(statistics, item, security);
    });
  });
}

void UDPIncremental::operator()(Trace<cme_mdp::MDIncrementalRefreshVolume37> const &event, sbe::Frame const &frame) {
  profile_.md_incremental_refresh_volume([&]() {
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("md_incremental_refresh_volume={}, frame={}"sv, value, frame);
    dispatch_statistics(event, [](auto &statistics, auto &item, [[maybe_unused]] auto &security) {
      statistics.emplace_back([&](auto &result) { emplace_size(result, StatisticsType::TRADE_VOLUME, item); });
    });
  });
}

void UDPIncremental::operator()(
    Trace<cme_mdp::MDIncrementalRefreshVolumeLongQty66> const &event, sbe::Frame const &frame) {
  profile_.md_incremental_refresh_volume_long_qty([&]() {
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("md_incremental_refresh_volume_long_qty={}, frame={}"sv, value, frame);
  });
}

void UDPIncremental::operator()(
    Trace<cme_mdp::MDIncrementalRefreshLimitsBanding50> const &event, sbe::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_limits_banding={}, frame={}"sv, value, frame);
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
      create_trace_and_dispatch(handler_, trace_info, market_by_price_update, true, false);
    };
    auto publish_snapshot = [&](auto &bids, auto &asks, [[maybe_unused]] auto exchange_sequence) {
      auto market_by_price_update = create_update(bids, asks, UpdateType::SNAPSHOT);
      create_trace_and_dispatch(handler_, trace_info, market_by_price_update, true, false);
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

template <typename T>
void UDPIncremental::dispatch_trade_summary(Trace<T> const &event) {
  auto &trace_info = event.trace_info;
  using value_type = typename std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  value.sbeRewind();                                    // note!
  std::chrono::nanoseconds exchange_time_utc{value.transactTime()};
  core::back_emplacer trades{shared_.trades};
  auto dispatch = [&](auto &security, auto is_last) {
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
    create_trace_and_dispatch(handler_, trace_info, trade_summary, is_last);
    trades.clear();
  };
  int32_t security_id = {};
  Shared::Security *security = nullptr;
  value.noMDEntries().forEach([&](auto const &item) {
    auto current_security_id = item.securityID();
    if (current_security_id != security_id) {
      if (security)
        dispatch(*security, true);
      security_id = current_security_id;
      if (get_security(shared_, security_id, [&security](auto &security_) { security = &security_; })) {
      } else {
        security = nullptr;
      }
    }
    if (security)
      trade_summary_emplace_back(trades, item, *security);
  });
  if (security)
    dispatch(*security, true);
}

template <typename T, typename Callback>
void UDPIncremental::dispatch_statistics(Trace<T> const &event, Callback callback) {
  auto &trace_info = event.trace_info;
  using value_type = typename std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  value.sbeRewind();                                    // note!
  std::chrono::nanoseconds exchange_time_utc{value.transactTime()};
  core::back_emplacer statistics{shared_.statistics};
  auto dispatch = [&](auto &security, auto is_last) {
    if (!std::empty(statistics)) {
      auto statistics_update = StatisticsUpdate{
          .stream_id = stream_id_,
          .exchange = security.exchange,
          .symbol = security.symbol,
          .statistics = statistics,
          .update_type = UpdateType::INCREMENTAL,
          .exchange_time_utc = exchange_time_utc,
      };
      log::info<3>("statistics_update={}"sv, statistics_update);
      create_trace_and_dispatch(handler_, trace_info, statistics_update, is_last);
      statistics.clear();
    }
  };
  int32_t security_id = {};
  Shared::Security *security = nullptr;
  value.noMDEntries().forEach([&](auto const &item) {
    auto current_security_id = item.securityID();
    if (current_security_id != security_id) {
      if (security)
        dispatch(*security, true);
      security_id = current_security_id;
      if (get_security(shared_, security_id, [&security](auto &security_) { security = &security_; })) {
      } else {
        security = nullptr;
      }
    }
    if (security)
      callback(statistics, item, *security);
  });
  if (security)
    dispatch(*security, true);
}

void UDPIncremental::publish_stream_status(TraceInfo const &trace_info, ConnectionStatus connection_status) {
  if (!utils::update(connection_status_, connection_status))
    return;
  auto stream_status = StreamStatus{
      .stream_id = stream_id_,
      .account = {},
      .supports = SUPPORTS,
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
      .write(profile_.md_incremental_refresh_volume_long_qty, metrics::PROFILE);
}

}  // namespace cme
}  // namespace roq
