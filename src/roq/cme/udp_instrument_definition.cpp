/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/cme/udp_instrument_definition.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "roq/utils/safe_cast.hpp"
#include "roq/utils/update.hpp"

#include "roq/utils/debug/hex/message.hpp"

#include "roq/core/metrics/factory.hpp"

#include "roq/io/network_address.hpp"

#include "roq/cme/mdp/utils.hpp"

using namespace std::literals;

namespace roq {
namespace cme {

// === CONSTANTS ===

namespace {
auto const NAME = "N"sv;

auto const SUPPORTS = Mask{
    SupportType::REFERENCE_DATA,
    SupportType::MARKET_STATUS,
};
}  // namespace

// === HELPERS ===

namespace {
auto create_name(auto stream_id, auto &channel_name) {
  return fmt::format("{}:{}"sv, stream_id, channel_name);
}

auto create_receiver(auto &handler, auto &context, auto &shared, auto &channel_id, auto priority) {
  log::info(R"(Create channel_id="{}, priority={}")"sv, channel_id, priority);
  auto [multicast_address, port] =
      shared.get_multicast_config(channel_id, mdp::ConnectionType::INSTRUMENT_REPLAY, priority);
  log::info("Create multicast receiver port={}"sv, port);
  auto network_address = io::NetworkAddress{port};
  auto socket_options = Mask{
      io::SocketOption::REUSE_ADDRESS,
  };
  auto receiver = context.create_udp_receiver(handler, network_address, socket_options);
  log::info(R"(Local interface is "{}")"sv, shared.settings.multicast.local_interface);
  auto local_interface = io::NetworkAddress::create_blocking(shared.settings.multicast.local_interface);
  log::info(R"(Add membership "{}")"sv, multicast_address);
  auto multicast_address_2 = io::NetworkAddress::create_blocking(multicast_address);
  (*receiver).add_membership(multicast_address_2, local_interface);
  return receiver;
}

struct create_metrics final : public core::metrics::Factory {
  explicit create_metrics(auto &settings, auto const &group, auto const &function)
      : core::metrics::Factory(settings.app.name, group, function) {}
};

// following are used from several places

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

UDPInstrumentDefinition::UDPInstrumentDefinition(
    Handler &handler, io::Context &context, uint16_t stream_id, Shared &shared, Channel &channel)
    : handler_{handler}, channel_name_{channel.get_channel_name(NAME)}, stream_id_{stream_id},
      name_{create_name(stream_id_, channel_name_)},
      receiver_{create_receiver(*this, context, shared, channel.channel_id, Priority::PRIMARY)},
      counter_{
          .disconnect = create_metrics(shared.settings, name_, "disconnect"sv),
      },
      profile_{
          .parse = create_metrics(shared.settings, name_, "parse"sv),
          .admin_heartbeat = create_metrics(shared.settings, name_, "admin_heartbeat"sv),
          .channel_reset = create_metrics(shared.settings, name_, "channel_reset"sv),
          .md_instrument_definition_future =
              create_metrics(shared.settings, name_, "md_instrument_definition_future"sv),
          .md_instrument_definition_option =
              create_metrics(shared.settings, name_, "md_instrument_definition_option"sv),
          .md_instrument_definition_spread =
              create_metrics(shared.settings, name_, "md_instrument_definition_spread"sv),
          .md_instrument_definition_fixed_income =
              create_metrics(shared.settings, name_, "md_instrument_definition_fixed_income"sv),
          .md_instrument_definition_repo = create_metrics(shared.settings, name_, "md_instrument_definition_repo"sv),
          .md_instrument_definition_fx = create_metrics(shared.settings, name_, "md_instrument_definition_fx"sv),
      },
      shared_{shared}, channel_{channel} {
}

void UDPInstrumentDefinition::operator()(Event<Start> const &) {
  TraceInfo trace_info;
  publish_stream_status(trace_info, ConnectionStatus::CONNECTING);
  last_update_time_ = trace_info.source_receive_time;
}

void UDPInstrumentDefinition::operator()(Event<Stop> const &) {
}

void UDPInstrumentDefinition::operator()(Event<Timer> const &event) {
  if (last_update_time_.count() && (last_update_time_ + shared_.settings.multicast.timeout) < event.value.now) {
    log::warn("*** DETECTED TIMEOUT ***"sv);
    last_update_time_ = {};
  }
}

void UDPInstrumentDefinition::operator()(io::net::udp::Receiver::Read const &) {
  TraceInfo trace_info;
  last_update_time_ = trace_info.source_receive_time;
  publish_stream_status(trace_info, ConnectionStatus::READY);  // first message will publish
  auto parse = [&](auto &message) {
    if (!mdp::Parser::dispatch(*this, message, trace_info)) {
      log::warn("{}"sv, utils::debug::hex::Message{message});
      log::fatal("Failed to parse message"sv);
    }
  };
  drain(*receiver_, shared_.buffer, stream_id_, parse);
}

void UDPInstrumentDefinition::operator()(io::net::udp::Receiver::Error const &error) {
  log::fatal("Error: what={}"sv, error.what);
}

// mdp::Parser::Handler

void UDPInstrumentDefinition::operator()(mdp::Frame const &) {
}

void UDPInstrumentDefinition::operator()(Trace<cme_mdp::AdminHeartbeat12> const &event, mdp::Frame const &frame) {
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

void UDPInstrumentDefinition::operator()(Trace<cme_mdp::ChannelReset4> const &event, mdp::Frame const &frame) {
  profile_.channel_reset([&]() {
    auto &[trace_info, value] = event;
    log::info<5>("channel_reset_4={}, frame={}"sv, value, frame);
  });
}

void UDPInstrumentDefinition::operator()(Trace<cme_mdp::SecurityStatus30> const &event, mdp::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("security_status_30={}, frame={}"sv, value, frame);
}

void UDPInstrumentDefinition::operator()(
    Trace<cme_mdp::MDInstrumentDefinitionFuture54> const &event, mdp::Frame const &frame) {
  profile_.md_instrument_definition_future([&]() {
    auto &trace_info = event.trace_info;
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("md_instrument_definition_future_54={}, frame={}"sv, value, frame);
    create_security(shared_, value, [&](auto &security) {
      auto reference_data = mdp::create_reference_data(value, stream_id_, security);
      create_trace_and_dispatch(handler_, trace_info, reference_data, true);
      if (security.discard)
        return;
      auto market_status = mdp::create_market_status(value, stream_id_, security);
      create_trace_and_dispatch(handler_, trace_info, market_status, true);
    });
  });
}

void UDPInstrumentDefinition::operator()(
    Trace<cme_mdp::MDInstrumentDefinitionOption55> const &event, mdp::Frame const &frame) {
  profile_.md_instrument_definition_option([&]() {
    auto &trace_info = event.trace_info;
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("md_instrument_definition_option_55={}, frame={}"sv, value, frame);
    create_security(shared_, value, [&](auto &security) {
      auto reference_data = mdp::create_reference_data(value, stream_id_, security);
      create_trace_and_dispatch(handler_, trace_info, reference_data, true);
      if (security.discard)
        return;
      auto market_status = mdp::create_market_status(value, stream_id_, security);
      create_trace_and_dispatch(handler_, trace_info, market_status, true);
    });
  });
}

void UDPInstrumentDefinition::operator()(
    Trace<cme_mdp::MDInstrumentDefinitionSpread56> const &event, mdp::Frame const &frame) {
  profile_.md_instrument_definition_spread([&]() {
    auto &trace_info = event.trace_info;
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("md_instrument_definition_spread_56={}, frame={}"sv, value, frame);
    create_security(shared_, value, [&](auto &security) {
      auto reference_data = mdp::create_reference_data(value, stream_id_, security);
      create_trace_and_dispatch(handler_, trace_info, reference_data, true);
      if (security.discard)
        return;
      auto market_status = mdp::create_market_status(value, stream_id_, security);
      create_trace_and_dispatch(handler_, trace_info, market_status, true);
    });
  });
}

void UDPInstrumentDefinition::operator()(
    Trace<cme_mdp::MDInstrumentDefinitionFixedIncome57> const &event, mdp::Frame const &frame) {
  profile_.md_instrument_definition_fixed_income([&]() {
    auto &trace_info = event.trace_info;
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("md_instrument_definition_fixed_income_57={}, frame={}"sv, value, frame);
    create_security(shared_, value, [&](auto &security) {
      auto reference_data = mdp::create_reference_data(value, stream_id_, security);
      create_trace_and_dispatch(handler_, trace_info, reference_data, true);
      if (security.discard)
        return;
      auto market_status = mdp::create_market_status(value, stream_id_, security);
      create_trace_and_dispatch(handler_, trace_info, market_status, true);
    });
  });
}

void UDPInstrumentDefinition::operator()(
    Trace<cme_mdp::MDInstrumentDefinitionRepo58> const &event, mdp::Frame const &frame) {
  profile_.md_instrument_definition_repo([&]() {
    auto &trace_info = event.trace_info;
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("md_instrument_definition_repo_58={}, frame={}"sv, value, frame);
    create_security(shared_, value, [&](auto &security) {
      auto reference_data = mdp::create_reference_data(value, stream_id_, security);
      create_trace_and_dispatch(handler_, trace_info, reference_data, true);
      if (security.discard)
        return;
      auto market_status = mdp::create_market_status(value, stream_id_, security);
      create_trace_and_dispatch(handler_, trace_info, market_status, true);
    });
  });
}

void UDPInstrumentDefinition::operator()(
    Trace<cme_mdp::MDInstrumentDefinitionFX63> const &event, mdp::Frame const &frame) {
  profile_.md_instrument_definition_fx([&]() {
    auto &trace_info = event.trace_info;
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("md_instrument_definition_fx_63={}, frame={}"sv, value, frame);
    create_security(shared_, value, [&](auto &security) {
      auto reference_data = mdp::create_reference_data(value, stream_id_, security);
      create_trace_and_dispatch(handler_, trace_info, reference_data, true);
      if (security.discard)
        return;
      auto market_status = mdp::create_market_status(value, stream_id_, security);
      create_trace_and_dispatch(handler_, trace_info, market_status, true);
    });
  });
}

void UDPInstrumentDefinition::operator()(Trace<cme_mdp::SnapshotFullRefresh52> const &event, mdp::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("snapshot_full_refresh_52={}, frame={}"sv, value, frame);
}

void UDPInstrumentDefinition::operator()(
    Trace<cme_mdp::SnapshotFullRefreshLongQty69> const &event, mdp::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("snapshot_full_refresh_long_qty_69={}, frame={}"sv, value, frame);
}

void UDPInstrumentDefinition::operator()(
    Trace<cme_mdp::MDIncrementalRefreshBook46> const &event, mdp::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_book_46={}, frame={}"sv, value, frame);
}

void UDPInstrumentDefinition::operator()(
    Trace<cme_mdp::MDIncrementalRefreshBookLongQty64> const &event, mdp::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_book_long_qty_64={}, frame={}"sv, value, frame);
}

void UDPInstrumentDefinition::operator()(
    Trace<cme_mdp::SnapshotFullRefreshOrderBook53> const &event, mdp::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("snapshot_full_refresh_order_book_53={}, frame={}"sv, value, frame);
}

void UDPInstrumentDefinition::operator()(
    Trace<cme_mdp::MDIncrementalRefreshOrderBook47> const &event, mdp::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_order_book_47={}, frame={}"sv, value, frame);
}

void UDPInstrumentDefinition::operator()(
    Trace<cme_mdp::MDIncrementalRefreshTradeSummary48> const &event, mdp::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_trade_summary_48={}, frame={}"sv, value, frame);
}

void UDPInstrumentDefinition::operator()(
    Trace<cme_mdp::MDIncrementalRefreshTradeSummaryLongQty65> const &event, mdp::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_trade_summary_long_qty_65={}, frame={}"sv, value, frame);
}

void UDPInstrumentDefinition::operator()(
    Trace<cme_mdp::MDIncrementalRefreshDailyStatistics49> const &event, mdp::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_daily_statistics_49={}, frame={}"sv, value, frame);
}

void UDPInstrumentDefinition::operator()(
    Trace<cme_mdp::MDIncrementalRefreshSessionStatistics51> const &event, mdp::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_session_statistics_51={}, frame={}"sv, value, frame);
}

void UDPInstrumentDefinition::operator()(
    Trace<cme_mdp::MDIncrementalRefreshSessionStatisticsLongQty67> const &event, mdp::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_session_statistics_long_qty_67={}, frame={}"sv, value, frame);
}

void UDPInstrumentDefinition::operator()(
    Trace<cme_mdp::MDIncrementalRefreshVolume37> const &event, mdp::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_volume_37={}, frame={}"sv, value, frame);
}

void UDPInstrumentDefinition::operator()(
    Trace<cme_mdp::MDIncrementalRefreshVolumeLongQty66> const &event, mdp::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_volume_long_qty_66={}, frame={}"sv, value, frame);
}

void UDPInstrumentDefinition::operator()(
    Trace<cme_mdp::MDIncrementalRefreshLimitsBanding50> const &event, mdp::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_limits_banding_50={}, frame={}"sv, value, frame);
}

void UDPInstrumentDefinition::operator()(Trace<cme_mdp::QuoteRequest39> const &event, mdp::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("quote_request_39={}, frame={}"sv, value, frame);
}

void UDPInstrumentDefinition::publish_stream_status(TraceInfo const &trace_info, ConnectionStatus connection_status) {
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
      .interface = shared_.settings.multicast.local_interface,
      .authority = {},
      .path = channel_name_,
      .proxy = {},
  };
  log::info("stream_status={}"sv, stream_status);
  create_trace_and_dispatch(handler_, trace_info, stream_status);
}

void UDPInstrumentDefinition::operator()(metrics::Writer &writer) {
  writer  //
      .write(counter_.disconnect, metrics::Type::COUNTER)
      .write(profile_.parse, metrics::Type::PROFILE)
      .write(profile_.admin_heartbeat, metrics::Type::PROFILE)
      .write(profile_.channel_reset, metrics::Type::PROFILE)
      .write(profile_.md_instrument_definition_future, metrics::Type::PROFILE)
      .write(profile_.md_instrument_definition_option, metrics::Type::PROFILE)
      .write(profile_.md_instrument_definition_spread, metrics::Type::PROFILE)
      .write(profile_.md_instrument_definition_fixed_income, metrics::Type::PROFILE)
      .write(profile_.md_instrument_definition_repo, metrics::Type::PROFILE)
      .write(profile_.md_instrument_definition_fx, metrics::Type::PROFILE);
}

}  // namespace cme
}  // namespace roq
