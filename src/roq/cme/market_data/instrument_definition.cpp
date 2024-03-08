/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/cme/market_data/instrument_definition.hpp"

#include "roq/logging.hpp"

#include "roq/utils/update.hpp"

using namespace std::literals;

namespace roq {
namespace cme {
namespace market_data {

// === CONSTANTS ===

namespace {
auto const CONNECTION_TYPE = mdp::ConnectionType::INSTRUMENT_DEFINITION;

auto const TRANSPORT = Transport::UDP;
auto const PROTOCOL = Protocol::SBE;
auto const ENCODING = Mask{
    Encoding::SBE,
};

auto const SUPPORTS = Mask{
    SupportType::REFERENCE_DATA,
    SupportType::MARKET_STATUS,
};
}  // namespace

// === HELPERS ===

namespace {
template <typename Callback>
void create_security(auto &shared, auto &value, Callback callback) {
  auto security_id = value.securityID();
  if (shared.has_security(security_id))
    return;
  auto market_segment_id = value.marketSegmentID();
  auto security_exchange = mdp::get_string_view(value.securityExchange(), value.securityExchangeLength());
  auto symbol = mdp::get_string_view(value.symbol(), value.symbolLength());
  auto display_factor = mdp::get_double(value.displayFactor());
  auto security_group = mdp::get_string_view(value.securityGroup(), value.securityGroupLength());
  auto discard = shared.discard_symbol(symbol);
  auto security = tools::Security{
      .exchange = security_exchange,
      .symbol = symbol,
      .display_factor = display_factor,
      .discard = discard,
  };
  shared.create_security(
      security_group, market_segment_id, security_id, std::move(security), [&](auto &security) { callback(security); });
}
}  // namespace

// === IMPLEMENTATION ===

InstrumentDefinition::InstrumentDefinition(
    Shared &shared, uint16_t stream_id, mdp::Config const &config, uint16_t channel_id, Priority priority)
    : priority{priority}, stream_id{stream_id}, name{config.get_name(channel_id, CONNECTION_TYPE, priority)},
      shared_{shared} {
}

void InstrumentDefinition::operator()(Event<Start> const &event) {
  TraceInfo trace_info{event.message_info};
  publish_stream_status(trace_info, ConnectionStatus::CONNECTING);
}

void InstrumentDefinition::operator()(Event<Stop> const &event) {
  TraceInfo trace_info{event.message_info};
  publish_stream_status(trace_info, ConnectionStatus::DISCONNECTED);
}

void InstrumentDefinition::operator()(Event<Timer> const &event) {
  if (last_update_time_.count() && (last_update_time_ + shared_.options.multicast_timeout) < event.value.now) {
    log::warn("*** DETECTED TIMEOUT ***"sv);
    last_update_time_ = {};
  }
}

void InstrumentDefinition::dispatch(std::span<std::byte const> const &payload, TraceInfo const &trace_info) {
  publish_stream_status(trace_info, ConnectionStatus::READY);  // first message will publish
  mdp::Parser::dispatch(*this, payload, trace_info);
}

// mdp::Parser::Handler

void InstrumentDefinition::operator()(mdp::Frame const &) {
}

void InstrumentDefinition::operator()(Trace<cme_mdp::AdminHeartbeat12> const &event, mdp::Frame const &frame) {
  auto &[trace_info, value] = event;
  log::info<5>("admin_heartbeat_12={}, frame={}"sv, value, frame);
  auto external_latency = ExternalLatency{
      .stream_id = stream_id,
      .account = {},
      .latency = trace_info.origin_create_time_utc - frame.sending_time,
  };
  create_trace_and_dispatch(shared_, trace_info, external_latency);
}

void InstrumentDefinition::operator()(Trace<cme_mdp::ChannelReset4> const &event, mdp::Frame const &frame) {
  auto &[trace_info, value] = event;
  log::info<5>("channel_reset_4={}, frame={}"sv, value, frame);
}

void InstrumentDefinition::operator()(Trace<cme_mdp::SecurityStatus30> const &event, mdp::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("security_status_30={}, frame={}"sv, value, frame);
}

void InstrumentDefinition::operator()(
    Trace<cme_mdp::MDInstrumentDefinitionFuture54> const &event, mdp::Frame const &frame) {
  auto &trace_info = event.trace_info;
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_instrument_definition_future_54={}, frame={}"sv, value, frame);
  create_security(shared_, value, [&](auto &security) {
    auto reference_data = mdp::create_reference_data(value, stream_id, security);
    create_trace_and_dispatch(shared_, trace_info, reference_data, true);
    if (security.discard)
      return;
    auto market_status = mdp::create_market_status(value, stream_id, security);
    create_trace_and_dispatch(shared_, trace_info, market_status, true);
  });
}

void InstrumentDefinition::operator()(
    Trace<cme_mdp::MDInstrumentDefinitionOption55> const &event, mdp::Frame const &frame) {
  auto &trace_info = event.trace_info;
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_instrument_definition_option_55={}, frame={}"sv, value, frame);
  create_security(shared_, value, [&](auto &security) {
    auto reference_data = mdp::create_reference_data(value, stream_id, security);
    create_trace_and_dispatch(shared_, trace_info, reference_data, true);
    if (security.discard)
      return;
    auto market_status = mdp::create_market_status(value, stream_id, security);
    create_trace_and_dispatch(shared_, trace_info, market_status, true);
  });
}

void InstrumentDefinition::operator()(
    Trace<cme_mdp::MDInstrumentDefinitionSpread56> const &event, mdp::Frame const &frame) {
  auto &trace_info = event.trace_info;
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_instrument_definition_spread_56={}, frame={}"sv, value, frame);
  create_security(shared_, value, [&](auto &security) {
    auto reference_data = mdp::create_reference_data(value, stream_id, security);
    create_trace_and_dispatch(shared_, trace_info, reference_data, true);
    if (security.discard)
      return;
    auto market_status = mdp::create_market_status(value, stream_id, security);
    create_trace_and_dispatch(shared_, trace_info, market_status, true);
  });
}

void InstrumentDefinition::operator()(
    Trace<cme_mdp::MDInstrumentDefinitionFixedIncome57> const &event, mdp::Frame const &frame) {
  auto &trace_info = event.trace_info;
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_instrument_definition_fixed_income_57={}, frame={}"sv, value, frame);
  create_security(shared_, value, [&](auto &security) {
    auto reference_data = mdp::create_reference_data(value, stream_id, security);
    create_trace_and_dispatch(shared_, trace_info, reference_data, true);
    if (security.discard)
      return;
    auto market_status = mdp::create_market_status(value, stream_id, security);
    create_trace_and_dispatch(shared_, trace_info, market_status, true);
  });
}

void InstrumentDefinition::operator()(
    Trace<cme_mdp::MDInstrumentDefinitionRepo58> const &event, mdp::Frame const &frame) {
  auto &trace_info = event.trace_info;
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_instrument_definition_repo_58={}, frame={}"sv, value, frame);
  create_security(shared_, value, [&](auto &security) {
    auto reference_data = mdp::create_reference_data(value, stream_id, security);
    create_trace_and_dispatch(shared_, trace_info, reference_data, true);
    if (security.discard)
      return;
    auto market_status = mdp::create_market_status(value, stream_id, security);
    create_trace_and_dispatch(shared_, trace_info, market_status, true);
  });
}

void InstrumentDefinition::operator()(
    Trace<cme_mdp::MDInstrumentDefinitionFX63> const &event, mdp::Frame const &frame) {
  auto &trace_info = event.trace_info;
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_instrument_definition_fx_63={}, frame={}"sv, value, frame);
  create_security(shared_, value, [&](auto &security) {
    auto reference_data = mdp::create_reference_data(value, stream_id, security);
    create_trace_and_dispatch(shared_, trace_info, reference_data, true);
    if (security.discard)
      return;
    auto market_status = mdp::create_market_status(value, stream_id, security);
    create_trace_and_dispatch(shared_, trace_info, market_status, true);
  });
}

void InstrumentDefinition::operator()(Trace<cme_mdp::SnapshotFullRefresh52> const &event, mdp::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("snapshot_full_refresh_52={}, frame={}"sv, value, frame);
}

void InstrumentDefinition::operator()(
    Trace<cme_mdp::SnapshotFullRefreshLongQty69> const &event, mdp::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("snapshot_full_refresh_long_qty_69={}, frame={}"sv, value, frame);
}

void InstrumentDefinition::operator()(
    Trace<cme_mdp::MDIncrementalRefreshBook46> const &event, mdp::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_book_46={}, frame={}"sv, value, frame);
}

void InstrumentDefinition::operator()(
    Trace<cme_mdp::MDIncrementalRefreshBookLongQty64> const &event, mdp::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_book_long_qty_64={}, frame={}"sv, value, frame);
}

void InstrumentDefinition::operator()(
    Trace<cme_mdp::SnapshotFullRefreshOrderBook53> const &event, mdp::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("snapshot_full_refresh_order_book_53={}, frame={}"sv, value, frame);
}

void InstrumentDefinition::operator()(
    Trace<cme_mdp::MDIncrementalRefreshOrderBook47> const &event, mdp::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_order_book_47={}, frame={}"sv, value, frame);
}

void InstrumentDefinition::operator()(
    Trace<cme_mdp::MDIncrementalRefreshTradeSummary48> const &event, mdp::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_trade_summary_48={}, frame={}"sv, value, frame);
}

void InstrumentDefinition::operator()(
    Trace<cme_mdp::MDIncrementalRefreshTradeSummaryLongQty65> const &event, mdp::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_trade_summary_long_qty_65={}, frame={}"sv, value, frame);
}

void InstrumentDefinition::operator()(
    Trace<cme_mdp::MDIncrementalRefreshDailyStatistics49> const &event, mdp::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_daily_statistics_49={}, frame={}"sv, value, frame);
}

void InstrumentDefinition::operator()(
    Trace<cme_mdp::MDIncrementalRefreshSessionStatistics51> const &event, mdp::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_session_statistics_51={}, frame={}"sv, value, frame);
}

void InstrumentDefinition::operator()(
    Trace<cme_mdp::MDIncrementalRefreshSessionStatisticsLongQty67> const &event, mdp::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_session_statistics_long_qty_67={}, frame={}"sv, value, frame);
}

void InstrumentDefinition::operator()(
    Trace<cme_mdp::MDIncrementalRefreshVolume37> const &event, mdp::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_volume_37={}, frame={}"sv, value, frame);
}

void InstrumentDefinition::operator()(
    Trace<cme_mdp::MDIncrementalRefreshVolumeLongQty66> const &event, mdp::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_volume_long_qty_66={}, frame={}"sv, value, frame);
}

void InstrumentDefinition::operator()(
    Trace<cme_mdp::MDIncrementalRefreshLimitsBanding50> const &event, mdp::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_limits_banding_50={}, frame={}"sv, value, frame);
}

void InstrumentDefinition::operator()(Trace<cme_mdp::QuoteRequest39> const &event, mdp::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("quote_request_39={}, frame={}"sv, value, frame);
}

void InstrumentDefinition::publish_stream_status(TraceInfo const &trace_info, ConnectionStatus connection_status) {
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
