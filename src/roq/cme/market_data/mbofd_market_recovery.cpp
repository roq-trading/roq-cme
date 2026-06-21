/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/cme/market_data/mbofd_market_recovery.hpp"

#include "roq/utils/update.hpp"

#include "roq/utils/debug/hex/message.hpp"

#include "roq/logging.hpp"

#include "roq/cme/protocol/mdp/map.hpp"

using namespace std::literals;

namespace roq {
namespace cme {
namespace market_data {

// === CONSTANTS ===

namespace {
auto const CONNECTION_TYPE = protocol::mdp::ConnectionType::MBOFD_MARKET_RECOVERY;

auto const TRANSPORT = Transport::UDP;
auto const PROTOCOL = Protocol::SBE;
auto const ENCODING = {
    Encoding::SBE,
};

auto const SUPPORTS = Mask{
    SupportType::MARKET_BY_ORDER,
};
}  // namespace

// === HELPERS ===

namespace {
void emplace_back(::cme::sbe::mdp::SnapshotFullRefreshOrderBook53::NoMDEntries const &item, auto &security, auto &orders) {
  auto price = map(const_cast<::cme::sbe::mdp::SnapshotFullRefreshOrderBook53::NoMDEntries &>(item).mDEntryPx()).template get<double>();
  auto quantity = item.mDDisplayQty();
  auto priority = protocol::mdp::get_int(item.mDOrderPriority(), item.mDOrderPriorityNullValue());
  auto order_id = protocol::mdp::get_int(item.orderID(), item.orderIDNullValue());
  auto order = MBOUpdate{
      .price = price * security.display_factor,
      .quantity = static_cast<double>(quantity),
      .priority = priority,
      .order_id = {},
      .side = map(item.mDEntryType()),
      .action = UpdateAction::NEW,
      .reason = {},
  };
  fmt::format_to(std::back_inserter(order.order_id), "{}"sv, order_id);
  orders.emplace_back(std::move(order));
}

// note! don't care about re-ordering or dropped messages (*** maybe we should ??? ***)
void drain(auto &receiver, auto &buffer, auto stream_id, auto parse) {
  while (true) {
    // read into buffer
    auto bytes = receiver.recv(buffer);
    log::info<5>("Received {} byte(s) (stream_id={})"sv, bytes, stream_id);
    if (!bytes) {
      return;
    }
    // parse message
    std::span message{std::data(buffer), bytes};
    log::info<5>("{}"sv, utils::debug::hex::Message{message});
    if (protocol::mdp::Frame::parse(message, [&](auto &frame) { log::info<5>("frame={}"sv, frame); })) {
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

MBOFDMarketRecovery::MBOFDMarketRecovery(
    Shared &shared, Channel &channel, uint16_t stream_id, protocol::mdp::Config const &config, uint16_t channel_id, Priority priority)
    : priority{priority}, stream_id{stream_id}, name{config.get_name(channel_id, CONNECTION_TYPE, priority)}, shared_{shared}, channel_{channel} {
}

void MBOFDMarketRecovery::operator()(Event<Start> const &) {
}

void MBOFDMarketRecovery::operator()(Event<Stop> const &) {
}

void MBOFDMarketRecovery::operator()(Event<Timer> const &event) {
  if (last_update_time_.count() && (last_update_time_ + shared_.options.multicast_timeout) < event.value.now) {
    log::warn("*** DETECTED TIMEOUT ***"sv);
    last_update_time_ = {};
  }
}

void MBOFDMarketRecovery::dispatch(std::span<std::byte const> const &payload, TraceInfo const &trace_info) {
  publish_stream_status(trace_info, ConnectionStatus::READY);  // first message will publish
  protocol::mdp::Parser::dispatch(*this, payload, trace_info);
}

// protocol::mdp::Parser::Handler

void MBOFDMarketRecovery::operator()(protocol::mdp::Frame const &) {
}

void MBOFDMarketRecovery::operator()(Trace<::cme::sbe::mdp::AdminHeartbeat12> const &event, protocol::mdp::Frame const &frame) {
  auto &[trace_info, admin_heartbeat_12] = event;
  using value_type = std::remove_cvref_t<decltype(event)>::value_type;
  auto &value = const_cast<value_type &>(admin_heartbeat_12);  // note! not const-safe
  log::info<5>("admin_heartbeat_12={}, frame={}"sv, value, frame);
  auto external_latency = ExternalLatency{
      .stream_id = stream_id,
      .account = {},
      .latency = trace_info.origin_create_time_utc - frame.sending_time,
  };
  create_trace_and_dispatch(shared_, trace_info, external_latency);
}

void MBOFDMarketRecovery::operator()(Trace<::cme::sbe::mdp::ChannelReset4> const &event, protocol::mdp::Frame const &frame) {
  using value_type = std::remove_cvref_t<decltype(event)>::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("channel_reset_4={}, frame={}"sv, value, frame);
}

void MBOFDMarketRecovery::operator()(Trace<::cme::sbe::mdp::SecurityStatus30> const &event, protocol::mdp::Frame const &frame) {
  using value_type = std::remove_cvref_t<decltype(event)>::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("security_status_30={}, frame={}"sv, value, frame);
}

void MBOFDMarketRecovery::operator()(Trace<::cme::sbe::mdp::MDInstrumentDefinitionFuture54> const &event, protocol::mdp::Frame const &frame) {
  using value_type = std::remove_cvref_t<decltype(event)>::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_instrument_definition_future_54={}, frame={}"sv, value, frame);
}

void MBOFDMarketRecovery::operator()(Trace<::cme::sbe::mdp::MDInstrumentDefinitionOption55> const &event, protocol::mdp::Frame const &frame) {
  using value_type = std::remove_cvref_t<decltype(event)>::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_instrument_definition_option_55={}, frame={}"sv, value, frame);
}

void MBOFDMarketRecovery::operator()(Trace<::cme::sbe::mdp::MDInstrumentDefinitionSpread56> const &event, protocol::mdp::Frame const &frame) {
  using value_type = std::remove_cvref_t<decltype(event)>::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_instrument_definition_spread_56={}, frame={}"sv, value, frame);
}

void MBOFDMarketRecovery::operator()(Trace<::cme::sbe::mdp::MDInstrumentDefinitionFixedIncome57> const &event, protocol::mdp::Frame const &frame) {
  using value_type = std::remove_cvref_t<decltype(event)>::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_instrument_definition_fixed_income_57={}, frame={}"sv, value, frame);
}

void MBOFDMarketRecovery::operator()(Trace<::cme::sbe::mdp::MDInstrumentDefinitionRepo58> const &event, protocol::mdp::Frame const &frame) {
  using value_type = std::remove_cvref_t<decltype(event)>::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_instrument_definition_repo_58={}, frame={}"sv, value, frame);
}

void MBOFDMarketRecovery::operator()(Trace<::cme::sbe::mdp::MDInstrumentDefinitionFX63> const &event, protocol::mdp::Frame const &frame) {
  using value_type = std::remove_cvref_t<decltype(event)>::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_instrument_definition_fx_63={}, frame={}"sv, value, frame);
}

void MBOFDMarketRecovery::operator()(Trace<::cme::sbe::mdp::SnapshotFullRefresh52> const &event, protocol::mdp::Frame const &frame) {
  using value_type = std::remove_cvref_t<decltype(event)>::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("snapshot_full_refresh_52={}, frame={}"sv, value, frame);
}

void MBOFDMarketRecovery::operator()(Trace<::cme::sbe::mdp::SnapshotFullRefreshLongQty69> const &event, protocol::mdp::Frame const &frame) {
  using value_type = std::remove_cvref_t<decltype(event)>::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("snapshot_full_refresh_long_qty_69={}, frame={}"sv, value, frame);
}

void MBOFDMarketRecovery::operator()(Trace<::cme::sbe::mdp::MDIncrementalRefreshBook46> const &event, protocol::mdp::Frame const &frame) {
  using value_type = std::remove_cvref_t<decltype(event)>::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_book_46={}, frame={}"sv, value, frame);
}

void MBOFDMarketRecovery::operator()(Trace<::cme::sbe::mdp::MDIncrementalRefreshBookLongQty64> const &event, protocol::mdp::Frame const &frame) {
  using value_type = std::remove_cvref_t<decltype(event)>::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_book_long_qty_64={}, frame={}"sv, value, frame);
}

void MBOFDMarketRecovery::operator()(Trace<::cme::sbe::mdp::SnapshotFullRefreshOrderBook53> const &event, protocol::mdp::Frame const &frame) {
  auto &[trace_info, snapshot_full_refresh_order_book_53] = event;
  using value_type = std::remove_cvref_t<decltype(event)>::value_type;
  auto &value = const_cast<value_type &>(snapshot_full_refresh_order_book_53);  // note! not const-safe
  log::info<5>("snapshot_full_refresh_order_book_53={}, frame={}"sv, value, frame);
  auto security_id = value.securityID();
  shared_.security_definitions.get_security(security_id, [&](auto &security) {
    if (!security.mbo.resubscribe) {
      return;
    }
    auto last_msg_seq_num_processed = value.lastMsgSeqNumProcessed();
    if (last_msg_seq_num_processed < security.mbo.resubscribe) {
      return;
    }
    auto current_chunk = value.currentChunk();
    auto no_chunks = value.noChunks();
    log::info(
        R"(DEBUG SNAPSHOT exchange="{}", symbol="{}", security_id={}, exchange_sequence={}, current_chunk={}, no_chunks={})"sv,
        security.exchange,
        security.symbol,
        security_id,
        last_msg_seq_num_processed,
        current_chunk,
        no_chunks);
    /*
    if (current_chunk == no_chunks)
      log::info(
          R"(DEBUG SNAPSHOT exchange="{}", symbol="{}", security_id={}, exchange_sequence={})"sv,
          security.exchange,
          security.symbol,
          security_id,
          last_msg_seq_num_processed);
    */
    if (security.update_mbo_snapshot(current_chunk, no_chunks, [&](auto &orders, bool last) {
          if (current_chunk == 1 && !std::empty(orders)) {
            log::warn("MBO UNEXPECTED"sv);
          }
          value.sbeRewind();  // note!
          value.noMDEntries().forEach([&](auto const &item) { emplace_back(item, security, orders); });
          log::info<5>("DEBUG MBO orders=[{}]"sv, fmt::join(orders, ","sv));
          if (last) {
            auto &sequencer = security.mbo.sequencer;
            try {
              auto publish_snapshot = [&](auto &orders, auto exchange_sequence, auto retries, auto delay) {
                log::info(
                    R"(PUBLISH SNAPSHOT exchange="{}", symbol="{}", security_id={}, exchange_sequence={}, last_sequence={}, retries={}, delay={})"sv,
                    security.exchange,
                    security.symbol,
                    security_id,
                    last_msg_seq_num_processed,
                    sequencer.last_sequence(),
                    retries,
                    std::chrono::duration_cast<std::chrono::milliseconds>(delay));
                auto market_by_order_update = MarketByOrderUpdate{
                    .stream_id = stream_id,
                    .exchange = security.exchange,
                    .symbol = security.symbol,
                    .orders = orders,
                    .update_type = UpdateType::SNAPSHOT,
                    .exchange_time_utc = {},
                    .exchange_sequence = sequencer.last_sequence(),
                    .sending_time_utc = {},
                    .price_precision = {},
                    .quantity_precision = {},
                    .checksum = {},
                };
                Trace event(trace_info, market_by_order_update);
                shared_(event, true, [&](auto &market_by_order) {
                  sequencer.apply(market_by_order, exchange_sequence, false);
                  // sequencer.apply(market_by_order, last_msg_seq_num_processed, false);
                });
                security.mbo.resubscribe = {};
              };
              auto request_snapshot = [&]([[maybe_unused]] auto retries) {
                log::info(
                    R"(REQUEST SNAPSHOT exchange="{}", symbol="{}", security_id={}, exchange_sequence={}, retries={})"sv,
                    security.exchange,
                    security.symbol,
                    security_id,
                    last_msg_seq_num_processed,
                    retries);
                security.mbo.resubscribe = last_msg_seq_num_processed;
              };
              log::info(
                  R"(DEBUG UPDATE MBO SNAPSHOT exchange="{}", symbol="{}", security_id={}, exchange_sequence={})"sv,
                  security.exchange,
                  security.symbol,
                  security_id,
                  last_msg_seq_num_processed);
              // note! last_msg_seq_num_processed sometimes point to a completely unrelated security
              auto force =
                  channel_.sequence.first_sequence_number <= security.mbo.resubscribe && last_msg_seq_num_processed <= channel_.sequence.last_sequence_number;
              log::warn(
                  "DEBUG force={}, first_sequence={}, resubscribe={}, exchange_sequence={}, last_sequence={}"sv,
                  force,
                  channel_.sequence.first_sequence_number,
                  security.mbo.resubscribe,
                  last_msg_seq_num_processed,
                  channel_.sequence.last_sequence_number);
              sequencer(orders, last_msg_seq_num_processed, force, publish_snapshot, request_snapshot);
            } catch (BadState &) {
              log::warn(
                  R"(RESUBSCRIBE MBO exchange="{}", symbol="{}", security_id={}, exchange_sequence={}, security_id={})"sv,
                  security.exchange,
                  security.symbol,
                  security_id,
                  last_msg_seq_num_processed,
                  security_id);
              // XXX HANS publish stale
              sequencer.clear();  // note! wait for next incremental
            }
          }
        })) {
      log::info("DONE"sv);
    }
  });
}

void MBOFDMarketRecovery::operator()(Trace<::cme::sbe::mdp::MDIncrementalRefreshOrderBook47> const &event, protocol::mdp::Frame const &frame) {
  using value_type = std::remove_cvref_t<decltype(event)>::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_order_book_47={}, frame={}"sv, value, frame);
}

void MBOFDMarketRecovery::operator()(Trace<::cme::sbe::mdp::MDIncrementalRefreshTradeSummary48> const &event, protocol::mdp::Frame const &frame) {
  using value_type = std::remove_cvref_t<decltype(event)>::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_trade_summary_48={}, frame={}"sv, value, frame);
}

void MBOFDMarketRecovery::operator()(Trace<::cme::sbe::mdp::MDIncrementalRefreshTradeSummaryLongQty65> const &event, protocol::mdp::Frame const &frame) {
  using value_type = std::remove_cvref_t<decltype(event)>::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_trade_summary_long_qty_65={}, frame={}"sv, value, frame);
}

void MBOFDMarketRecovery::operator()(Trace<::cme::sbe::mdp::MDIncrementalRefreshDailyStatistics49> const &event, protocol::mdp::Frame const &frame) {
  using value_type = std::remove_cvref_t<decltype(event)>::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_daily_statistics_49={}, frame={}"sv, value, frame);
}

void MBOFDMarketRecovery::operator()(Trace<::cme::sbe::mdp::MDIncrementalRefreshSessionStatistics51> const &event, protocol::mdp::Frame const &frame) {
  using value_type = std::remove_cvref_t<decltype(event)>::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_session_statistics_51={}, frame={}"sv, value, frame);
}

void MBOFDMarketRecovery::operator()(Trace<::cme::sbe::mdp::MDIncrementalRefreshSessionStatisticsLongQty67> const &event, protocol::mdp::Frame const &frame) {
  using value_type = std::remove_cvref_t<decltype(event)>::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_session_statistics_long_qty_67={}, frame={}"sv, value, frame);
}

void MBOFDMarketRecovery::operator()(Trace<::cme::sbe::mdp::MDIncrementalRefreshVolume37> const &event, protocol::mdp::Frame const &frame) {
  using value_type = std::remove_cvref_t<decltype(event)>::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_volume_37={}, frame={}"sv, value, frame);
}

void MBOFDMarketRecovery::operator()(Trace<::cme::sbe::mdp::MDIncrementalRefreshVolumeLongQty66> const &event, protocol::mdp::Frame const &frame) {
  using value_type = std::remove_cvref_t<decltype(event)>::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_volume_long_qty_66={}, frame={}"sv, value, frame);
}

void MBOFDMarketRecovery::operator()(Trace<::cme::sbe::mdp::MDIncrementalRefreshLimitsBanding50> const &event, protocol::mdp::Frame const &frame) {
  using value_type = std::remove_cvref_t<decltype(event)>::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_limits_banding_50={}, frame={}"sv, value, frame);
}

void MBOFDMarketRecovery::operator()(Trace<::cme::sbe::mdp::QuoteRequest39> const &event, protocol::mdp::Frame const &frame) {
  using value_type = std::remove_cvref_t<decltype(event)>::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("quote_request_39={}, frame={}"sv, value, frame);
}

void MBOFDMarketRecovery::publish_stream_status(TraceInfo const &trace_info, ConnectionStatus connection_status) {
  if (!utils::update(connection_status_, connection_status)) {
    return;
  }
  auto stream_status = StreamStatus{
      .stream_id = stream_id,
      .account = {},
      .supports = SUPPORTS,
      .transport = TRANSPORT,
      .protocol = PROTOCOL,
      .encoding = ENCODING,
      .priority = priority,
      .connection_status = connection_status_,
      .reason = {},
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
