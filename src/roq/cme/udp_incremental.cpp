/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/cme/udp_incremental.hpp"

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
auto const NAME = "udp_ia"sv;

Mask<SupportType> const SUPPORTS{
    SupportType::TOP_OF_BOOK,
    SupportType::MARKET_BY_PRICE,
    SupportType::STATISTICS,
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

template <typename Callback>
bool get_security(auto &shared, auto &value, Callback callback) {
  if constexpr (utils::is_integer<decltype(value)>::value) {
    auto security_id = value;
    auto iter = shared.securities.find(security_id);
    if (iter == std::end(shared.securities))
      return false;
    callback((*iter).second);
    return true;
  } else {
    auto security_id = value.securityID();
    auto iter = shared.securities.find(security_id);
    if (iter == std::end(shared.securities))
      return false;
    callback((*iter).second);
    return true;
  }
}

template <typename T>
void emplace(MBPUpdate &result, T const &item, auto &security) {
  auto price = sbe::get_double(const_cast<T &>(item).mDEntryPx());
  auto quantity = sbe::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue());
  auto number_of_orders = sbe::get_int(item.numberOfOrders(), item.numberOfOrdersNullValue());
  auto price_level = sbe::get_int(item.mDPriceLevel(), item.mDPriceLevelNullValue());
  new (&result) MBPUpdate{
      .price = price * security.display_factor,
      .quantity = utils::safe_cast(quantity),
      .implied_quantity = NaN,
      .number_of_orders = utils::safe_cast(number_of_orders),
      .update_action = {},
      .price_level = utils::safe_cast(price_level),
  };
}

template <typename T>
void emplace(Trade &result, T const &item, auto &security) {
  auto price = sbe::get_double(const_cast<T &>(item).mDEntryPx());
  auto quantity = sbe::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue());
  new (&result) Trade{
      .side = sbe::map_side(item.aggressorSide()),
      .price = price * security.display_factor,
      .quantity = utils::safe_cast(quantity),
      .trade_id = {},
  };
  auto trade_id = sbe::get_int(item.mDTradeEntryID(), item.mDTradeEntryIDNullValue());
  fmt::format_to(std::back_inserter(result.trade_id), "{}"sv, trade_id);
}

template <typename T>
void trade_summary_emplace_back(auto &result, T const &item, auto &security) {
  result.emplace_back([&item, &security](auto &result) { emplace(result, item, security); });
}

template <typename T>
void emplace_price(Statistics &result, auto type, T const &item, auto factor) {
  auto value = sbe::get_double(const_cast<T &>(item).mDEntryPx());
  new (&result) Statistics{
      .type = type,
      .value = value * factor,
      .begin_time_utc = {},
      .end_time_utc = {},
  };
}

template <typename T>
void emplace_size(Statistics &result, auto type, T const &item) {
  auto value = sbe::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue());
  new (&result) Statistics{
      .type = type,
      .value = utils::safe_cast(value),
      .begin_time_utc = {},
      .end_time_utc = {},
  };
}

template <typename T>
void statistics_emplace_back(auto &result, T const &item, auto &security) {
  auto statistics_type = sbe::map(item.mDEntryType());
  if (statistics_type == StatisticsType::OPEN_INTEREST) {
    result.emplace_back([&](auto &result) { emplace_size(result, statistics_type, item); });
  } else {
    result.emplace_back([&](auto &result) { emplace_price(result, statistics_type, item, security.display_factor); });
  }
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

UDPIncremental::UDPIncremental(Handler &handler, io::Context &context, uint16_t stream_id, Shared &shared)
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

void UDPIncremental::operator()(Event<Start> const &) {
  auto trace_info = server::create_trace_info();
  publish_stream_status(trace_info, ConnectionStatus::CONNECTING);
  last_update_time_ = trace_info.source_receive_time;
}

void UDPIncremental::operator()(Event<Stop> const &) {
}

void UDPIncremental::operator()(Event<Timer> const &event) {
  if (last_update_time_.count() && (last_update_time_ + flags::Multicast::multicast_timeout()) < event.value.now) {
    log::warn("*** DETECTED TIMEOUT ***"sv);
    last_update_time_ = {};
  }
}

void UDPIncremental::operator()(io::net::udp::Receiver::Read const &) {
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
    receive_buffer_.drain(std::size(message));
  }
}

void UDPIncremental::operator()(io::net::udp::Receiver::Error const &error) {
  log::fatal("Error: what={}"sv, error.what);
}

// sbe::Parser::Handler

void UDPIncremental::operator()(Trace<cme_mdp::AdminHeartbeat12> const &event, sbe::Frame const &frame) {
  auto &[trace_info, value] = event;
  log::info<5>("admin_heartbeat_12={}, frame={}"sv, const_cast<decltype(value) &>(value), frame);
}

// - MDInstrumentDefinition

void UDPIncremental::operator()(Trace<cme_mdp::MDInstrumentDefinitionFuture54> const &event, sbe::Frame const &frame) {
  auto &[trace_info, value] = event;
  log::info<3>("md_instrument_definition_future_54={}, frame={}"sv, const_cast<decltype(value) &>(value), frame);
}

void UDPIncremental::operator()(Trace<cme_mdp::MDInstrumentDefinitionOption55> const &event, sbe::Frame const &frame) {
  auto &[trace_info, value] = event;
  log::info<3>("md_instrument_definition_option_55={}, frame={}"sv, const_cast<decltype(value) &>(value), frame);
}

void UDPIncremental::operator()(Trace<cme_mdp::MDInstrumentDefinitionSpread56> const &event, sbe::Frame const &frame) {
  auto &[trace_info, value] = event;
  log::info<3>("md_instrument_definition_spread_56={}, frame={}"sv, const_cast<decltype(value) &>(value), frame);
}

void UDPIncremental::operator()(
    Trace<cme_mdp::MDInstrumentDefinitionFixedIncome57> const &event, sbe::Frame const &frame) {
  auto &[trace_info, value] = event;
  log::info<3>("md_instrument_definition_fixed_income_57={}, frame={}"sv, const_cast<decltype(value) &>(value), frame);
}

void UDPIncremental::operator()(Trace<cme_mdp::MDInstrumentDefinitionRepo58> const &event, sbe::Frame const &frame) {
  auto &[trace_info, value] = event;
  log::info<3>("md_instrument_definition_repo_58={}, frame={}"sv, const_cast<decltype(value) &>(value), frame);
}

void UDPIncremental::operator()(Trace<cme_mdp::MDInstrumentDefinitionFX63> const &event, sbe::Frame const &frame) {
  auto &[trace_info, value] = event;
  log::info<3>("md_instrument_definition_fx_63={}, frame={}"sv, const_cast<decltype(value) &>(value), frame);
}

// - MbP

void UDPIncremental::operator()(Trace<cme_mdp::SnapshotFullRefresh52> const &event, sbe::Frame const &frame) {
  auto &trace_info = event.trace_info;
  auto &value = event.value;
  log::info<3>("snapshot_full_refresh_52={}, frame={}"sv, const_cast<decltype(value) &>(value), frame);
  get_security(shared_, value, [&](auto &security) {
    std::chrono::nanoseconds exchange_time_utc{value.transactTime()};
    auto exchange_sequence = value.lastMsgSeqNumProcessed();
    TopOfBook top_of_book{
        .stream_id = stream_id_,
        .exchange = security.exchange,
        .symbol = security.symbol,
        .layer = {},
        .exchange_time_utc = exchange_time_utc,
        .exchange_sequence = exchange_sequence,
    };
    core::back_emplacer bids{shared_.bids}, asks{shared_.asks};
    core::back_emplacer statistics{shared_.statistics};
    value.sbeRewind();  // note!
    value.noMDEntries().forEach(
        [&](auto const &item) { emplace_back(item, security, top_of_book.layer, bids, asks, statistics); });
    if (!(std::isnan(top_of_book.layer.bid_price) && std::isnan(top_of_book.layer.ask_price))) {
      log::info<3>("top_of_book={}"sv, top_of_book);
      create_trace_and_dispatch(handler_, trace_info, std::as_const(top_of_book), true);
    }
    MarketByPriceUpdate const market_by_price_update{
        .stream_id = stream_id_,
        .exchange = security.exchange,
        .symbol = security.symbol,
        .bids = bids,
        .asks = asks,
        .update_type = UpdateType::SNAPSHOT,
        .exchange_time_utc = exchange_time_utc,
        .exchange_sequence = exchange_sequence,
        .price_decimals = {},
        .quantity_decimals = {},
        .checksum = {},
    };
    if (!(std::empty(market_by_price_update.bids) && std::empty(market_by_price_update.bids))) {
      log::info<3>("market_by_price_update={}"sv, market_by_price_update);
      create_trace_and_dispatch(handler_, trace_info, market_by_price_update, true, false);
    }
    StatisticsUpdate const statistics_update{
        .stream_id = stream_id_,
        .exchange = security.exchange,
        .symbol = security.symbol,
        .statistics = statistics,
        .update_type = UpdateType::SNAPSHOT,
        .exchange_time_utc = exchange_time_utc,
    };
    if (!std::empty(statistics_update.statistics)) {
      log::info<3>("statistics_update={}"sv, statistics_update);
      create_trace_and_dispatch(handler_, trace_info, statistics_update, true);
    }
  });
}

void UDPIncremental::operator()(Trace<cme_mdp::SnapshotFullRefreshLongQty69> const &event, sbe::Frame const &frame) {
  auto &[trace_info, value] = event;
  log::info<3>("snapshot_full_refresh_long_qty_69={}, frame={}"sv, const_cast<decltype(value) &>(value), frame);
}

// MbO

void UDPIncremental::operator()(Trace<cme_mdp::SnapshotFullRefreshOrderBook53> const &event, sbe::Frame const &frame) {
  auto &[trace_info, value] = event;
  log::info<3>("snapshot_full_refresh_order_book_53={}, frame={}"sv, const_cast<decltype(value) &>(value), frame);
}

// - MDIncrementalRefresh

void UDPIncremental::operator()(Trace<cme_mdp::MDIncrementalRefreshVolume37> const &event, sbe::Frame const &frame) {
  auto &[trace_info, value] = event;
  log::info<3>("md_incremental_refresh_volume_37={}, frame={}"sv, const_cast<decltype(value) &>(value), frame);
  /*
  auto security_id = value.securityID();
  auto &collector = shared_.mbp_collector[security_id];
  core::back_emplacer bids{shared_.bids}, asks{shared_.asks};
  value.sbeRewind();  // note!
  value.noMDEntries().forEach(
      [&](auto const &item) { emplace_back(item, security, top_of_book.layer, bids, asks, statistics); });
  if (!(std::isnan(top_of_book.layer.bid_price) && std::isnan(top_of_book.layer.ask_price))) {
    log::info<3>("top_of_book={}"sv, top_of_book);
    create_trace_and_dispatch(handler_, trace_info, std::as_const(top_of_book), true);
  }
  */
}

void UDPIncremental::operator()(Trace<cme_mdp::MDIncrementalRefreshBook46> const &event, sbe::Frame const &frame) {
  auto &[trace_info, value] = event;
  log::info<3>("md_incremental_refresh_book_46={}, frame={}"sv, const_cast<decltype(value) &>(value), frame);
  /*
  auto security_id = value.securityID();
  auto &collector = shared_.mbp_collector[security_id];
  */
}

void UDPIncremental::operator()(Trace<cme_mdp::MDIncrementalRefreshOrderBook47> const &event, sbe::Frame const &frame) {
  auto &[trace_info, value] = event;
  log::info<3>("md_incremental_refresh_order_book_47={}, frame={}"sv, const_cast<decltype(value) &>(value), frame);
  /*
  auto security_id = value.securityID();
  auto &collector = shared_.mbp_collector[security_id];
  */
}

void UDPIncremental::operator()(
    Trace<cme_mdp::MDIncrementalRefreshTradeSummary48> const &event, sbe::Frame const &frame) {
  auto &[trace_info, value] = event;
  log::info<3>("md_incremental_refresh_trade_summary_48={}, frame={}"sv, const_cast<decltype(value) &>(value), frame);
  value.sbeRewind();  // note!
  std::chrono::nanoseconds exchange_time_utc{value.transactTime()};
  core::back_emplacer trades{shared_.trades};
  auto dispatch = [&](auto &trades, auto &security, auto is_last) {
    const TradeSummary trade_summary{
        .stream_id = stream_id_,
        .exchange = security.exchange,
        .symbol = security.symbol,
        .trades = trades,
        .exchange_time_utc = exchange_time_utc,
    };
    create_trace_and_dispatch(handler_, trace_info, trade_summary, is_last);
  };
  int32_t previous_security_id = {};
  Shared::Security *security = nullptr;
  value.noMDEntries().forEach([&](auto const &item) {
    auto security_id = item.securityID();
    if (security_id != previous_security_id) {
      if (!std::empty(trades)) {
        assert(security);
        dispatch(trades, *security, true);
        trades.clear();
      }
      previous_security_id = security_id;
      get_security(shared_, item, [&security](auto &security_) { security = &security_; });
    }
    assert(security);
    trade_summary_emplace_back(trades, item, *security);
  });
  if (!std::empty(trades)) {
    assert(security);
    dispatch(trades, *security, true);
  }
}

void UDPIncremental::operator()(
    Trace<cme_mdp::MDIncrementalRefreshDailyStatistics49> const &event, sbe::Frame const &frame) {
  auto &[trace_info, value] = event;
  log::info<3>(
      "md_incremental_refresh_daily_statistics_49={}, frame={}"sv, const_cast<decltype(value) &>(value), frame);
  value.sbeRewind();  // note!
  std::chrono::nanoseconds exchange_time_utc{value.transactTime()};
  core::back_emplacer statistics{shared_.statistics};
  auto dispatch = [&](auto &statistics, auto &security, auto is_last) {
    StatisticsUpdate const statistics_update{
        .stream_id = stream_id_,
        .exchange = security.exchange,
        .symbol = security.symbol,
        .statistics = statistics,
        .update_type = UpdateType::INCREMENTAL,
        .exchange_time_utc = exchange_time_utc,
    };
    log::info<3>("statistics_update={}"sv, statistics_update);
    create_trace_and_dispatch(handler_, trace_info, statistics_update, is_last);
  };
  int32_t previous_security_id = {};
  Shared::Security *security = nullptr;
  value.noMDEntries().forEach([&](auto const &item) {
    auto security_id = item.securityID();
    if (security_id != previous_security_id) {
      if (!std::empty(statistics)) {
        assert(security);
        dispatch(statistics, *security, true);
        statistics.clear();
      }
      previous_security_id = security_id;
      get_security(shared_, item, [&security](auto &security_) { security = &security_; });
    }
    assert(security);
    statistics_emplace_back(statistics, item, *security);
  });
  if (!std::empty(statistics)) {
    assert(security);
    dispatch(statistics, *security, true);
  }
}

void UDPIncremental::operator()(
    Trace<cme_mdp::MDIncrementalRefreshLimitsBanding50> const &event, sbe::Frame const &frame) {
  auto &[trace_info, value] = event;
  log::info<3>("md_incremental_refresh_limits_banding_50={}, frame={}"sv, const_cast<decltype(value) &>(value), frame);
}

void UDPIncremental::operator()(
    Trace<cme_mdp::MDIncrementalRefreshSessionStatistics51> const &event, sbe::Frame const &frame) {
  auto &[trace_info, value] = event;
  log::info<3>(
      "md_incremental_refresh_session_statistics_51={}, frame={}"sv, const_cast<decltype(value) &>(value), frame);
  value.sbeRewind();  // note!
  std::chrono::nanoseconds exchange_time_utc{value.transactTime()};
  core::back_emplacer statistics{shared_.statistics};
  auto dispatch = [&](auto &statistics, auto &security, auto is_last) {
    StatisticsUpdate const statistics_update{
        .stream_id = stream_id_,
        .exchange = security.exchange,
        .symbol = security.symbol,
        .statistics = statistics,
        .update_type = UpdateType::INCREMENTAL,
        .exchange_time_utc = exchange_time_utc,
    };
    log::info<3>("statistics_update={}"sv, statistics_update);
    create_trace_and_dispatch(handler_, trace_info, statistics_update, is_last);
  };
  int32_t previous_security_id = {};
  Shared::Security *security = nullptr;
  value.noMDEntries().forEach([&](auto const &item) {
    auto security_id = item.securityID();
    if (security_id != previous_security_id) {
      if (!std::empty(statistics)) {
        assert(security);
        dispatch(statistics, *security, true);
        statistics.clear();
      }
      previous_security_id = security_id;
      get_security(shared_, item, [&security](auto &security_) { security = &security_; });
    }
    assert(security);
    statistics_emplace_back(statistics, item, *security);
  });
  if (!std::empty(statistics)) {
    assert(security);
    dispatch(statistics, *security, true);
  }
}

void UDPIncremental::operator()(
    Trace<cme_mdp::MDIncrementalRefreshBookLongQty64> const &event, sbe::Frame const &frame) {
  auto &[trace_info, value] = event;
  log::info<3>("md_incremental_refresh_book_long_qty_64={}, frame={}"sv, const_cast<decltype(value) &>(value), frame);
}

void UDPIncremental::operator()(
    Trace<cme_mdp::MDIncrementalRefreshTradeSummaryLongQty65> const &event, sbe::Frame const &frame) {
  auto &[trace_info, value] = event;
  log::info<3>(
      "md_incremental_refresh_trade_summary_long_qty_65={}, frame={}"sv, const_cast<decltype(value) &>(value), frame);
  value.sbeRewind();  // note!
  std::chrono::nanoseconds exchange_time_utc{value.transactTime()};
  core::back_emplacer trades{shared_.trades};
  auto dispatch = [&](auto &trades, auto &security, auto is_last) {
    const TradeSummary trade_summary{
        .stream_id = stream_id_,
        .exchange = security.exchange,
        .symbol = security.symbol,
        .trades = trades,
        .exchange_time_utc = exchange_time_utc,
    };
    create_trace_and_dispatch(handler_, trace_info, trade_summary, is_last);
  };
  int32_t previous_security_id = {};
  Shared::Security *security = nullptr;
  value.noMDEntries().forEach([&](auto const &item) {
    auto security_id = item.securityID();
    if (security_id != previous_security_id) {
      if (!std::empty(trades)) {
        assert(security);
        dispatch(trades, *security, true);
        trades.clear();
      }
      previous_security_id = security_id;
      get_security(shared_, item, [&security](auto &security_) { security = &security_; });
    }
    assert(security);
    trade_summary_emplace_back(trades, item, *security);
  });
  if (!std::empty(trades)) {
    assert(security);
    dispatch(trades, *security, true);
  }
}

void UDPIncremental::operator()(
    Trace<cme_mdp::MDIncrementalRefreshVolumeLongQty66> const &event, sbe::Frame const &frame) {
  auto &[trace_info, value] = event;
  log::info<3>("md_incremental_refresh_volume_long_qty_66={}, frame={}"sv, const_cast<decltype(value) &>(value), frame);
}

void UDPIncremental::operator()(
    Trace<cme_mdp::MDIncrementalRefreshSessionStatisticsLongQty67> const &event, sbe::Frame const &frame) {
  auto &[trace_info, value] = event;
  log::info<3>(
      "md_incremental_refresh_session_statistics_long_qty_67={}, frame={}"sv,
      const_cast<decltype(value) &>(value),
      frame);
  value.sbeRewind();  // note!
  std::chrono::nanoseconds exchange_time_utc{value.transactTime()};
  core::back_emplacer statistics{shared_.statistics};
  auto dispatch = [&](auto &statistics, auto &security, auto is_last) {
    StatisticsUpdate const statistics_update{
        .stream_id = stream_id_,
        .exchange = security.exchange,
        .symbol = security.symbol,
        .statistics = statistics,
        .update_type = UpdateType::INCREMENTAL,
        .exchange_time_utc = exchange_time_utc,
    };
    log::info<3>("statistics_update={}"sv, statistics_update);
    create_trace_and_dispatch(handler_, trace_info, statistics_update, is_last);
  };
  int32_t previous_security_id = {};
  Shared::Security *security = nullptr;
  value.noMDEntries().forEach([&](auto const &item) {
    auto security_id = item.securityID();
    if (security_id != previous_security_id) {
      if (!std::empty(statistics)) {
        assert(security);
        dispatch(statistics, *security, true);
        statistics.clear();
      }
      previous_security_id = security_id;
      get_security(shared_, item, [&security](auto &security_) { security = &security_; });
    }
    assert(security);
    statistics_emplace_back(statistics, item, *security);
  });
  if (!std::empty(statistics)) {
    assert(security);
    dispatch(statistics, *security, true);
  }
}

// - MDIncrementalRefresh

void UDPIncremental::operator()(metrics::Writer &writer) {
  writer  //
      .write(counter_.disconnect, metrics::COUNTER)
      .write(profile_.parse, metrics::PROFILE);
}

void UDPIncremental::publish_stream_status(TraceInfo const &trace_info, ConnectionStatus connection_status) {
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

}  // namespace cme
}  // namespace roq
