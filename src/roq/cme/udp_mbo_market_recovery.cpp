/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/cme/udp_mbo_market_recovery.hpp"

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
auto const NAME = "O"sv;

auto const SUPPORTS = Mask{
    SupportType::MARKET_BY_ORDER,
};
}  // namespace

// === HELPERS ===

namespace {
auto create_name(auto stream_id, auto const &channel_id) {
  return fmt::format("{}:{}{}"sv, stream_id, NAME, channel_id);
}

auto create_receiver(auto &handler, auto &context, auto &shared, auto &channel_id, auto priority) {
  log::info(R"(Create channel_id="{}, priority={}")"sv, channel_id, priority);
  auto [multicast_address, port] = shared.get_multicast_config(channel_id, multicast::Type::SNAPSHOT_MBO, priority);
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
    auto iter = channel.mbo_last_sequence.find(security_id);
    if (iter != std::end(channel.mbo_last_sequence)) {
      callback((*iter).second);
      return true;
    }
  }
  return false;
}

template <typename T, typename U, typename std::enable_if<std::is_same<T, MBOUpdate>::value, int>::type = 0>
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

UDPMBOMarketRecovery::UDPMBOMarketRecovery(
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
          .snapshot_full_refresh_order_book = create_metrics(name_, "snapshot_full_refresh_order_book"sv),
      },
      shared_{shared}, channel_{channel} {
}

void UDPMBOMarketRecovery::operator()(Event<Start> const &) {
  TraceInfo trace_info;
  publish_stream_status(trace_info, ConnectionStatus::CONNECTING);
  last_update_time_ = trace_info.source_receive_time;
}

void UDPMBOMarketRecovery::operator()(Event<Stop> const &) {
}

void UDPMBOMarketRecovery::operator()(Event<Timer> const &event) {
  if (last_update_time_.count() && (last_update_time_ + flags::Multicast::multicast_timeout()) < event.value.now) {
    log::warn("*** DETECTED TIMEOUT ***"sv);
    last_update_time_ = {};
  }
}

void UDPMBOMarketRecovery::operator()(io::net::udp::Receiver::Read const &) {
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

void UDPMBOMarketRecovery::operator()(io::net::udp::Receiver::Error const &error) {
  log::fatal("Error: what={}"sv, error.what);
}

// sbe::Parser::Handler

void UDPMBOMarketRecovery::operator()(sbe::Frame const &) {
}

void UDPMBOMarketRecovery::operator()(Trace<cme_mdp::AdminHeartbeat12> const &event, sbe::Frame const &frame) {
  profile_.admin_heartbeat([&]() {
    auto &trace_info = event.trace_info;
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("admin_heartbeat_12={}, frame={}"sv, value, frame);
    auto external_latency = ExternalLatency{
        .stream_id = stream_id_,
        .account = {},
        .latency = trace_info.origin_create_time_utc - frame.sending_time,
    };
    create_trace_and_dispatch(handler_, trace_info, external_latency);
  });
}

void UDPMBOMarketRecovery::operator()(Trace<cme_mdp::ChannelReset4> const &event, sbe::Frame const &frame) {
  profile_.channel_reset([&]() {
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("channel_reset_4={}, frame={}"sv, value, frame);
  });
}

void UDPMBOMarketRecovery::operator()(Trace<cme_mdp::SecurityStatus30> const &event, sbe::Frame const &frame) {
#ifndef NDEBUG
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("security_status_30={}, frame={}"sv, value, frame);
#endif
}

void UDPMBOMarketRecovery::operator()(
    Trace<cme_mdp::MDInstrumentDefinitionFuture54> const &event, sbe::Frame const &frame) {
#ifndef NDEBUG
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_instrument_definition_future_54={}, frame={}"sv, value, frame);
#endif
}

void UDPMBOMarketRecovery::operator()(
    Trace<cme_mdp::MDInstrumentDefinitionOption55> const &event, sbe::Frame const &frame) {
#ifndef NDEBUG
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_instrument_definition_option_55={}, frame={}"sv, value, frame);
#endif
}

void UDPMBOMarketRecovery::operator()(
    Trace<cme_mdp::MDInstrumentDefinitionSpread56> const &event, sbe::Frame const &frame) {
#ifndef NDEBUG
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_instrument_definition_spread_56={}, frame={}"sv, value, frame);
#endif
}

void UDPMBOMarketRecovery::operator()(
    Trace<cme_mdp::MDInstrumentDefinitionFixedIncome57> const &event, sbe::Frame const &frame) {
#ifndef NDEBUG
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_instrument_definition_fixed_income_57={}, frame={}"sv, value, frame);
#endif
}

void UDPMBOMarketRecovery::operator()(
    Trace<cme_mdp::MDInstrumentDefinitionRepo58> const &event, sbe::Frame const &frame) {
#ifndef NDEBUG
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_instrument_definition_repo_58={}, frame={}"sv, value, frame);
#endif
}

void UDPMBOMarketRecovery::operator()(
    Trace<cme_mdp::MDInstrumentDefinitionFX63> const &event, sbe::Frame const &frame) {
#ifndef NDEBUG
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_instrument_definition_fx_63={}, frame={}"sv, value, frame);
#endif
}

void UDPMBOMarketRecovery::operator()(Trace<cme_mdp::SnapshotFullRefresh52> const &event, sbe::Frame const &frame) {
#ifndef NDEBUG
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("snapshot_full_refresh_52={}, frame={}"sv, value, frame);
#endif
}

void UDPMBOMarketRecovery::operator()(
    Trace<cme_mdp::SnapshotFullRefreshLongQty69> const &event, sbe::Frame const &frame) {
#ifndef NDEBUG
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("snapshot_full_refresh_long_qty_69={}, frame={}"sv, value, frame);
#endif
}

void UDPMBOMarketRecovery::operator()(
    Trace<cme_mdp::MDIncrementalRefreshBook46> const &event, sbe::Frame const &frame) {
#ifndef NDEBUG
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_book_46={}, frame={}"sv, value, frame);
#endif
}

void UDPMBOMarketRecovery::operator()(
    Trace<cme_mdp::MDIncrementalRefreshBookLongQty64> const &event, sbe::Frame const &frame) {
#ifndef NDEBUG
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_book_long_qty_64={}, frame={}"sv, value, frame);
#endif
}

void UDPMBOMarketRecovery::operator()(
    Trace<cme_mdp::SnapshotFullRefreshOrderBook53> const &event, sbe::Frame const &frame) {
  profile_.snapshot_full_refresh_order_book([&]() {
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("snapshot_full_refresh_order_book_53={}, frame={}"sv, value, frame);
  });
}

void UDPMBOMarketRecovery::operator()(
    Trace<cme_mdp::MDIncrementalRefreshOrderBook47> const &event, sbe::Frame const &frame) {
#ifndef NDEBUG
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_order_book_47={}, frame={}"sv, value, frame);
#endif
}

void UDPMBOMarketRecovery::operator()(
    Trace<cme_mdp::MDIncrementalRefreshTradeSummary48> const &event, sbe::Frame const &frame) {
#ifndef NDEBUG
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_trade_summary_48={}, frame={}"sv, value, frame);
#endif
}

void UDPMBOMarketRecovery::operator()(
    Trace<cme_mdp::MDIncrementalRefreshTradeSummaryLongQty65> const &event, sbe::Frame const &frame) {
#ifndef NDEBUG
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_trade_summary_long_qty_65={}, frame={}"sv, value, frame);
#endif
}

void UDPMBOMarketRecovery::operator()(
    Trace<cme_mdp::MDIncrementalRefreshDailyStatistics49> const &event, sbe::Frame const &frame) {
#ifndef NDEBUG
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_daily_statistics_49={}, frame={}"sv, value, frame);
#endif
}

void UDPMBOMarketRecovery::operator()(
    Trace<cme_mdp::MDIncrementalRefreshSessionStatistics51> const &event, sbe::Frame const &frame) {
#ifndef NDEBUG
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_session_statistics_51={}, frame={}"sv, value, frame);
#endif
}

void UDPMBOMarketRecovery::operator()(
    Trace<cme_mdp::MDIncrementalRefreshSessionStatisticsLongQty67> const &event, sbe::Frame const &frame) {
#ifndef NDEBUG
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_session_statistics_long_qty_67={}, frame={}"sv, value, frame);
#endif
}

void UDPMBOMarketRecovery::operator()(
    Trace<cme_mdp::MDIncrementalRefreshVolume37> const &event, sbe::Frame const &frame) {
#ifndef NDEBUG
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_volume_37={}, frame={}"sv, value, frame);
#endif
}

void UDPMBOMarketRecovery::operator()(
    Trace<cme_mdp::MDIncrementalRefreshVolumeLongQty66> const &event, sbe::Frame const &frame) {
#ifndef NDEBUG
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_volume_long_qty_66={}, frame={}"sv, value, frame);
#endif
}

void UDPMBOMarketRecovery::operator()(
    Trace<cme_mdp::MDIncrementalRefreshLimitsBanding50> const &event, sbe::Frame const &frame) {
#ifndef NDEBUG
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_limits_banding_50={}, frame={}"sv, value, frame);
#endif
}

void UDPMBOMarketRecovery::operator()(Trace<cme_mdp::QuoteRequest39> const &event, sbe::Frame const &frame) {
#ifndef NDEBUG
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("quote_request_39={}, frame={}"sv, value, frame);
#endif
}

void UDPMBOMarketRecovery::publish_stream_status(TraceInfo const &trace_info, ConnectionStatus connection_status) {
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

void UDPMBOMarketRecovery::operator()(metrics::Writer &writer) {
  writer  //
      .write(counter_.disconnect, metrics::COUNTER)
      .write(profile_.parse, metrics::PROFILE)
      .write(profile_.admin_heartbeat, metrics::PROFILE)
      .write(profile_.channel_reset, metrics::PROFILE)
      .write(profile_.snapshot_full_refresh_order_book, metrics::PROFILE);
}

}  // namespace cme
}  // namespace roq
