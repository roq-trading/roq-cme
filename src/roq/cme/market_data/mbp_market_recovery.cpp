/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/cme/market_data/mbp_market_recovery.hpp"

#include "roq/utils/safe_cast.hpp"
#include "roq/utils/update.hpp"

#include "roq/utils/debug/hex/message.hpp"

#include "roq/logging.hpp"

using namespace std::literals;

namespace roq {
namespace cme {
namespace market_data {

// === CONSTANTS ===

namespace {
auto const CONNECTION_TYPE = mdp::ConnectionType::MBP_MARKET_RECOVERY;

auto const TRANSPORT = Transport::UDP;
auto const PROTOCOL = Protocol::SBE;
auto const ENCODING = {
    Encoding::SBE,
};

auto const SUPPORTS = Mask{
    SupportType::MARKET_BY_PRICE,
    SupportType::STATISTICS,
};
}  // namespace

// === HELPERS ===

namespace {
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
    log::info<5>("{}"sv, utils::debug::hex::Message{message});
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

MBPMarketRecovery::MBPMarketRecovery(Shared &shared, Channel &channel, uint16_t stream_id, mdp::Config const &config, uint16_t channel_id, Priority priority)
    : priority{priority}, stream_id{stream_id}, name{config.get_name(channel_id, CONNECTION_TYPE, priority)}, shared_{shared}, channel_{channel} {
}

void MBPMarketRecovery::operator()(Event<Start> const &) {
}

void MBPMarketRecovery::operator()(Event<Stop> const &) {
}

void MBPMarketRecovery::operator()(Event<Timer> const &event) {
  if (last_update_time_.count() && (last_update_time_ + shared_.options.multicast_timeout) < event.value.now) {
    log::warn("*** DETECTED TIMEOUT ***"sv);
    last_update_time_ = {};
  }
}

void MBPMarketRecovery::dispatch(std::span<std::byte const> const &payload, TraceInfo const &trace_info) {
  publish_stream_status(trace_info, ConnectionStatus::READY);  // first message will publish
  mdp::Parser::dispatch(*this, payload, trace_info);
}

// mdp::Parser::Handler

void MBPMarketRecovery::operator()(mdp::Frame const &) {
}

void MBPMarketRecovery::operator()(Trace<cme_mdp::AdminHeartbeat12> const &event, mdp::Frame const &frame) {
  auto &trace_info = event.trace_info;
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("admin_heartbeat_12={}, frame={}"sv, value, frame);
  auto external_latency = ExternalLatency{
      .stream_id = stream_id,
      .account = {},
      .latency = trace_info.origin_create_time_utc - frame.sending_time,
  };
  create_trace_and_dispatch(shared_, trace_info, external_latency);
}

void MBPMarketRecovery::operator()(Trace<cme_mdp::ChannelReset4> const &event, mdp::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("channel_reset_4={}, frame={}"sv, value, frame);
}

void MBPMarketRecovery::operator()(Trace<cme_mdp::SecurityStatus30> const &event, mdp::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("security_status_30={}, frame={}"sv, value, frame);
}

void MBPMarketRecovery::operator()(Trace<cme_mdp::MDInstrumentDefinitionFuture54> const &event, mdp::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_instrument_definition_future_54={}, frame={}"sv, value, frame);
}

void MBPMarketRecovery::operator()(Trace<cme_mdp::MDInstrumentDefinitionOption55> const &event, mdp::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_instrument_definition_option_55={}, frame={}"sv, value, frame);
}

void MBPMarketRecovery::operator()(Trace<cme_mdp::MDInstrumentDefinitionSpread56> const &event, mdp::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_instrument_definition_spread_56={}, frame={}"sv, value, frame);
}

void MBPMarketRecovery::operator()(Trace<cme_mdp::MDInstrumentDefinitionFixedIncome57> const &event, mdp::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_instrument_definition_fixed_income_57={}, frame={}"sv, value, frame);
}

void MBPMarketRecovery::operator()(Trace<cme_mdp::MDInstrumentDefinitionRepo58> const &event, mdp::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_instrument_definition_repo_58={}, frame={}"sv, value, frame);
}

void MBPMarketRecovery::operator()(Trace<cme_mdp::MDInstrumentDefinitionFX63> const &event, mdp::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_instrument_definition_fx_63={}, frame={}"sv, value, frame);
}

void MBPMarketRecovery::operator()(Trace<cme_mdp::SnapshotFullRefresh52> const &event, mdp::Frame const &frame) {
  auto &trace_info = event.trace_info;
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("snapshot_full_refresh_52={}, frame={}"sv, value, frame);
  auto security_id = value.securityID();
  shared_.security_definitions.get_security(security_id, [&](auto &security) {
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
      dispatch_market_by_price(trace_info, security_id, security, last_msg_seq_num_processed, transact_time, frame.sending_time, mbp.bids, mbp.asks);
    }
  });
}

void MBPMarketRecovery::operator()(Trace<cme_mdp::SnapshotFullRefreshLongQty69> const &event, mdp::Frame const &frame) {
  auto &trace_info = event.trace_info;
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("snapshot_full_refresh_long_qty_69={}, frame={}"sv, value, frame);
  auto security_id = value.securityID();
  shared_.security_definitions.get_security(security_id, [&](auto &security) {
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
      dispatch_market_by_price(trace_info, security_id, security, last_msg_seq_num_processed, transact_time, frame.sending_time, mbp.bids, mbp.asks);
    }
  });
}

void MBPMarketRecovery::operator()(Trace<cme_mdp::MDIncrementalRefreshBook46> const &event, mdp::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_book_46={}, frame={}"sv, value, frame);
}

void MBPMarketRecovery::operator()(Trace<cme_mdp::MDIncrementalRefreshBookLongQty64> const &event, mdp::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_book_long_qty_64={}, frame={}"sv, value, frame);
}

void MBPMarketRecovery::operator()(Trace<cme_mdp::SnapshotFullRefreshOrderBook53> const &event, mdp::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("snapshot_full_refresh_order_book_53={}, frame={}"sv, value, frame);
}

void MBPMarketRecovery::operator()(Trace<cme_mdp::MDIncrementalRefreshOrderBook47> const &event, mdp::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_order_book_47={}, frame={}"sv, value, frame);
}

void MBPMarketRecovery::operator()(Trace<cme_mdp::MDIncrementalRefreshTradeSummary48> const &event, mdp::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_trade_summary_48={}, frame={}"sv, value, frame);
}

void MBPMarketRecovery::operator()(Trace<cme_mdp::MDIncrementalRefreshTradeSummaryLongQty65> const &event, mdp::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_trade_summary_long_qty_65={}, frame={}"sv, value, frame);
}

void MBPMarketRecovery::operator()(Trace<cme_mdp::MDIncrementalRefreshDailyStatistics49> const &event, mdp::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_daily_statistics_49={}, frame={}"sv, value, frame);
}

void MBPMarketRecovery::operator()(Trace<cme_mdp::MDIncrementalRefreshSessionStatistics51> const &event, mdp::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_session_statistics_51={}, frame={}"sv, value, frame);
}

void MBPMarketRecovery::operator()(Trace<cme_mdp::MDIncrementalRefreshSessionStatisticsLongQty67> const &event, mdp::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_session_statistics_long_qty_67={}, frame={}"sv, value, frame);
}

void MBPMarketRecovery::operator()(Trace<cme_mdp::MDIncrementalRefreshVolume37> const &event, mdp::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_volume_37={}, frame={}"sv, value, frame);
}

void MBPMarketRecovery::operator()(Trace<cme_mdp::MDIncrementalRefreshVolumeLongQty66> const &event, mdp::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_volume_long_qty_66={}, frame={}"sv, value, frame);
}

void MBPMarketRecovery::operator()(Trace<cme_mdp::MDIncrementalRefreshLimitsBanding50> const &event, mdp::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_limits_banding_50={}, frame={}"sv, value, frame);
}

void MBPMarketRecovery::operator()(Trace<cme_mdp::QuoteRequest39> const &event, mdp::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("quote_request_39={}, frame={}"sv, value, frame);
}

// helpers

void MBPMarketRecovery::dispatch_market_by_price(
    auto &trace_info, auto security_id, auto &security, auto exchange_sequence, auto exchange_time_utc, auto sending_time_utc, auto &bids, auto &asks) {
  if (!security.mbp.resubscribe)
    return;
  auto &sequencer = security.mbp.sequencer;
  try {
    auto publish_snapshot = [&](auto &bids, auto &asks, auto exchange_sequence, auto retries, auto delay) {
      log::info(
          R"(PUBLISH MBP SNAPSHOT exchange="{}", symbol="{}", security_id={}, exchange_sequence={}, retries={}, delay={})"sv,
          security.exchange,
          security.symbol,
          security_id,
          exchange_sequence,
          retries,
          std::chrono::duration_cast<std::chrono::milliseconds>(delay));
      auto market_by_price_update = MarketByPriceUpdate{
          .stream_id = stream_id,
          .exchange = security.exchange,
          .symbol = security.symbol,
          .bids = bids,
          .asks = asks,
          .update_type = UpdateType::SNAPSHOT,
          .exchange_time_utc = exchange_time_utc,
          .exchange_sequence = sequencer.last_sequence(),
          .sending_time_utc = sending_time_utc,
          .price_precision = {},
          .quantity_precision = {},
          .checksum = {},
      };
      Trace event(trace_info, market_by_price_update);
      shared_(event, true, [&](auto &market_by_price) { sequencer.apply(market_by_price, exchange_sequence, false); });
      security.mbp.resubscribe = {};
    };
    auto request_snapshot = [&]([[maybe_unused]] auto retries) {
      log::info(
          R"(REQUEST MBP SNAPSHOT exchange="{}", symbol="{}", security_id={}, exchange_sequence={}, retries={})"sv,
          security.exchange,
          security.symbol,
          security_id,
          exchange_sequence,
          retries);
      security.mbp.resubscribe = exchange_sequence;
    };
    log::info(
        R"(DEBUG UPDATE MBP SNAPSHOT exchange="{}", symbol="{}", security_id={}, exchange_sequence={})"sv,
        security.exchange,
        security.symbol,
        security_id,
        exchange_sequence);
    // note! last_msg_seq_num_processed sometimes point to a completely unrelated security
    auto force = channel_.sequence.first_sequence_number <= security.mbp.resubscribe && exchange_sequence <= channel_.sequence.last_sequence_number;
    log::warn(
        "DEBUG force={}, first_sequence={}, resubscribe={}, exchange_sequence={}, last_sequence={}"sv,
        force,
        channel_.sequence.first_sequence_number,
        security.mbp.resubscribe,
        exchange_sequence,
        channel_.sequence.last_sequence_number);
    sequencer(bids, asks, exchange_sequence, force, publish_snapshot, request_snapshot);
  } catch (BadState &) {
    log::warn(
        R"(RESUBSCRIBE MBP exchange="{}", symbol="{}", security_id={}, exchange_sequence={})"sv,
        security.exchange,
        security.symbol,
        security_id,
        exchange_sequence);
    // XXX HANS publish stale
    sequencer.clear();  // note! wait for next incremental
  }
}

void MBPMarketRecovery::publish_stream_status(TraceInfo const &trace_info, ConnectionStatus connection_status) {
  if (!utils::update(connection_status_, connection_status))
    return;
  auto stream_status = StreamStatus{
      .stream_id = stream_id,
      .account = {},
      .supports = SUPPORTS,
      .transport = TRANSPORT,
      .protocol = PROTOCOL,
      .encoding = ENCODING,
      .priority = priority,
      .connection_status = connection_status_,
      .interface = shared_.options.local_interface,
      .authority = {},
      .path = name,
      .proxy = {},
  };
  log::info("stream_status={}"sv, stream_status);
  create_trace_and_dispatch(shared_, trace_info, stream_status);
}

}  // namespace market_data
}  // namespace cme
}  // namespace roq
