/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/cme/udp_instrument_definition.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

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
auto const NAME = "N"sv;

auto const SUPPORTS = Mask{
    SupportType::REFERENCE_DATA,
    SupportType::MARKET_STATUS,
};
}  // namespace

// === HELPERS ===

namespace {
auto create_name(auto stream_id, auto const &channel_id) {
  return fmt::format("{}:{}{}"sv, stream_id, NAME, channel_id);
}

auto create_receiver(auto &handler, auto &context, auto &shared, auto &channel_id, auto priority) {
  log::info(R"(Create channel_id="{}, priority={}")"sv, channel_id, priority);
  auto [multicast_address, port] =
      shared.get_multicast_config(channel_id, multicast::Type::INSTRUMENT_REPLAY, priority);
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
void create_security(auto &shared, auto &value, Callback callback) {
  auto security_id = value.securityID();
  if (shared.has_security(security_id))
    return;
  auto security_exchange = sbe::get_string_view(value.securityExchange(), value.securityExchangeLength());
  auto symbol = sbe::get_string_view(value.symbol(), value.symbolLength());
  auto display_factor = sbe::get_double(value.displayFactor());
  auto security_group = sbe::get_string_view(value.securityGroup(), value.securityGroupLength());
  auto discard = shared.discard_symbol(symbol);
  auto security = Security{
      .exchange = security_exchange,
      .symbol = symbol,
      .display_factor = display_factor,
      .discard = discard,
  };
  shared.create_security(security_group, security_id, std::move(security), [&](auto &security) { callback(security); });
}
}  // namespace

// === IMPLEMENTATION ===

UDPInstrumentDefinition::UDPInstrumentDefinition(
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
          .md_instrument_definition_future = create_metrics(name_, "md_instrument_definition_future"sv),
          .md_instrument_definition_option = create_metrics(name_, "md_instrument_definition_option"sv),
          .md_instrument_definition_spread = create_metrics(name_, "md_instrument_definition_spread"sv),
          .md_instrument_definition_fixed_income = create_metrics(name_, "md_instrument_definition_fixed_income"sv),
          .md_instrument_definition_repo = create_metrics(name_, "md_instrument_definition_repo"sv),
          .md_instrument_definition_fx = create_metrics(name_, "md_instrument_definition_fx"sv),
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
  if (last_update_time_.count() && (last_update_time_ + flags::Multicast::multicast_timeout()) < event.value.now) {
    log::warn("*** DETECTED TIMEOUT ***"sv);
    last_update_time_ = {};
  }
}

void UDPInstrumentDefinition::operator()(io::net::udp::Receiver::Read const &) {
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

void UDPInstrumentDefinition::operator()(io::net::udp::Receiver::Error const &error) {
  log::fatal("Error: what={}"sv, error.what);
}

// sbe::Parser::Handler

void UDPInstrumentDefinition::operator()(sbe::Frame const &) {
}

void UDPInstrumentDefinition::operator()(Trace<cme_mdp::AdminHeartbeat12> const &event, sbe::Frame const &frame) {
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

void UDPInstrumentDefinition::operator()(Trace<cme_mdp::ChannelReset4> const &event, sbe::Frame const &frame) {
  profile_.channel_reset([&]() {
    auto &[trace_info, value] = event;
    log::info<5>("channel_reset_4={}, frame={}"sv, value, frame);
  });
}

void UDPInstrumentDefinition::operator()(Trace<cme_mdp::SecurityStatus30> const &event, sbe::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("security_status_30={}, frame={}"sv, value, frame);
}

void UDPInstrumentDefinition::operator()(
    Trace<cme_mdp::MDInstrumentDefinitionFuture54> const &event, sbe::Frame const &frame) {
  profile_.md_instrument_definition_future([&]() {
    auto &trace_info = event.trace_info;
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("md_instrument_definition_future_54={}, frame={}"sv, value, frame);
    create_security(shared_, value, [&](auto &security) {
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

void UDPInstrumentDefinition::operator()(
    Trace<cme_mdp::MDInstrumentDefinitionOption55> const &event, sbe::Frame const &frame) {
  profile_.md_instrument_definition_option([&]() {
    auto &trace_info = event.trace_info;
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("md_instrument_definition_option_55={}, frame={}"sv, value, frame);
    create_security(shared_, value, [&](auto &security) {
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

void UDPInstrumentDefinition::operator()(
    Trace<cme_mdp::MDInstrumentDefinitionSpread56> const &event, sbe::Frame const &frame) {
  profile_.md_instrument_definition_spread([&]() {
    auto &trace_info = event.trace_info;
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("md_instrument_definition_spread_56={}, frame={}"sv, value, frame);
    create_security(shared_, value, [&](auto &security) {
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

void UDPInstrumentDefinition::operator()(
    Trace<cme_mdp::MDInstrumentDefinitionFixedIncome57> const &event, sbe::Frame const &frame) {
  profile_.md_instrument_definition_fixed_income([&]() {
    auto &trace_info = event.trace_info;
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("md_instrument_definition_fixed_income_57={}, frame={}"sv, value, frame);
    create_security(shared_, value, [&](auto &security) {
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

void UDPInstrumentDefinition::operator()(
    Trace<cme_mdp::MDInstrumentDefinitionRepo58> const &event, sbe::Frame const &frame) {
  profile_.md_instrument_definition_repo([&]() {
    auto &trace_info = event.trace_info;
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("md_instrument_definition_repo_58={}, frame={}"sv, value, frame);
    create_security(shared_, value, [&](auto &security) {
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

void UDPInstrumentDefinition::operator()(
    Trace<cme_mdp::MDInstrumentDefinitionFX63> const &event, sbe::Frame const &frame) {
  profile_.md_instrument_definition_fx([&]() {
    auto &trace_info = event.trace_info;
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    log::info<5>("md_instrument_definition_fx_63={}, frame={}"sv, value, frame);
    create_security(shared_, value, [&](auto &security) {
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

void UDPInstrumentDefinition::operator()(Trace<cme_mdp::SnapshotFullRefresh52> const &event, sbe::Frame const &frame) {
#ifndef NDEBUG
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("snapshot_full_refresh_52={}, frame={}"sv, value, frame);
#endif
}

void UDPInstrumentDefinition::operator()(
    Trace<cme_mdp::SnapshotFullRefreshLongQty69> const &event, sbe::Frame const &frame) {
#ifndef NDEBUG
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("snapshot_full_refresh_long_qty_69={}, frame={}"sv, value, frame);
#endif
}

void UDPInstrumentDefinition::operator()(
    Trace<cme_mdp::MDIncrementalRefreshBook46> const &event, sbe::Frame const &frame) {
#ifndef NDEBUG
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_book_46={}, frame={}"sv, value, frame);
#endif
}

void UDPInstrumentDefinition::operator()(
    Trace<cme_mdp::MDIncrementalRefreshBookLongQty64> const &event, sbe::Frame const &frame) {
#ifndef NDEBUG
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_book_long_qty_64={}, frame={}"sv, value, frame);
#endif
}

void UDPInstrumentDefinition::operator()(
    Trace<cme_mdp::SnapshotFullRefreshOrderBook53> const &event, sbe::Frame const &frame) {
#ifndef NDEBUG
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("snapshot_full_refresh_order_book_53={}, frame={}"sv, value, frame);
#endif
}

void UDPInstrumentDefinition::operator()(
    Trace<cme_mdp::MDIncrementalRefreshOrderBook47> const &event, sbe::Frame const &frame) {
#ifndef NDEBUG
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_order_book_47={}, frame={}"sv, value, frame);
#endif
}

void UDPInstrumentDefinition::operator()(
    Trace<cme_mdp::MDIncrementalRefreshTradeSummary48> const &event, sbe::Frame const &frame) {
#ifndef NDEBUG
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_trade_summary_48={}, frame={}"sv, value, frame);
#endif
}

void UDPInstrumentDefinition::operator()(
    Trace<cme_mdp::MDIncrementalRefreshTradeSummaryLongQty65> const &event, sbe::Frame const &frame) {
#ifndef NDEBUG
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_trade_summary_long_qty_65={}, frame={}"sv, value, frame);
#endif
}

void UDPInstrumentDefinition::operator()(
    Trace<cme_mdp::MDIncrementalRefreshDailyStatistics49> const &event, sbe::Frame const &frame) {
#ifndef NDEBUG
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_daily_statistics_49={}, frame={}"sv, value, frame);
#endif
}

void UDPInstrumentDefinition::operator()(
    Trace<cme_mdp::MDIncrementalRefreshSessionStatistics51> const &event, sbe::Frame const &frame) {
#ifndef NDEBUG
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_session_statistics_51={}, frame={}"sv, value, frame);
#endif
}

void UDPInstrumentDefinition::operator()(
    Trace<cme_mdp::MDIncrementalRefreshSessionStatisticsLongQty67> const &event, sbe::Frame const &frame) {
#ifndef NDEBUG
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_session_statistics_long_qty_67={}, frame={}"sv, value, frame);
#endif
}

void UDPInstrumentDefinition::operator()(
    Trace<cme_mdp::MDIncrementalRefreshVolume37> const &event, sbe::Frame const &frame) {
#ifndef NDEBUG
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_volume_37={}, frame={}"sv, value, frame);
#endif
}

void UDPInstrumentDefinition::operator()(
    Trace<cme_mdp::MDIncrementalRefreshVolumeLongQty66> const &event, sbe::Frame const &frame) {
#ifndef NDEBUG
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_volume_long_qty_66={}, frame={}"sv, value, frame);
#endif
}

void UDPInstrumentDefinition::operator()(
    Trace<cme_mdp::MDIncrementalRefreshLimitsBanding50> const &event, sbe::Frame const &frame) {
#ifndef NDEBUG
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_limits_banding_50={}, frame={}"sv, value, frame);
#endif
}

void UDPInstrumentDefinition::operator()(Trace<cme_mdp::QuoteRequest39> const &event, sbe::Frame const &frame) {
#ifndef NDEBUG
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("quote_request_39={}, frame={}"sv, value, frame);
#endif
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
  };
  log::info("stream_status={}"sv, stream_status);
  create_trace_and_dispatch(handler_, trace_info, stream_status);
}

void UDPInstrumentDefinition::operator()(metrics::Writer &writer) {
  writer  //
      .write(counter_.disconnect, metrics::COUNTER)
      .write(profile_.parse, metrics::PROFILE)
      .write(profile_.admin_heartbeat, metrics::PROFILE)
      .write(profile_.channel_reset, metrics::PROFILE)
      .write(profile_.md_instrument_definition_future, metrics::PROFILE)
      .write(profile_.md_instrument_definition_option, metrics::PROFILE)
      .write(profile_.md_instrument_definition_spread, metrics::PROFILE)
      .write(profile_.md_instrument_definition_fixed_income, metrics::PROFILE)
      .write(profile_.md_instrument_definition_repo, metrics::PROFILE)
      .write(profile_.md_instrument_definition_fx, metrics::PROFILE);
}

}  // namespace cme
}  // namespace roq
