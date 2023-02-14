/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/cme/udp_mbo_market_recovery.hpp"

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

void emplace_back(
    cme_mdp::SnapshotFullRefreshOrderBook53::NoMDEntries const &item, auto &security, auto &bids, auto &asks) {
  auto create_update = [&]() {
    auto price = sbe::get_double(const_cast<cme_mdp::SnapshotFullRefreshOrderBook53::NoMDEntries &>(item).mDEntryPx());
    auto quantity = item.mDDisplayQty();
    auto priority = sbe::get_int(item.mDOrderPriority(), item.mDOrderPriorityNullValue());
    auto order_id = sbe::get_int(item.orderID(), item.orderIDNullValue());
    auto result = MBOUpdate{
        .price = price * security.display_factor,
        .quantity = static_cast<double>(quantity),
        .priority = priority,
        .order_id = {},
        .action = UpdateAction::NEW,
        .reason = {},
    };
    fmt::format_to(std::back_inserter(result.order_id), "{}"sv, order_id);
    return result;
  };
  auto side = sbe::map(item.mDEntryType());
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

// note! don't care about re-ordering or dropped messages (*** maybe we should ??? ***)
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
    if (sbe::Frame::parse(message, [&](auto &frame) { log::info<5>("frame={}"sv, frame); })) {
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
  auto parse = [&](auto &message) {
    if (!sbe::Parser::dispatch(*this, message, trace_info)) {
      log::warn("{}"sv, debug::hex::Message{message});
      log::fatal("Failed to parse message"sv);
    }
  };
  drain(*receiver_, shared_.buffer, stream_id_, parse);
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
    auto &trace_info = event.trace_info;
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("snapshot_full_refresh_order_book_53={}, frame={}"sv, value, frame);
    auto security_id = value.securityID();
    auto iter = channel_.mbo_resubscribe.find(security_id);
    if (iter == std::end(channel_.mbo_resubscribe))
      return;
    shared_.get_security(security_id, [&](auto &security) {
      auto last_msg_seq_num_processed = value.lastMsgSeqNumProcessed();
      auto no_chunks = value.noChunks();
      auto current_chunk = value.currentChunk();
      if (security.update_mbo_snapshot(current_chunk, no_chunks, [&](auto &bids, auto &asks, bool last) {
            value.sbeRewind();  // note!
            value.noMDEntries().forEach([&](auto const &item) { emplace_back(item, security, bids, asks); });
            log::info<5>("DEBUG MBO bids=[{}]"sv, fmt::join(bids, ","sv));
            log::info<5>("DEBUG MBO asks=[{}]"sv, fmt::join(asks, ","sv));
            log::info(
                "DEBUG MBO COLLECT {} / {}, len(bids)={}, len(asks)={}"sv,
                current_chunk,
                no_chunks,
                std::size(bids),
                std::size(asks));
            if (last) {
              auto market_by_order_update = MarketByOrderUpdate{
                  .stream_id = stream_id_,
                  .exchange = security.exchange,
                  .symbol = security.symbol,
                  .bids = bids,
                  .asks = asks,
                  .update_type = UpdateType::SNAPSHOT,
                  .exchange_time_utc = {},
                  .exchange_sequence = last_msg_seq_num_processed,
                  .price_decimals = {},
                  .quantity_decimals = {},
                  .checksum = {},
              };
              create_trace_and_dispatch(handler_, trace_info, market_by_order_update, true);
              log::info("DEBUG MBO READY"sv);
            }
          })) {
        log::info("DEBUG MBO DONE"sv);
        channel_.mbo_resubscribe.erase(iter);
      }
    });
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
