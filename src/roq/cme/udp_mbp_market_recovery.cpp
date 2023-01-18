/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/cme/udp_mbp_market_recovery.hpp"

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
auto const NAME = "S"sv;

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

auto create_receiver(auto &handler, auto &context, auto &shared, auto &channel_id, auto priority) {
  log::info(R"(Create channel_id="{}, priority={}")"sv, channel_id, priority);
  auto [multicast_address, port] = shared.get_multicast_config(channel_id, multicast::Type::SNAPSHOT, priority);
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

template <typename Callback>
bool get_last_exchange_sequence(auto &channel, auto security_id, auto &value, Callback callback) {
  auto last_processed = value.lastMsgSeqNumProcessed();
  if (last_processed <= channel.last_sequence.second) {
    auto iter = channel.mbp_last_sequence.find(security_id);
    if (iter != std::end(channel.mbp_last_sequence)) {
      callback((*iter).second);
      return true;
    }
  }
  return false;
}

template <typename T, typename U, typename std::enable_if<std::is_same<T, MBPUpdate>::value, int>::type = 0>
void emplace(T &result, U const &item, auto &security) {
  auto price = sbe::get_double(const_cast<U &>(item).mDEntryPx());
  auto quantity = sbe::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue());
  auto number_of_orders = sbe::get_int(item.numberOfOrders(), item.numberOfOrdersNullValue());
  auto md_price_level = sbe::get_int(item.mDPriceLevel(), item.mDPriceLevelNullValue());
  uint32_t price_level = md_price_level > 0 ? (md_price_level - 1) : 0;
  new (&result) T{
      .price = price * security.display_factor,
      .quantity = utils::safe_cast(quantity),
      .implied_quantity = NaN,
      .number_of_orders = utils::safe_cast(number_of_orders),
      .update_action = {},
      .price_level = price_level,
  };
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

template <typename T, typename std::enable_if<std::is_same<T, Statistics>::value, int>::type = 0>
void emplace_size(T &result, auto type, auto const &item) {
  auto value = sbe::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue());
  new (&result) T{
      .type = type,
      .value = utils::safe_cast(value),
      .begin_time_utc = {},
      .end_time_utc = {},
  };
}

template <typename T>
void emplace_back(T const &item, auto &security, auto &top_of_book, auto &bids, auto &asks, auto &statistics) {
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
      auto price = sbe::get_double(const_cast<T &>(item).mDEntryPx());
      top_of_book.ask_price = price * security.display_factor;
      break;
    }
    case MarketBestBid: {
      auto price = sbe::get_double(const_cast<T &>(item).mDEntryPx());
      top_of_book.bid_price = price * security.display_factor;
      break;
    }
    case NULL_VALUE:
      break;
  }
}
}  // namespace

// === IMPLEMENTATION ===

UDPMBPMarketRecovery::UDPMBPMarketRecovery(
    Handler &handler, io::Context &context, uint16_t stream_id, Shared &shared, Channel &channel)
    : handler_{handler}, stream_id_{stream_id}, name_{create_name(stream_id_, channel.channel_id)},
      receiver_{create_receiver(*this, context, shared, channel.channel_id, Priority::PRIMARY)},
      counter_{
          .disconnect = create_metrics(name_, "disconnect"sv),
      },
      profile_{
          .parse = create_metrics(name_, "parse"sv),
          .admin_heartbeat = create_metrics(name_, "admin_heartbeat"sv),
          .channel_reset = create_metrics(name_, "channel_resetparse"sv),
          .snapshot_full_refresh = create_metrics(name_, "snapshot_full_refresh"sv),
          .snapshot_full_refresh_long_qty = create_metrics(name_, "snapshot_full_refresh_long_qty"sv),
      },
      shared_{shared}, channel_{channel} {
}

void UDPMBPMarketRecovery::operator()(Event<Start> const &) {
  TraceInfo trace_info;
  publish_stream_status(trace_info, ConnectionStatus::CONNECTING);
  last_update_time_ = trace_info.source_receive_time;
}

void UDPMBPMarketRecovery::operator()(Event<Stop> const &) {
}

void UDPMBPMarketRecovery::operator()(Event<Timer> const &event) {
  if (last_update_time_.count() && (last_update_time_ + flags::Multicast::multicast_timeout()) < event.value.now) {
    log::warn("*** DETECTED TIMEOUT ***"sv);
    last_update_time_ = {};
  }
}

void UDPMBPMarketRecovery::operator()(io::net::udp::Receiver::Read const &) {
  TraceInfo trace_info;
  last_update_time_ = trace_info.source_receive_time;
  publish_stream_status(trace_info, ConnectionStatus::READY);  // first message will publish
  while (receive_buffer_.append(*receiver_)) {
    profile_.parse([&]() {
      auto message = receive_buffer_.data();
      log::info<5>("received {} byte(s)"sv, std::size(message));
      log::info<5>("{}"sv, debug::hex::Message{message});
      if (!sbe::Parser::dispatch(*this, message, trace_info)) {
        log::warn("{}"sv, debug::hex::Message{message});
        log::fatal("Failed to parse message"sv);
      }
      receive_buffer_.clear();
    });
  }
}

void UDPMBPMarketRecovery::operator()(io::net::udp::Receiver::Error const &error) {
  log::fatal("Error: what={}"sv, error.what);
}

// sbe::Parser::Handler

void UDPMBPMarketRecovery::operator()(sbe::Frame const &) {
}

void UDPMBPMarketRecovery::operator()(Trace<cme_mdp::AdminHeartbeat12> const &event, sbe::Frame const &frame) {
  profile_.admin_heartbeat([&]() {
    auto &trace_info = event.trace_info;
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("admin_heartbeat={}, frame={}"sv, value, frame);
    auto external_latency = ExternalLatency{
        .stream_id = stream_id_,
        .account = {},
        .latency = trace_info.origin_create_time_utc - frame.sending_time,
    };
    create_trace_and_dispatch(handler_, trace_info, external_latency);
  });
}

void UDPMBPMarketRecovery::operator()(Trace<cme_mdp::ChannelReset4> const &event, sbe::Frame const &frame) {
  profile_.channel_reset([&]() {
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("channel_reset={}, frame={}"sv, value, frame);
  });
}

void UDPMBPMarketRecovery::operator()(Trace<cme_mdp::SecurityStatus30> const &event, sbe::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("security_status={}, frame={}"sv, value, frame);
}

void UDPMBPMarketRecovery::operator()(
    Trace<cme_mdp::MDInstrumentDefinitionFuture54> const &event, sbe::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_instrument_definition_future={}, frame={}"sv, value, frame);
}

void UDPMBPMarketRecovery::operator()(
    Trace<cme_mdp::MDInstrumentDefinitionOption55> const &event, sbe::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_instrument_definition_option={}, frame={}"sv, value, frame);
}

void UDPMBPMarketRecovery::operator()(
    Trace<cme_mdp::MDInstrumentDefinitionSpread56> const &event, sbe::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_instrument_definition_spread={}, frame={}"sv, value, frame);
}

void UDPMBPMarketRecovery::operator()(
    Trace<cme_mdp::MDInstrumentDefinitionFixedIncome57> const &event, sbe::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_instrument_definition_fixed_income={}, frame={}"sv, value, frame);
}

void UDPMBPMarketRecovery::operator()(
    Trace<cme_mdp::MDInstrumentDefinitionRepo58> const &event, sbe::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_instrument_definition_repo={}, frame={}"sv, value, frame);
}

void UDPMBPMarketRecovery::operator()(
    Trace<cme_mdp::MDInstrumentDefinitionFX63> const &event, sbe::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_instrument_definition_fx={}, frame={}"sv, value, frame);
}

void UDPMBPMarketRecovery::operator()(Trace<cme_mdp::SnapshotFullRefresh52> const &event, sbe::Frame const &frame) {
  profile_.snapshot_full_refresh([&]() {
    auto &trace_info = event.trace_info;
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("snapshot_full_refresh={}, frame={}"sv, value, frame);
    auto security_id = value.securityID();
    get_security(shared_, security_id, [&](auto &security) {
      get_last_exchange_sequence(channel_, security_id, value, [&](auto exchange_sequence) {
        std::chrono::nanoseconds exchange_time_utc{value.transactTime()};
        Layer layer = {};
        core::back_emplacer bids{shared_.bids}, asks{shared_.asks};
        core::back_emplacer statistics{shared_.statistics};
        value.sbeRewind();  // note!
        value.noMDEntries().forEach(
            [&](auto const &item) { emplace_back(item, security, layer, bids, asks, statistics); });
        if (!(std::isnan(layer.bid_price) && std::isnan(layer.ask_price))) {
          /* note! we don't publish
          TopOfBook top_of_book{
              .stream_id = stream_id_,
              .exchange = security.exchange,
              .symbol = security.symbol,
              .layer = layer,
              .exchange_time_utc = exchange_time_utc,
              .exchange_sequence = exchange_sequence,
          };
          */
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
  });
}

void UDPMBPMarketRecovery::operator()(
    Trace<cme_mdp::SnapshotFullRefreshLongQty69> const &event, sbe::Frame const &frame) {
  profile_.snapshot_full_refresh_long_qty([&]() {
    auto &trace_info = event.trace_info;
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("snapshot_full_refresh_long_qty={}, frame={}"sv, value, frame);
    auto security_id = value.securityID();
    get_security(shared_, security_id, [&](auto &security) {
      get_last_exchange_sequence(channel_, security_id, value, [&](auto exchange_sequence) {
        std::chrono::nanoseconds exchange_time_utc{value.transactTime()};
        Layer layer = {};
        core::back_emplacer bids{shared_.bids}, asks{shared_.asks};
        core::back_emplacer statistics{shared_.statistics};
        value.sbeRewind();  // note!
        value.noMDEntries().forEach(
            [&](auto const &item) { emplace_back(item, security, layer, bids, asks, statistics); });
        if (!(std::isnan(layer.bid_price) && std::isnan(layer.ask_price))) {
          /* note! we don't publish
          TopOfBook top_of_book{
              .stream_id = stream_id_,
              .exchange = security.exchange,
              .symbol = security.symbol,
              .layer = layer,
              .exchange_time_utc = exchange_time_utc,
              .exchange_sequence = exchange_sequence,
          };
          */
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
  });
}

void UDPMBPMarketRecovery::operator()(
    Trace<cme_mdp::MDIncrementalRefreshBook46> const &event, sbe::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_book={}, frame={}"sv, value, frame);
}

void UDPMBPMarketRecovery::operator()(
    Trace<cme_mdp::MDIncrementalRefreshBookLongQty64> const &event, sbe::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_book_long_qty={}, frame={}"sv, value, frame);
}

void UDPMBPMarketRecovery::operator()(
    Trace<cme_mdp::SnapshotFullRefreshOrderBook53> const &event, sbe::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("snapshot_full_refresh_order_book={}, frame={}"sv, value, frame);
}

void UDPMBPMarketRecovery::operator()(
    Trace<cme_mdp::MDIncrementalRefreshOrderBook47> const &event, sbe::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_order_book={}, frame={}"sv, value, frame);
}

void UDPMBPMarketRecovery::operator()(
    Trace<cme_mdp::MDIncrementalRefreshTradeSummary48> const &event, sbe::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_trade_summary={}, frame={}"sv, value, frame);
}

void UDPMBPMarketRecovery::operator()(
    Trace<cme_mdp::MDIncrementalRefreshTradeSummaryLongQty65> const &event, sbe::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_trade_summary_long_qty={}, frame={}"sv, value, frame);
}

void UDPMBPMarketRecovery::operator()(
    Trace<cme_mdp::MDIncrementalRefreshDailyStatistics49> const &event, sbe::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_daily_statistics={}, frame={}"sv, value, frame);
}

void UDPMBPMarketRecovery::operator()(
    Trace<cme_mdp::MDIncrementalRefreshSessionStatistics51> const &event, sbe::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_session_statistics={}, frame={}"sv, value, frame);
}

void UDPMBPMarketRecovery::operator()(
    Trace<cme_mdp::MDIncrementalRefreshSessionStatisticsLongQty67> const &event, sbe::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_session_statistics_long_qty={}, frame={}"sv, value, frame);
}

void UDPMBPMarketRecovery::operator()(
    Trace<cme_mdp::MDIncrementalRefreshVolume37> const &event, sbe::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_volume={}, frame={}"sv, value, frame);
}

void UDPMBPMarketRecovery::operator()(
    Trace<cme_mdp::MDIncrementalRefreshVolumeLongQty66> const &event, sbe::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_volume_long_qty={}, frame={}"sv, value, frame);
}

void UDPMBPMarketRecovery::operator()(
    Trace<cme_mdp::MDIncrementalRefreshLimitsBanding50> const &event, sbe::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_limits_banding={}, frame={}"sv, value, frame);
}

void UDPMBPMarketRecovery::dispatch_market_by_price(
    auto &trace_info,
    auto security_id,
    auto &security,
    auto exchange_sequence,
    auto exchange_time_utc,
    auto &bids,
    auto &asks) {
  auto iter = channel_.mbp_resubscribe.find(security_id);
  if (iter == std::end(channel_.mbp_resubscribe))
    return;
  auto &collector = channel_.mbp_collector[security_id];
  try {
    auto publish_snapshot = [&](auto &bids, auto &asks, auto exchange_sequence) {
      auto market_by_price_update = MarketByPriceUpdate{
          .stream_id = stream_id_,
          .exchange = security.exchange,
          .symbol = security.symbol,
          .bids = bids,
          .asks = asks,
          .update_type = UpdateType::SNAPSHOT,
          .exchange_time_utc = exchange_time_utc,
          .exchange_sequence = collector.last_sequence(),
          .price_decimals = {},
          .quantity_decimals = {},
          .checksum = {},
      };
      Trace event(trace_info, market_by_price_update);
      shared_(event, true, [&](auto &market_by_price) { collector.apply(market_by_price, exchange_sequence, false); });
      channel_.mbp_resubscribe.erase(security_id);  // remove
    };
    auto request_snapshot = [&]([[maybe_unused]] auto retries) {
      // note! wait for next snapshot
      channel_.mbp_resubscribe.emplace(security_id, exchange_sequence);
    };
    collector(bids, asks, exchange_sequence, publish_snapshot, request_snapshot);
  } catch (BadState &) {
    log::warn(
        R"(RESUBSCRIBE exchange="{}", symbol="{}", security_id={})"sv, security.exchange, security.symbol, security_id);
    // XXX HANS publish stale
    collector.clear();  // note! wait for next incremental
  }
}

void UDPMBPMarketRecovery::publish_stream_status(TraceInfo const &trace_info, ConnectionStatus connection_status) {
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

void UDPMBPMarketRecovery::operator()(metrics::Writer &writer) {
  writer  //
      .write(counter_.disconnect, metrics::COUNTER)
      .write(profile_.parse, metrics::PROFILE)
      .write(profile_.admin_heartbeat, metrics::PROFILE)
      .write(profile_.channel_reset, metrics::PROFILE)
      .write(profile_.snapshot_full_refresh, metrics::PROFILE)
      .write(profile_.snapshot_full_refresh_long_qty, metrics::PROFILE);
}

}  // namespace cme
}  // namespace roq
