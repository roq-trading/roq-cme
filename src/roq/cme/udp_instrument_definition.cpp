/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/cme/udp_instrument_definition.hpp"

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

namespace {
auto const NAME = "udp_na"sv;

Mask<SupportType> const SUPPORTS{
    SupportType::REFERENCE_DATA,
    SupportType::MARKET_STATUS,
};

struct create_metrics final : public core::metrics::Factory {
  explicit create_metrics(std::string_view const &group, std::string_view const &function)
      : core::metrics::Factory(server::Flags::name(), group, function) {}
};

auto create_receiver(auto &handler, auto &context, auto &shared) {
  auto [multicast_address, port] = shared.get_multicast_config(multicast::Type::INCREMENTAL, Priority::PRIMARY);
  log::info<0>("Create multicast socket port={}"sv, port);
  auto receiver = context.create_udp_receiver(handler, io::NetworkAddress{port});
  log::info<0>(R"(Local interface is "{}")"sv, flags::Multicast::multicast_local_interface());
  std::string local_interface{flags::Multicast::multicast_local_interface()};
  struct in_addr local = {};
  local.s_addr = inet_addr(local_interface.c_str());
  log::info<0>(R"(Add membership "{}")"sv, multicast_address);
  struct in_addr multicast = {};
  multicast.s_addr = inet_addr(multicast_address.c_str());
  (*receiver).add_membership(io::NetworkAddress{0, multicast}, io::NetworkAddress{0, local});
  return receiver;
}

bool test_sequence(auto &cache, auto instrument_id, auto sequence_number) {
  auto result = false;
  const constexpr uint32_t midpoint = 1 << 31;
  auto iter = cache.find(instrument_id);
  if (iter != cache.end()) {
    auto previous = (*iter).second;
    if (previous < sequence_number) {
      result = true;
    } else if (sequence_number < midpoint && midpoint < previous) {
      result = true;  // wraparound
    } else {
      // out of sequence
    }
  } else {
    iter = cache.emplace(instrument_id, sequence_number).first;
    result = true;
  }
  if (result)
    (*iter).second = sequence_number;
  return result;
}

template <typename Callback>
void create_security(auto &shared, auto &value, Callback callback) {
  auto security_id = value.securityID();
  auto iter = shared.securities.find(security_id);
  if (iter == std::end(shared.securities)) {
    auto symbol = sbe::get_string_view(value.symbol(), value.symbolLength());
    Shared::Security security{
        .exchange = sbe::get_string_view(value.securityExchange(), value.securityExchangeLength()),
        .symbol = symbol,
        .display_factor = sbe::get_double(value.displayFactor()),
        .discard = shared.discard_symbol(symbol),
    };
    iter = shared.securities.try_emplace(security_id, std::move(security)).first;
    callback((*iter).second);
  }
}
}  // namespace

UDPInstrumentDefinition::UDPInstrumentDefinition(
    Handler &handler, io::Context &context, uint16_t stream_id, Shared &shared)
    : handler_(handler), stream_id_(stream_id), name_(fmt::format("{}:{}"sv, stream_id_, NAME)),
      receiver_(create_receiver(*this, context, shared)),
      counter_{
          .disconnect = create_metrics(name_, "disconnect"sv),
      },
      profile_{
          .parse = create_metrics(name_, "parse"sv),
      },
      shared_(shared) {
}

void UDPInstrumentDefinition::operator()(Event<Start> const &) {
  auto trace_info = server::create_trace_info();
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
  auto trace_info = server::create_trace_info();
  last_update_time_ = trace_info.source_receive_time;
  publish_stream_status(trace_info, ConnectionStatus::READY);  // first message will publish
  while (receive_buffer_.append(*receiver_)) {
    auto message = receive_buffer_.data();
    log::info<5>("received {} byte(s)"sv, std::size(message));
    log::info<5>("{}"sv, debug::hex::Message{message});
    if (!sbe::Parser::dispatch(*this, message, trace_info)) {
      log::warn("{}"sv, debug::hex::Message{message});
      log::fatal("Failed to parse message"sv);
    }
    receive_buffer_.clear();
  }
}

void UDPInstrumentDefinition::operator()(io::net::udp::Receiver::Error const &error) {
  log::fatal("Error: what={}"sv, error.what);
}

// sbe::Parser::Handler

void UDPInstrumentDefinition::operator()(Trace<cme_mdp::ChannelReset4> const &event, sbe::Frame const &frame) {
  auto &[trace_info, value] = event;
  log::info<3>("channel_reset_4={}, frame={}"sv, const_cast<decltype(value) &>(value), frame);
  log::info<3>("HERE"sv);
}

void UDPInstrumentDefinition::operator()(Trace<cme_mdp::AdminHeartbeat12> const &event, sbe::Frame const &frame) {
  auto &[trace_info, value] = event;
  log::info<5>("admin_heartbeat_12={}, frame={}"sv, const_cast<decltype(value) &>(value), frame);
}

// - MDInstrumentDefinition

void UDPInstrumentDefinition::operator()(
    Trace<cme_mdp::MDInstrumentDefinitionFuture54> const &event, sbe::Frame const &frame) {
  auto &trace_info = event.trace_info;
  auto &value = event.value;
  log::info<3>("md_instrument_definition_future_54={}, frame={}"sv, const_cast<decltype(value) &>(value), frame);
  create_security(shared_, value, [&](auto &security) {
    auto min_price_increment = sbe::get_double(value.minPriceIncrement());
    auto contract_multiplier = sbe::get_int(value.contractMultiplier(), value.contractMultiplierNullValue());
    double multiplier = contract_multiplier == 0 ? NaN : utils::safe_cast<double>(contract_multiplier);
    ReferenceData const reference_data{
        .stream_id = stream_id_,
        .exchange = security.exchange,
        .symbol = security.symbol,
        .description = {},
        .security_type = SecurityType::FUTURES,
        .base_currency = {},
        .quote_currency = sbe::get_string_view(value.currency(), value.currencyLength()),
        .margin_currency = {},
        .commission_currency = {},
        .tick_size = min_price_increment * security.display_factor,
        .multiplier = multiplier,
        .min_trade_vol = utils::safe_cast(value.minTradeVol()),
        .max_trade_vol = utils::safe_cast(value.maxTradeVol()),
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
    MarketStatus const market_status{
        .stream_id = stream_id_,
        .exchange = security.exchange,
        .symbol = security.symbol,
        .trading_status = sbe::map_security_trading_status(value.mDSecurityTradingStatus()),
    };
    create_trace_and_dispatch(handler_, trace_info, market_status, true);
  });
}

void UDPInstrumentDefinition::operator()(
    Trace<cme_mdp::MDInstrumentDefinitionOption55> const &event, sbe::Frame const &frame) {
  auto &trace_info = event.trace_info;
  auto &value = event.value;
  log::info<3>("md_instrument_definition_option_55={}, frame={}"sv, const_cast<decltype(value) &>(value), frame);
  create_security(shared_, value, [&](auto &security) {
    auto min_price_increment = sbe::get_double(value.minPriceIncrement());
    ReferenceData const reference_data{
        .stream_id = stream_id_,
        .exchange = security.exchange,
        .symbol = security.symbol,
        .description = {},
        .security_type = SecurityType::OPTION,
        .base_currency = {},
        .quote_currency = sbe::get_string_view(value.currency(), value.currencyLength()),
        .margin_currency = {},
        .commission_currency = {},
        .tick_size = min_price_increment * security.display_factor,
        .multiplier = NaN,
        .min_notional = NaN,
        .min_trade_vol = utils::safe_cast(value.minTradeVol()),
        .max_trade_vol = utils::safe_cast(value.maxTradeVol()),
        .trade_vol_step_size = NaN,
        .option_type = {},
        .strike_currency = sbe::get_string_view(value.strikeCurrency(), value.strikeCurrencyLength()),
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
    MarketStatus const market_status{
        .stream_id = stream_id_,
        .exchange = security.exchange,
        .symbol = security.symbol,
        .trading_status = sbe::map_security_trading_status(value.mDSecurityTradingStatus()),
    };
    create_trace_and_dispatch(handler_, trace_info, market_status, true);
  });
}

void UDPInstrumentDefinition::operator()(
    Trace<cme_mdp::MDInstrumentDefinitionSpread56> const &event, sbe::Frame const &frame) {
  auto &trace_info = event.trace_info;
  auto &value = event.value;
  log::info<3>("md_instrument_definition_spread_56={}, frame={}"sv, const_cast<decltype(value) &>(value), frame);
  create_security(shared_, value, [&](auto &security) {
    ReferenceData const reference_data{
        .stream_id = stream_id_,
        .exchange = security.exchange,
        .symbol = security.symbol,
        .description = {},
        .security_type = SecurityType::FUTURES,
        .base_currency = {},
        .quote_currency = sbe::get_string_view(value.currency(), value.currencyLength()),
        .margin_currency = {},
        .commission_currency = {},
        .tick_size = sbe::get_double(value.minPriceIncrement()),
        .multiplier = NaN,
        .min_trade_vol = utils::safe_cast(value.minTradeVol()),
        .max_trade_vol = utils::safe_cast(value.maxTradeVol()),
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
  });
}

void UDPInstrumentDefinition::operator()(
    Trace<cme_mdp::MDInstrumentDefinitionFixedIncome57> const &event, sbe::Frame const &frame) {
  auto &trace_info = event.trace_info;
  auto &value = event.value;
  log::info<3>("md_instrument_definition_fixed_income_57={}, frame={}"sv, const_cast<decltype(value) &>(value), frame);
  create_security(shared_, value, [&](auto &security) {
    ReferenceData const reference_data{
        .stream_id = stream_id_,
        .exchange = security.exchange,
        .symbol = security.symbol,
        .description = {},
        .security_type = SecurityType::FUTURES,
        .base_currency = {},
        .quote_currency = sbe::get_string_view(value.currency(), value.currencyLength()),
        .margin_currency = {},
        .commission_currency = {},
        .tick_size = sbe::get_double(value.minPriceIncrement()),
        .multiplier = NaN,
        .min_trade_vol = utils::safe_cast(value.minTradeVol()),
        .max_trade_vol = utils::safe_cast(value.maxTradeVol()),
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
  });
}

void UDPInstrumentDefinition::operator()(
    Trace<cme_mdp::MDInstrumentDefinitionRepo58> const &event, sbe::Frame const &frame) {
  auto &trace_info = event.trace_info;
  auto &value = event.value;
  log::info<3>("md_instrument_definition_repo_58={}, frame={}"sv, const_cast<decltype(value) &>(value), frame);
  create_security(shared_, value, [&](auto &security) {
    ReferenceData const reference_data{
        .stream_id = stream_id_,
        .exchange = security.exchange,
        .symbol = security.symbol,
        .description = {},
        .security_type = SecurityType::FUTURES,
        .base_currency = {},
        .quote_currency = sbe::get_string_view(value.currency(), value.currencyLength()),
        .margin_currency = {},
        .commission_currency = {},
        .tick_size = sbe::get_double(value.minPriceIncrement()),
        .multiplier = NaN,
        .min_trade_vol = utils::safe_cast(value.minTradeVol()),
        .max_trade_vol = utils::safe_cast(value.maxTradeVol()),
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
  });
}

void UDPInstrumentDefinition::operator()(
    Trace<cme_mdp::MDInstrumentDefinitionFX63> const &event, sbe::Frame const &frame) {
  auto &trace_info = event.trace_info;
  auto &value = event.value;
  log::info<3>("md_instrument_definition_fx_63={}, frame={}"sv, const_cast<decltype(value) &>(value), frame);
  create_security(shared_, value, [&](auto &security) {
    ReferenceData const reference_data{
        .stream_id = stream_id_,
        .exchange = security.exchange,
        .symbol = security.symbol,
        .description = {},
        .security_type = SecurityType::FUTURES,
        .base_currency = {},
        .quote_currency = sbe::get_string_view(value.currency(), value.currencyLength()),
        .margin_currency = {},
        .commission_currency = {},
        .tick_size = sbe::get_double(value.minPriceIncrement()),
        .multiplier = NaN,
        .min_trade_vol = utils::safe_cast(value.minTradeVol()),
        .max_trade_vol = utils::safe_cast(value.maxTradeVol()),
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
  });
}
// - MbP

void UDPInstrumentDefinition::operator()(Trace<cme_mdp::SnapshotFullRefresh52> const &, sbe::Frame const &) {
  log::warn<5>("Unexpected: template_id=52"sv);
}

void UDPInstrumentDefinition::operator()(Trace<cme_mdp::SnapshotFullRefreshLongQty69> const &, sbe::Frame const &) {
  log::warn<5>("Unexpected: template_id=69"sv);
}

// - MbO

void UDPInstrumentDefinition::operator()(Trace<cme_mdp::SnapshotFullRefreshOrderBook53> const &, sbe::Frame const &) {
  log::warn<5>("Unexpected: template_id=53"sv);
}

// - MDIncrementalRefresh

void UDPInstrumentDefinition::operator()(Trace<cme_mdp::MDIncrementalRefreshVolume37> const &, sbe::Frame const &) {
  log::warn<5>("Unexpected: template_id=37"sv);
}

void UDPInstrumentDefinition::operator()(Trace<cme_mdp::MDIncrementalRefreshBook46> const &, sbe::Frame const &) {
  log::warn<5>("Unexpected: template_id=46"sv);
}

void UDPInstrumentDefinition::operator()(Trace<cme_mdp::MDIncrementalRefreshOrderBook47> const &, sbe::Frame const &) {
  log::warn("Unexpected: template_id=47"sv);
}

void UDPInstrumentDefinition::operator()(
    Trace<cme_mdp::MDIncrementalRefreshTradeSummary48> const &, sbe::Frame const &) {
  log::warn<5>("Unexpected: template_id=48"sv);
}

void UDPInstrumentDefinition::operator()(
    Trace<cme_mdp::MDIncrementalRefreshDailyStatistics49> const &, sbe::Frame const &) {
  log::warn<5>("Unexpected: template_id=49"sv);
}

void UDPInstrumentDefinition::operator()(
    Trace<cme_mdp::MDIncrementalRefreshLimitsBanding50> const &, sbe::Frame const &) {
  log::warn<5>("Unexpected: template_id=50"sv);
}

void UDPInstrumentDefinition::operator()(
    Trace<cme_mdp::MDIncrementalRefreshSessionStatistics51> const &, sbe::Frame const &) {
  log::warn<5>("Unexpected: template_id=51"sv);
}

void UDPInstrumentDefinition::operator()(
    Trace<cme_mdp::MDIncrementalRefreshBookLongQty64> const &, sbe::Frame const &) {
  log::warn<5>("Unexpected: template_id=64"sv);
}

void UDPInstrumentDefinition::operator()(
    Trace<cme_mdp::MDIncrementalRefreshTradeSummaryLongQty65> const &, sbe::Frame const &) {
  log::warn<5>("Unexpected: template_id=65"sv);
}

void UDPInstrumentDefinition::operator()(
    Trace<cme_mdp::MDIncrementalRefreshVolumeLongQty66> const &, sbe::Frame const &) {
  log::warn<5>("Unexpected: template_id=66"sv);
}

void UDPInstrumentDefinition::operator()(
    Trace<cme_mdp::MDIncrementalRefreshSessionStatisticsLongQty67> const &, sbe::Frame const &) {
  log::warn<5>("Unexpected: template_id=67"sv);
}

// - MDIncrementalRefresh

void UDPInstrumentDefinition::operator()(metrics::Writer &writer) {
  writer  //
      .write(counter_.disconnect, metrics::COUNTER)
      .write(profile_.parse, metrics::PROFILE);
}

void UDPInstrumentDefinition::publish_stream_status(TraceInfo const &trace_info, ConnectionStatus connection_status) {
  if (!utils::update(connection_status_, connection_status))
    return;
  const StreamStatus stream_status{
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

Aggregator &UDPInstrumentDefinition::get_aggregator(uint16_t channel_id) {
  auto iter = aggregator_.find(channel_id);
  if (iter == std::end(aggregator_)) {
    iter = aggregator_.emplace(channel_id, server::Flags::cache_mbp_max_depth()).first;
  }
  return (*iter).second;
}

}  // namespace cme
}  // namespace roq
