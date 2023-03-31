/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/cme/udp_mbp_market_recovery.hpp"

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
auto const NAME = "S"sv;

auto const SUPPORTS = Mask{
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
  auto [multicast_address, port] = shared.get_multicast_config(channel_id, mdp::ConnectionType::SNAPSHOT, priority);
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

template <typename T>
void mbp_emplace_back(auto &result, T const &item, auto &security) {
  auto price = mdp::get_double(const_cast<T &>(item).mDEntryPx());
  auto quantity = mdp::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue());
  auto number_of_orders = mdp::get_int(item.numberOfOrders(), item.numberOfOrdersNullValue());
  auto md_price_level = mdp::get_int(item.mDPriceLevel(), item.mDPriceLevelNullValue());
  uint32_t price_level = md_price_level > 0 ? (md_price_level - 1) : 0;
  auto update = MBPUpdate{
      .price = price * security.display_factor,
      .quantity = utils::safe_cast(quantity),
      .implied_quantity = NaN,
      .number_of_orders = utils::safe_cast(number_of_orders),
      .update_action = {},
      .price_level = price_level,
  };
  result.emplace_back(std::move(update));
}

template <typename T>
void emplace_back(T const &item, auto &security, auto &bids, auto &asks) {
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
      break;
    case SettlementPrice:
      break;
    case TradingSessionHighPrice:
      break;
    case TradingSessionLowPrice:
      break;
    case VWAP:
      break;
    case ClearedVolume:
      break;
    case OpenInterest:
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
      break;
    case ElectronicVolume:
      break;
    case ThresholdLimitsandPriceBandVariation:
      break;
    case MarketBestOffer:
      break;
    case MarketBestBid:
      break;
    case NULL_VALUE:
      break;
  }
}

// note! don't care about re-ordering or dropped messages
void drain(auto &receiver, auto &buffer, auto stream_id, auto parse) {
  while (true) {
    // read into buffer
    auto bytes = receiver.recv(buffer);
    log::info<5>("Received {} byte(s) (stream_id={})"sv, bytes, stream_id);
    if (!bytes)
      return;
    // parse message
    std::span message{std::data(buffer), bytes};
    log::info<5>("{}"sv, debug::hex::Message{message});
    if (mdp::Frame::parse(message, [&](auto &frame) { log::info<5>("frame={}"sv, frame); })) {
    } else {
      // failed to parse frame
      log::warn("Unexpected"sv);
      continue;
    }
    parse(message);
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
          .channel_reset = create_metrics(name_, "channel_reset"sv),
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
  auto parse = [&](auto &message) {
    if (!mdp::Parser::dispatch(*this, message, trace_info)) {
      log::warn("{}"sv, debug::hex::Message{message});
      log::fatal("Failed to parse message"sv);
    }
  };
  drain(*receiver_, shared_.buffer, stream_id_, parse);
}

void UDPMBPMarketRecovery::operator()(io::net::udp::Receiver::Error const &error) {
  log::fatal("Error: what={}"sv, error.what);
}

// mdp::Parser::Handler

void UDPMBPMarketRecovery::operator()(mdp::Frame const &) {
}

void UDPMBPMarketRecovery::operator()(Trace<cme_mdp::AdminHeartbeat12> const &event, mdp::Frame const &frame) {
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

void UDPMBPMarketRecovery::operator()(Trace<cme_mdp::ChannelReset4> const &event, mdp::Frame const &frame) {
  profile_.channel_reset([&]() {
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("channel_reset_4={}, frame={}"sv, value, frame);
  });
}

void UDPMBPMarketRecovery::operator()(Trace<cme_mdp::SecurityStatus30> const &event, mdp::Frame const &frame) {
#ifndef NDEBUG
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("security_status_30={}, frame={}"sv, value, frame);
#endif
}

void UDPMBPMarketRecovery::operator()(
    Trace<cme_mdp::MDInstrumentDefinitionFuture54> const &event, mdp::Frame const &frame) {
#ifndef NDEBUG
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_instrument_definition_future_54={}, frame={}"sv, value, frame);
#endif
}

void UDPMBPMarketRecovery::operator()(
    Trace<cme_mdp::MDInstrumentDefinitionOption55> const &event, mdp::Frame const &frame) {
#ifndef NDEBUG
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_instrument_definition_option_55={}, frame={}"sv, value, frame);
#endif
}

void UDPMBPMarketRecovery::operator()(
    Trace<cme_mdp::MDInstrumentDefinitionSpread56> const &event, mdp::Frame const &frame) {
#ifndef NDEBUG
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_instrument_definition_spread_56={}, frame={}"sv, value, frame);
#endif
}

void UDPMBPMarketRecovery::operator()(
    Trace<cme_mdp::MDInstrumentDefinitionFixedIncome57> const &event, mdp::Frame const &frame) {
#ifndef NDEBUG
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_instrument_definition_fixed_income_57={}, frame={}"sv, value, frame);
#endif
}

void UDPMBPMarketRecovery::operator()(
    Trace<cme_mdp::MDInstrumentDefinitionRepo58> const &event, mdp::Frame const &frame) {
#ifndef NDEBUG
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_instrument_definition_repo_58={}, frame={}"sv, value, frame);
#endif
}

void UDPMBPMarketRecovery::operator()(
    Trace<cme_mdp::MDInstrumentDefinitionFX63> const &event, mdp::Frame const &frame) {
#ifndef NDEBUG
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_instrument_definition_fx_63={}, frame={}"sv, value, frame);
#endif
}

void UDPMBPMarketRecovery::operator()(Trace<cme_mdp::SnapshotFullRefresh52> const &event, mdp::Frame const &frame) {
  profile_.snapshot_full_refresh([&]() {
    auto &trace_info = event.trace_info;
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("snapshot_full_refresh_52={}, frame={}"sv, value, frame);
    auto security_id = value.securityID();
    shared_.get_security(security_id, [&](auto &security) {
      if (!security.mbp.resubscribe)
        return;
      auto last_msg_seq_num_processed = value.lastMsgSeqNumProcessed();
      if (last_msg_seq_num_processed < security.mbp.resubscribe)
        return;
      auto transact_time = std::chrono::nanoseconds{value.transactTime()};
      auto &mbp = shared_.get_mbp();
      value.sbeRewind();  // note!
      value.noMDEntries().forEach([&](auto const &item) { emplace_back(item, security, mbp.bids, mbp.asks); });
      if (!std::empty(mbp)) {
        dispatch_market_by_price(
            trace_info,
            security_id,
            security,
            last_msg_seq_num_processed,
            transact_time,
            frame.sending_time,
            mbp.bids,
            mbp.asks);
      }
    });
  });
}

void UDPMBPMarketRecovery::operator()(
    Trace<cme_mdp::SnapshotFullRefreshLongQty69> const &event, mdp::Frame const &frame) {
  profile_.snapshot_full_refresh_long_qty([&]() {
    auto &trace_info = event.trace_info;
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("snapshot_full_refresh_long_qty_69={}, frame={}"sv, value, frame);
    auto security_id = value.securityID();
    shared_.get_security(security_id, [&](auto &security) {
      if (!security.mbp.resubscribe)
        return;
      auto last_msg_seq_num_processed = value.lastMsgSeqNumProcessed();
      if (last_msg_seq_num_processed < security.mbp.resubscribe)
        return;
      auto transact_time = std::chrono::nanoseconds{value.transactTime()};
      auto &mbp = shared_.get_mbp();
      value.sbeRewind();  // note!
      value.noMDEntries().forEach([&](auto const &item) { emplace_back(item, security, mbp.bids, mbp.asks); });
      if (!std::empty(mbp)) {
        dispatch_market_by_price(
            trace_info,
            security_id,
            security,
            last_msg_seq_num_processed,
            transact_time,
            frame.sending_time,
            mbp.bids,
            mbp.asks);
      }
    });
  });
}

void UDPMBPMarketRecovery::operator()(
    Trace<cme_mdp::MDIncrementalRefreshBook46> const &event, mdp::Frame const &frame) {
#ifndef NDEBUG
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_book_46={}, frame={}"sv, value, frame);
#endif
}

void UDPMBPMarketRecovery::operator()(
    Trace<cme_mdp::MDIncrementalRefreshBookLongQty64> const &event, mdp::Frame const &frame) {
#ifndef NDEBUG
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_book_long_qty_64={}, frame={}"sv, value, frame);
#endif
}

void UDPMBPMarketRecovery::operator()(
    Trace<cme_mdp::SnapshotFullRefreshOrderBook53> const &event, mdp::Frame const &frame) {
#ifndef NDEBUG
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("snapshot_full_refresh_order_book_53={}, frame={}"sv, value, frame);
#endif
}

void UDPMBPMarketRecovery::operator()(
    Trace<cme_mdp::MDIncrementalRefreshOrderBook47> const &event, mdp::Frame const &frame) {
#ifndef NDEBUG
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_order_book_47={}, frame={}"sv, value, frame);
#endif
}

void UDPMBPMarketRecovery::operator()(
    Trace<cme_mdp::MDIncrementalRefreshTradeSummary48> const &event, mdp::Frame const &frame) {
#ifndef NDEBUG
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_trade_summary_48={}, frame={}"sv, value, frame);
#endif
}

void UDPMBPMarketRecovery::operator()(
    Trace<cme_mdp::MDIncrementalRefreshTradeSummaryLongQty65> const &event, mdp::Frame const &frame) {
#ifndef NDEBUG
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_trade_summary_long_qty_65={}, frame={}"sv, value, frame);
#endif
}

void UDPMBPMarketRecovery::operator()(
    Trace<cme_mdp::MDIncrementalRefreshDailyStatistics49> const &event, mdp::Frame const &frame) {
#ifndef NDEBUG
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_daily_statistics_49={}, frame={}"sv, value, frame);
#endif
}

void UDPMBPMarketRecovery::operator()(
    Trace<cme_mdp::MDIncrementalRefreshSessionStatistics51> const &event, mdp::Frame const &frame) {
#ifndef NDEBUG
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_session_statistics_51={}, frame={}"sv, value, frame);
#endif
}

void UDPMBPMarketRecovery::operator()(
    Trace<cme_mdp::MDIncrementalRefreshSessionStatisticsLongQty67> const &event, mdp::Frame const &frame) {
#ifndef NDEBUG
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_session_statistics_long_qty_67={}, frame={}"sv, value, frame);
#endif
}

void UDPMBPMarketRecovery::operator()(
    Trace<cme_mdp::MDIncrementalRefreshVolume37> const &event, mdp::Frame const &frame) {
#ifndef NDEBUG
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_volume_37={}, frame={}"sv, value, frame);
#endif
}

void UDPMBPMarketRecovery::operator()(
    Trace<cme_mdp::MDIncrementalRefreshVolumeLongQty66> const &event, mdp::Frame const &frame) {
#ifndef NDEBUG
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_volume_long_qty_66={}, frame={}"sv, value, frame);
#endif
}

void UDPMBPMarketRecovery::operator()(
    Trace<cme_mdp::MDIncrementalRefreshLimitsBanding50> const &event, mdp::Frame const &frame) {
#ifndef NDEBUG
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_limits_banding_50={}, frame={}"sv, value, frame);
#endif
}

void UDPMBPMarketRecovery::operator()(Trace<cme_mdp::QuoteRequest39> const &event, mdp::Frame const &frame) {
#ifndef NDEBUG
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("quote_request_39={}, frame={}"sv, value, frame);
#endif
}

void UDPMBPMarketRecovery::dispatch_market_by_price(
    auto &trace_info,
    auto security_id,
    auto &security,
    auto exchange_sequence,
    auto exchange_time_utc,
    auto sending_time_utc,
    auto &bids,
    auto &asks) {
  if (!security.mbp.resubscribe)
    return;
  auto &sequencer = security.mbp.sequencer;
  try {
    auto publish_snapshot = [&](auto &bids, auto &asks, auto exchange_sequence) {
      log::info(
          R"(PUBLISH MBP SNAPSHOT exchange="{}", symbol="{}", exchange_sequence={})"sv,
          security.exchange,
          security.symbol,
          exchange_sequence);
      auto market_by_price_update = MarketByPriceUpdate{
          .stream_id = stream_id_,
          .exchange = security.exchange,
          .symbol = security.symbol,
          .bids = bids,
          .asks = asks,
          .update_type = UpdateType::SNAPSHOT,
          .exchange_time_utc = exchange_time_utc,
          .exchange_sequence = sequencer.last_sequence(),
          .sending_time_utc = sending_time_utc,
          .price_decimals = {},
          .quantity_decimals = {},
          .checksum = {},
      };
      Trace event(trace_info, market_by_price_update);
      shared_(event, true, [&](auto &market_by_price) { sequencer.apply(market_by_price, exchange_sequence, false); });
      security.mbp.resubscribe = {};
    };
    auto request_snapshot = [&]([[maybe_unused]] auto retries) {
      log::info(
          R"(REQUEST MBP SNAPSHOT exchange="{}", symbol="{}", exchange_sequence={}, retries={})"sv,
          security.exchange,
          security.symbol,
          exchange_sequence,
          retries);
      security.mbp.resubscribe = exchange_sequence;
    };
    log::info(
        R"(DEBUG UPDATE MBP SNAPSHOT exchange="{}", symbol="{}", exchange_sequence={})"sv,
        security.exchange,
        security.symbol,
        exchange_sequence);
    // note! last_msg_seq_num_processed sometimes point to a completely unrelated security
    auto force = channel_.sequence.first_sequence_number <= security.mbo.resubscribe &&
                 exchange_sequence <= channel_.sequence.last_sequence_number;
    sequencer(bids, asks, exchange_sequence, force, publish_snapshot, request_snapshot);
  } catch (BadState &) {
    log::warn(
        R"(RESUBSCRIBE MBP exchange="{}", symbol="{}", exchange_sequence={}, security_id={})"sv,
        security.exchange,
        security.symbol,
        exchange_sequence,
        security_id);
    // XXX HANS publish stale
    sequencer.clear();  // note! wait for next incremental
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
      .interface = {},
      .authority = {},
      .path = {},
      .proxy = {},
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
