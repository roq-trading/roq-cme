/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/cme/market_data/incremental.hpp"

#include "roq/utils/common.hpp"
#include "roq/utils/safe_cast.hpp"
#include "roq/utils/update.hpp"

#include "roq/utils/debug/hex/message.hpp"

#include "roq/logging.hpp"

#include "roq/cme/mdp/map.hpp"

using namespace std::literals;

namespace roq {
namespace cme {
namespace market_data {

// === CONSTANTS ===

namespace {
auto const CONNECTION_TYPE = mdp::ConnectionType::INCREMENTAL;

auto const TRANSPORT = Transport::UDP;
auto const PROTOCOL = Protocol::SBE;
auto const ENCODING = Mask{
    Encoding::SBE,
};

auto const SUPPORTS_1 = Mask{
    SupportType::REFERENCE_DATA,
    SupportType::MARKET_STATUS,
    SupportType::TOP_OF_BOOK,
    SupportType::MARKET_BY_PRICE,
    SupportType::MARKET_BY_ORDER,
    SupportType::TRADE_SUMMARY,
    SupportType::STATISTICS,
};

auto const SUPPORTS_2 = Mask{
    SupportType::REFERENCE_DATA,
    SupportType::MARKET_STATUS,
    SupportType::TOP_OF_BOOK,
    SupportType::MARKET_BY_PRICE,
    SupportType::MARKET_BY_ORDER,
    SupportType::TRADE_SUMMARY,
    SupportType::STATISTICS,
    SupportType::MARKET_BY_ORDER,
};
}  // namespace

// === HELPERS ===

namespace {
auto get_supports(auto enable_market_by_order) {
  return enable_market_by_order ? SUPPORTS_2 : SUPPORTS_1;
}

template <typename Callback>
void create_security(auto &shared, auto &value, Callback callback) {
  auto security_id = value.securityID();
  if (shared.security_definitions.has_security(security_id))
    return;
  auto market_segment_id = value.marketSegmentID();
  auto security_exchange = mdp::get_string_view(value.securityExchange(), value.securityExchangeLength());
  auto symbol = mdp::get_string_view(value.symbol(), value.symbolLength());
  auto security_group = mdp::get_string_view(value.securityGroup(), value.securityGroupLength());
  auto discard = shared.discard_symbol(symbol);
  auto security = tools::Security{
      .exchange = security_exchange,
      .symbol = symbol,
      .display_factor = map(value.displayFactor()),
      .discard = discard,
  };
  auto callback_2 = [&](auto &security) { callback(security); };
  shared.security_definitions.create_security(security_group, market_segment_id, security_id, std::move(security), callback_2);
}

struct SecurityIterator final {
  SecurityIterator(Shared &shared) : shared_{shared} {}

  template <typename T, typename Dispatch, typename Callback>
  void operator()(T &value, Dispatch dispatch, Callback callback) {
    value.forEach([&](auto &item) {
      auto security_id = item.securityID();
      if (security_id != security_id_) {
        if (security_)
          dispatch(security_id, *security_);
        security_id_ = security_id;
        if (shared_.security_definitions.get_security(security_id, [&](auto &security) { security_ = &security; })) {
        } else {
          security_ = nullptr;
        }
      }
      if (security_)
        callback(security_id_, *security_, item);
    });
    if (security_)
      dispatch(security_id_, *security_);
  }

 private:
  Shared &shared_;
  int32_t security_id_ = {};
  tools::Security *security_ = nullptr;
};

// following are used from several places

template <typename T>
void mbp_emplace_back(auto &result, T const &item, auto &security) {
  auto price = map(const_cast<T &>(item).mDEntryPx()).template get<double>();
  auto quantity = mdp::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue());
  auto number_of_orders = mdp::get_int(item.numberOfOrders(), item.numberOfOrdersNullValue());
  auto update_action = [&]() -> UpdateAction {
    constexpr bool has_md_update_action = requires(T const &t) { t.mDUpdateAction(); };
    if constexpr (has_md_update_action) {
      return map(item.mDUpdateAction());
    }
    return {};
  }();
  if (update_action == UpdateAction::DELETE)
    quantity = {};  // note! exchange gives us the *old* value / we need this to be zero
  auto md_price_level = mdp::get_int(item.mDPriceLevel(), item.mDPriceLevelNullValue());
  uint32_t price_level = md_price_level > 0 ? (md_price_level - 1) : 0;
  auto update = MBPUpdate{
      .price = price * security.display_factor,
      .quantity = utils::safe_cast(quantity),
      .implied_quantity = NaN,
      .number_of_orders = utils::safe_cast(number_of_orders),
      .update_action = update_action,
      .price_level = price_level,
  };
  result.emplace_back(std::move(update));
}

template <typename T>
void trades_emplace_back(auto &result, T const &item, auto &security) {
  auto price = map(const_cast<T &>(item).mDEntryPx()).template get<double>();
  auto quantity = mdp::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue());
  auto trade = Trade{
      .side = map(item.aggressorSide()),
      .price = price * security.display_factor,
      .quantity = utils::safe_cast(quantity),
      .trade_id = {},
      .taker_order_id = {},
      .maker_order_id = {},
  };
  auto trade_id = mdp::get_int(item.mDTradeEntryID(), item.mDTradeEntryIDNullValue());
  fmt::format_to(std::back_inserter(trade.trade_id), "{}"sv, trade_id);
  result.emplace_back(std::move(trade));
}

template <typename T>
void statistics_emplace_back_price(auto &result, auto type, T const &item, auto factor) {
  auto value = map(const_cast<T &>(item).mDEntryPx()).template get<double>();
  auto statistics = Statistics{
      .type = type,
      .value = value * factor,
      .begin_time_utc = {},
      .end_time_utc = {},
  };
  result.emplace_back(std::move(statistics));
}

void statistics_emplace_back_size(auto &result, auto type, auto const &item) {
  auto value = mdp::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue());
  auto statistics = Statistics{
      .type = type,
      .value = utils::safe_cast(value),
      .begin_time_utc = {},
      .end_time_utc = {},
  };
  result.emplace_back(std::move(statistics));
}

template <typename T>
void statistics_emplace_back(auto &result, T const &item, auto &security) {
  auto statistics_type = map(item.mDEntryType()).template get<StatisticsType>();
  if (statistics_type == StatisticsType::OPEN_INTEREST) {
    statistics_emplace_back_size(result, statistics_type, item);
  } else {
    statistics_emplace_back_price(result, statistics_type, item, security.display_factor);
  }
}

void emplace_back(cme_mdp::MDIncrementalRefreshBook46::NoMDEntries const &item, auto &security, auto &layer, auto &bids, auto &asks) {
  using value_type = typename std::remove_cvref<decltype(item)>::type;
  switch (item.mDEntryType()) {
    using enum cme_mdp::MDEntryTypeBook::Value;
    case Bid:
      mbp_emplace_back(bids, item, security);
      break;
    case Offer:
      mbp_emplace_back(asks, item, security);
      break;
    case ImpliedBid:
      break;
    case ImpliedOffer:
      break;
    case BookReset:  // XXX ????????????????????????
      break;
    case MarketBestOffer: {
      auto price = map(const_cast<value_type &>(item).mDEntryPx()).template get<double>();
      layer.ask_price = price * security.display_factor;
      break;
    }
    case MarketBestBid: {
      auto price = map(const_cast<value_type &>(item).mDEntryPx()).template get<double>();
      layer.bid_price = price * security.display_factor;
      break;
    }
    case NULL_VALUE:
      break;
  }
}

void emplace_back(cme_mdp::MDIncrementalRefreshBook46::NoOrderIDEntries const &item, auto &security, auto side, auto price, auto &orders) {
  auto quantity = mdp::get_int(item.mDDisplayQty(), item.mDDisplayQtyNullValue());
  auto priority = mdp::get_int(item.mDOrderPriority(), item.mDOrderPriorityNullValue());
  auto order_id = mdp::get_int(item.orderID(), item.orderIDNullValue());
  auto order = MBOUpdate{
      .price = price * security.display_factor,
      .quantity = static_cast<double>(quantity),
      .priority = priority,
      .order_id = {},
      .side = side,
      .action = map(item.orderUpdateAction()),
      .reason = {},
  };
  fmt::format_to(std::back_inserter(order.order_id), "{}"sv, order_id);
  if (order.action != UpdateAction::DELETE && !quantity) [[unlikely]]  // DEBUG
    log::warn("Unexpected: update={}"sv, order);
  orders.emplace_back(std::move(order));
}

template <typename T>
void emplace_back(cme_mdp::MDIncrementalRefreshOrderBook47::NoMDEntries const &item, auto &security, auto security_id, T &orders) {
  using value_type = typename std::remove_cvref<decltype(item)>::type;
  using result_type = typename T::value_type;
  auto create_update = [&](auto side) {
    auto price = map(const_cast<value_type &>(item).mDEntryPx()).template get<double>();
    auto quantity = mdp::get_int(item.mDDisplayQty(), item.mDDisplayQtyNullValue());
    auto priority = mdp::get_int(item.mDOrderPriority(), item.mDOrderPriorityNullValue());
    auto order_id = mdp::get_int(item.orderID(), item.orderIDNullValue());
    auto result = result_type{
        .security_id = security_id,
        .price = price * security.display_factor,
        .quantity = static_cast<double>(quantity),
        .priority = priority,
        .order_id = order_id,
        .side = side,
        .action = map(item.mDUpdateAction()),
    };
    return result;
  };
  using value_type = typename std::remove_cvref<decltype(item)>::type;
  switch (item.mDEntryType()) {
    using enum cme_mdp::MDEntryTypeBook::Value;
    case Bid: {
      auto update = create_update(Side::BUY);
      orders.emplace_back(std::move(update));
      break;
    }
    case Offer: {
      auto update = create_update(Side::SELL);
      orders.emplace_back(std::move(update));
      break;
    }
    case ImpliedBid:
      break;
    case ImpliedOffer:
      break;
    case BookReset:  // XXX ????????????????????????
      break;
    case MarketBestOffer:
      break;
    case MarketBestBid:
      break;
    case NULL_VALUE:
      break;
  }
}
}  // namespace

// === IMPLEMENTATION ===

Incremental::Incremental(Shared &shared, Cache &cache, Channel &channel, uint16_t stream_id, mdp::Config const &config, uint16_t channel_id, Priority priority)
    : priority{priority}, stream_id{stream_id}, name{config.get_name(channel_id, CONNECTION_TYPE, priority)}, shared_{shared}, cache_{cache},
      channel_{channel} {
}

void Incremental::operator()(Event<Start> const &) {
}

void Incremental::operator()(Event<Stop> const &) {
}

void Incremental::operator()(Event<Timer> const &event) {
  if (last_update_time_.count() && (last_update_time_ + shared_.options.multicast_timeout) < event.value.now) {
    log::warn("*** DETECTED TIMEOUT ***"sv);
    last_update_time_ = {};
  }
}

void Incremental::dispatch(std::span<std::byte const> const &payload, TraceInfo const &trace_info) {
  publish_stream_status(trace_info, ConnectionStatus::READY);  // first message will publish
  auto parse = [&](auto &message) { mdp::Parser::dispatch(*this, message, trace_info); };
  assert(!std::empty(payload));
  using value_type = typename decltype(channel_.incremental.buffer)::value_type;
  auto stop = false;
  if (channel_.incremental.buffer.next([&](auto buffer) -> std::pair<size_t, value_type> {
        // read into buffer
        auto bytes = std::size(payload);
        assert(bytes <= std::size(buffer));
        std::memcpy(std::data(buffer), std::data(payload), bytes);
        /*
        auto bytes = receiver.recv(buffer);
        log::info<5>("Received {} byte(s) (stream_id={})"sv, bytes, stream_id);
        if (!bytes) {
          stop = true;
          return {};
        }
        */
        // parse message
        std::span message{std::data(buffer), bytes};
        log::info<5>("{}"sv, utils::debug::hex::Message{message});
        bool hold = false, drop = false;
        value_type sequence_number = {};
        if (mdp::Frame::parse(message, [&](auto &frame) {
              log::info<5>("frame={}, last_sequence_number={}"sv, frame, channel_.sequence.last_sequence_number);
              // check sequence number
              sequence_number = frame.sequence_number;
              if (sequence_number < shared_.options.filter_snapshot_from_incremental) {
                drop = true;
                return;
              }
              if (channel_.sequence.first_sequence_number) {
                auto next_sequence_number = channel_.sequence.last_sequence_number + 1;
                hold = sequence_number > next_sequence_number;
                drop = sequence_number < next_sequence_number;
              }
              if (hold) {
                auto delta = static_cast<int64_t>(sequence_number) - static_cast<int64_t>(channel_.sequence.last_sequence_number);
                log::warn("DEBUG HOLD sequence_number={}, last_sequence_number={}, delta={}"sv, sequence_number, channel_.sequence.last_sequence_number, delta);
              }
              /*
              // TEST >>>
              if (options.test.drop && !hold && !drop) {
                if ((sequence_number % options.test.drop) == 0) {
                  log::warn("DEBUG: *** SIMULATE DROP ***"sv);
                  drop = true;
                }
              }
              if (options.test.reordering && !hold && !drop) {
                if ((sequence_number % options.test.reordering) == 0) {
                  log::warn("DEBUG: *** SIMULATE REORDERING ***"sv);
                  hold = true;
                  stop = true;
                }
              }
              // <<< TEST
              */
            })) {
          log::info<5>("hold={}, drop={}"sv, hold, drop);
          if (drop)
            return {};
          if (hold)
            return {bytes, sequence_number};
          // parse this message
          channel_.update_sequence_number(sequence_number);
          parse(message);
          return {};
        } else {
          // failed to parse frame
          log::warn("Unexpected"sv);
          return {};  // XXX not sure what to do here
        }
      })) {
    // successfully parsed a message
    // now process any withheld messages
    while (!stop) {
      if (channel_.sequence.first_sequence_number) {
        // check next sequence number
        auto sequence_number = channel_.sequence.last_sequence_number + 1;
        if (channel_.incremental.buffer.get(sequence_number, [&](auto &message) {
              // parse this message
              channel_.update_sequence_number(sequence_number);
              parse(message);
            })) {
        } else {
          // does not exist
          stop = true;
        }
      } else {
        // not ready
        stop = true;
      }
    }
  } else {
    // full: no available buffer
    log::warn<0>("*** BUFFER FULL ***"sv);  // XXX should be log level 1
    channel_.incremental.buffer.clear();
    on_sequence_reset();
    // XXX resubscribe
  }
}

// mdp::Parser::Handler

void Incremental::operator()(mdp::Frame const &) {
}

void Incremental::operator()(Trace<cme_mdp::AdminHeartbeat12> const &event, mdp::Frame const &frame) {
  auto &[trace_info, value] = event;
  log::info<5>("admin_heartbeat_12={}, frame={}"sv, value, frame);
  auto external_latency = ExternalLatency{
      .stream_id = stream_id,
      .account = {},
      .latency = trace_info.origin_create_time_utc - frame.sending_time,
  };
  create_trace_and_dispatch(shared_, trace_info, external_latency);
}

void Incremental::operator()(Trace<cme_mdp::ChannelReset4> const &event, mdp::Frame const &frame) {
  auto &[trace_info, value] = event;
  log::info<5>("channel_reset_4={}, frame={}"sv, value, frame);
}

void Incremental::operator()(Trace<cme_mdp::SecurityStatus30> const &event, mdp::Frame const &frame) {
  auto &trace_info = event.trace_info;
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("security_status_30={}, frame={}"sv, value, frame);
  value.sbeRewind();  // note!
  auto security_id = mdp::get_int(value.securityID(), value.securityIDNullValue());
  auto security_group = mdp::get_string_view(value.securityGroup(), value.securityGroupLength());
  auto exchange_time_utc = std::chrono::nanoseconds{value.transactTime()};
  auto dispatch = [&](auto security_id) {
    shared_.security_definitions.get_security(security_id, [&](auto &security) {
      auto market_status = MarketStatus{
          .stream_id = stream_id,
          .exchange = security.exchange,
          .symbol = security.symbol,
          .trading_status = map(value.securityTradingStatus()),
          .exchange_time_utc = exchange_time_utc,
          .exchange_sequence = frame.sequence_number,
          .sending_time_utc = frame.sending_time,
      };
      create_trace_and_dispatch(shared_, trace_info, market_status, true);
    });
  };
  if (security_id)
    dispatch(security_id);
  if (!std::empty(security_group))
    shared_.security_definitions.get_security_group(security_group, [&](auto security_id) { dispatch(security_id); });
}

void Incremental::operator()(Trace<cme_mdp::MDInstrumentDefinitionFuture54> const &event, mdp::Frame const &frame) {
  auto &trace_info = event.trace_info;
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_instrument_definition_future_54={}, frame={}"sv, value, frame);
  value.sbeRewind();  // note!
  [[maybe_unused]] auto security_id = value.securityID();
  /*
  auto security_id = value.securityID();
  shared_.security_definitions.get_security_incl_discard(security_id, [&](auto &security) {
    auto reference_data = mdp::create_reference_data(value, stream_id, security);
    create_trace_and_dispatch(shared_, trace_info, reference_data, true);
    if (security.discard)
      return;
    auto market_status = mdp::create_market_status(value, stream_id, security);
    create_trace_and_dispatch(shared_, trace_info, market_status, true);
  });
  */
  create_security(shared_, value, [&](auto &security) {
    auto reference_data = mdp::create_reference_data(value, stream_id, security);
    create_trace_and_dispatch(shared_, trace_info, reference_data, true);
    if (security.discard)
      return;
    auto market_status = mdp::create_market_status(value, stream_id, security);
    create_trace_and_dispatch(shared_, trace_info, market_status, true);
  });
}

void Incremental::operator()(Trace<cme_mdp::MDInstrumentDefinitionOption55> const &event, mdp::Frame const &frame) {
  auto &trace_info = event.trace_info;
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_instrument_definition_option_55={}, frame={}"sv, value, frame);
  value.sbeRewind();  // note!
  auto security_id = value.securityID();
  shared_.security_definitions.get_security_incl_discard(security_id, [&](auto &security) {
    auto reference_data = mdp::create_reference_data(value, stream_id, security);
    create_trace_and_dispatch(shared_, trace_info, reference_data, true);
    if (security.discard)
      return;
    auto market_status = mdp::create_market_status(value, stream_id, security);
    create_trace_and_dispatch(shared_, trace_info, market_status, true);
  });
}

void Incremental::operator()(Trace<cme_mdp::MDInstrumentDefinitionSpread56> const &event, mdp::Frame const &frame) {
  auto &trace_info = event.trace_info;
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_instrument_definition_spread_56={}, frame={}"sv, value, frame);
  value.sbeRewind();  // note!
  auto security_id = value.securityID();
  shared_.security_definitions.get_security_incl_discard(security_id, [&](auto &security) {
    auto reference_data = mdp::create_reference_data(value, stream_id, security);
    create_trace_and_dispatch(shared_, trace_info, reference_data, true);
    if (security.discard)
      return;
    auto market_status = mdp::create_market_status(value, stream_id, security);
    create_trace_and_dispatch(shared_, trace_info, market_status, true);
  });
}

void Incremental::operator()(Trace<cme_mdp::MDInstrumentDefinitionFixedIncome57> const &event, mdp::Frame const &frame) {
  auto &trace_info = event.trace_info;
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_instrument_definition_fixed_income_57={}, frame={}"sv, value, frame);
  value.sbeRewind();  // note!
  auto security_id = value.securityID();
  shared_.security_definitions.get_security_incl_discard(security_id, [&](auto &security) {
    auto reference_data = mdp::create_reference_data(value, stream_id, security);
    create_trace_and_dispatch(shared_, trace_info, reference_data, true);
    if (security.discard)
      return;
    auto market_status = mdp::create_market_status(value, stream_id, security);
    create_trace_and_dispatch(shared_, trace_info, market_status, true);
  });
}

void Incremental::operator()(Trace<cme_mdp::MDInstrumentDefinitionRepo58> const &event, mdp::Frame const &frame) {
  auto &trace_info = event.trace_info;
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_instrument_definition_repo_58={}, frame={}"sv, value, frame);
  value.sbeRewind();  // note!
  auto security_id = value.securityID();
  shared_.security_definitions.get_security_incl_discard(security_id, [&](auto &security) {
    auto reference_data = mdp::create_reference_data(value, stream_id, security);
    create_trace_and_dispatch(shared_, trace_info, reference_data, true);
    if (security.discard)
      return;
    auto market_status = mdp::create_market_status(value, stream_id, security);
    create_trace_and_dispatch(shared_, trace_info, market_status, true);
  });
}

void Incremental::operator()(Trace<cme_mdp::MDInstrumentDefinitionFX63> const &event, mdp::Frame const &frame) {
  auto &trace_info = event.trace_info;
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_instrument_definition_fx_63={}, frame={}"sv, value, frame);
  value.sbeRewind();  // note!
  auto security_id = value.securityID();
  shared_.security_definitions.get_security_incl_discard(security_id, [&](auto &security) {
    auto reference_data = mdp::create_reference_data(value, stream_id, security);
    create_trace_and_dispatch(shared_, trace_info, reference_data, true);
    if (security.discard)
      return;
    auto market_status = mdp::create_market_status(value, stream_id, security);
    create_trace_and_dispatch(shared_, trace_info, market_status, true);
  });
}

void Incremental::operator()(Trace<cme_mdp::SnapshotFullRefresh52> const &, mdp::Frame const &) {
}

void Incremental::operator()(Trace<cme_mdp::SnapshotFullRefreshLongQty69> const &, mdp::Frame const &) {
}

void Incremental::operator()(Trace<cme_mdp::QuoteRequest39> const &, mdp::Frame const &) {
}

void Incremental::operator()(Trace<cme_mdp::MDIncrementalRefreshBook46> const &event, mdp::Frame const &frame) {
  auto &trace_info = event.trace_info;
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_book_46={}, frame={}"sv, value, frame);
  value.sbeRewind();  // note!
  // ---
  auto exchange_sequence = frame.sequence_number;
  auto exchange_time_utc = std::chrono::nanoseconds{value.transactTime()};
  // note! MBO contains indexed references to MBP entries
  shared_.md_entries_.clear();
  auto is_snapshot_workaround = false;  // note! we assume that a snapshot only includes a single security_id
  {                                     // MBP
    Layer layer;
    auto &mbp = shared_.get_mbp();
    auto &mbo = shared_.get_mbo();
    auto dispatch = [&](auto security_id, auto &security, auto is_last, auto is_snapshot) {
      if (!(std::isnan(layer.bid_price) && std::isnan(layer.ask_price))) {
        auto top_of_book = TopOfBook{
            .stream_id = stream_id,
            .exchange = security.exchange,
            .symbol = security.symbol,
            .layer = layer,
            .update_type = UpdateType::SNAPSHOT,
            .exchange_time_utc = exchange_time_utc,
            .exchange_sequence = exchange_sequence,
            .sending_time_utc = frame.sending_time,
        };
        create_trace_and_dispatch(shared_, trace_info, std::as_const(top_of_book), is_last);
        layer = {};
      }
      if (!std::empty(mbp)) {
        dispatch_market_by_price(trace_info, security_id, security, exchange_sequence, exchange_time_utc, frame.sending_time, mbp.bids, mbp.asks, is_snapshot);
        mbp.clear();
      }
      if (!std::empty(mbo)) {
        dispatch_market_by_order(trace_info, security_id, security, exchange_sequence, exchange_time_utc, frame.sending_time, mbo.orders, false);
        mbo.clear();
      }
    };
    // HANS here we need to append every item regardless of security ==> can't use SecurityIterator
    int32_t security_id = {};
    tools::Security *security = nullptr;
    auto is_snapshot = false;
    auto process = [&](auto &item) {
      auto current_security_id = item.securityID();
      if (current_security_id != security_id) {
        if (security)
          dispatch(security_id, *security, true, is_snapshot);
        security_id = current_security_id;
        if (shared_.security_definitions.get_security(security_id, [&security](auto &security_2) { security = &security_2; })) {
        } else {
          security = nullptr;
        }
        is_snapshot = false;
      }
      if (item.rptSeq() == 1) {
        is_snapshot = true;
        is_snapshot_workaround = true;  // XXX
      }
      if (security)
        check_report_sequence(*security, item, frame);
      using value_type = typename std::remove_cvref<decltype(item)>::type;
      auto &value = const_cast<value_type &>(item);  // note! not const-safe
      auto price = map(value.mDEntryPx()).template get<double>();
      auto side = map(item.mDEntryType()).template get<Side>();
      auto action = map(item.mDUpdateAction()).template get<UpdateAction>();
      // ... need these for MBO referencing
      if (shared_.options.enable_market_by_order)
        shared_.md_entries_.emplace_back(security_id, side, price, action);
      if (security) {
        emplace_back(item, *security, layer, mbp.bids, mbp.asks);
        if (shared_.options.enable_market_by_order) {
          if (action == UpdateAction::DELETE && side != Side::UNDEFINED && shared_.options.mbp_to_mbo_clear_price_level) {
            auto update = MBOUpdate{
                .price = price * (*security).display_factor,
                .quantity = {},
                .priority = {},
                .order_id = {},
                .side = side,
                .action = action,
                .reason = {},
            };
            mbo.orders.emplace_back(std::move(update));
          }
        }
      }
    };
    value.noMDEntries().forEach(process);
    if (security)
      dispatch(security_id, *security, true, is_snapshot);
  }
  if (shared_.options.enable_market_by_order) {
    // MBO
    int32_t security_id = {};
    tools::Security *security = nullptr;
    auto &orders = cache_.orders;
    auto dispatch = [&](auto security_id, auto &security) {
      if (std::empty(orders))
        return;
      dispatch_market_by_order(trace_info, security_id, security, exchange_sequence, exchange_time_utc, frame.sending_time, orders, is_snapshot_workaround);
      orders.clear();
    };
    auto process = [&](auto &item) {
      auto reference_id = mdp::get_int(item.referenceID(), item.referenceIDNullValue());
      if (!reference_id)
        return;
      auto index = static_cast<size_t>(reference_id) - 1;  // indexing is 1-based
      if (!(index < std::size(shared_.md_entries_))) [[unlikely]] {
        log::warn("Unexpected: index={}, len={}"sv, index, std::size(shared_.md_entries_));
        return;
      }
      auto [current_security_id, side, price, action] = shared_.md_entries_[index];
      if (current_security_id != security_id) {
        if (security)
          dispatch(security_id, *security);
        security_id = current_security_id;
        if (shared_.security_definitions.get_security(security_id, [&security](auto &security_2) { security = &security_2; })) {
        } else {
          security = nullptr;
        }
      }
      if (security) {
        // if (action != UpdateAction::DELETE)
        emplace_back(item, *security, side, price, orders);
      }
    };
    value.noOrderIDEntries().forEach(process);
    if (security)
      dispatch(security_id, *security);
    assert(std::empty(orders));
  }
}

void Incremental::operator()(Trace<cme_mdp::MDIncrementalRefreshBookLongQty64> const &event, mdp::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_book_long_qty_64={}, frame={}"sv, value, frame);
  auto dispatch = []([[maybe_unused]] auto security_id, [[maybe_unused]] auto &security) {};
  auto update = [&]([[maybe_unused]] auto security_id, auto &security, auto &item) { check_report_sequence(security, item, frame); };
  SecurityIterator{shared_}(value.noMDEntries(), dispatch, update);
}

void Incremental::operator()(Trace<cme_mdp::SnapshotFullRefreshOrderBook53> const &, mdp::Frame const &) {
}

void Incremental::operator()(Trace<cme_mdp::MDIncrementalRefreshOrderBook47> const &event, mdp::Frame const &frame) {
  if (!shared_.options.enable_market_by_order)
    return;
  auto &trace_info = event.trace_info;
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  value.sbeRewind();                                    // note!
  // ---
  // note! not possible to detect first message -- this is an issue after packet loss
  auto exchange_sequence = frame.sequence_number;
  auto exchange_time_utc = std::chrono::nanoseconds{value.transactTime()};
  auto &match_event_indicator = value.matchEventIndicator();
  auto last = !match_event_indicator.isEmpty();
  int32_t security_id = {};
  tools::Security *security = nullptr;
  auto process = [&](auto &item) {
    auto current_security_id = item.securityID();
    if (current_security_id != security_id) {
      security_id = current_security_id;
      if (shared_.security_definitions.get_security(security_id, [&security](auto &security_2) { security = &security_2; })) {
      } else {
        security = nullptr;
      }
      if (security)
        cache_.security_ids_47.insert(security_id);
    }
    if (security)
      emplace_back(item, *security, security_id, cache_.orders_47);
  };
  value.noMDEntries().forEach(process);
  if (!last)
    return;
  for (auto security_id : cache_.security_ids_47) {
    assert(std::empty(cache_.orders));
    auto dispatch = [&](auto &security) {
      for (auto &item : cache_.orders_47)
        if (item.security_id == security_id) {
          auto result = MBOUpdate{
              .price = item.price,
              .quantity = item.quantity,
              .priority = item.priority,
              .order_id = {},
              .side = item.side,
              .action = item.action,
              .reason = {},
          };
          fmt::format_to(std::back_inserter(result.order_id), "{}"sv, item.order_id);
          cache_.orders.emplace_back(std::move(result));
        }
      auto snapshot = !security.rpt_seq;
      dispatch_market_by_order(trace_info, security_id, security, exchange_sequence, exchange_time_utc, frame.sending_time, cache_.orders, snapshot);
      cache_.orders.clear();
    };
    shared_.security_definitions.get_security(security_id, dispatch);
  }
  cache_.orders_47.clear();
  cache_.security_ids_47.clear();
}

void Incremental::operator()(Trace<cme_mdp::MDIncrementalRefreshTradeSummary48> const &event, mdp::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_trade_summary_48={}, frame={}"sv, value, frame);
  dispatch_trade_summary(event, frame);
}

void Incremental::operator()(Trace<cme_mdp::MDIncrementalRefreshTradeSummaryLongQty65> const &event, mdp::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_trade_summary_long_qty_65={}, frame={}"sv, value, frame);
  dispatch_trade_summary(event, frame);
}

void Incremental::operator()(Trace<cme_mdp::MDIncrementalRefreshDailyStatistics49> const &event, mdp::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_daily_statistics_49={}, frame={}"sv, value, frame);
  auto callback = [](auto &statistics, auto &item, auto &security) { statistics_emplace_back(statistics, item, security); };
  dispatch_statistics(event, frame, callback);
}

void Incremental::operator()(Trace<cme_mdp::MDIncrementalRefreshSessionStatistics51> const &event, mdp::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_session_statistics_51={}, frame={}"sv, value, frame);
  auto callback = [](auto &statistics, auto &item, auto &security) { statistics_emplace_back(statistics, item, security); };
  dispatch_statistics(event, frame, callback);
}

void Incremental::operator()(Trace<cme_mdp::MDIncrementalRefreshSessionStatisticsLongQty67> const &event, mdp::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_session_statistics_long_qty_67={}, frame={}"sv, value, frame);
  auto callback = [](auto &statistics, auto &item, auto &security) { statistics_emplace_back(statistics, item, security); };
  dispatch_statistics(event, frame, callback);
}

void Incremental::operator()(Trace<cme_mdp::MDIncrementalRefreshVolume37> const &event, mdp::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_volume_37={}, frame={}"sv, value, frame);
  auto callback = [](auto &statistics, auto &item, [[maybe_unused]] auto &security) {
    statistics_emplace_back_size(statistics, StatisticsType::TRADE_VOLUME, item);
  };
  dispatch_statistics(event, frame, callback);
}

void Incremental::operator()(Trace<cme_mdp::MDIncrementalRefreshVolumeLongQty66> const &event, mdp::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_volume_long_qty_66={}, frame={}"sv, value, frame);
  auto dispatch = []([[maybe_unused]] auto security_id, [[maybe_unused]] auto &security) {};
  auto update = [&]([[maybe_unused]] auto security_id, auto &security, auto &item) { check_report_sequence(security, item, frame); };
  SecurityIterator{shared_}(value.noMDEntries(), dispatch, update);
}

void Incremental::operator()(Trace<cme_mdp::MDIncrementalRefreshLimitsBanding50> const &event, mdp::Frame const &frame) {
  using value_type = std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  log::info<5>("md_incremental_refresh_limits_banding_50={}, frame={}"sv, value, frame);
  value.sbeRewind();  // note!
  auto dispatch = []([[maybe_unused]] auto security_id, [[maybe_unused]] auto &security) {};
  auto update = [&]([[maybe_unused]] auto security_id, auto &security, auto &item) { check_report_sequence(security, item, frame); };
  SecurityIterator{shared_}(value.noMDEntries(), dispatch, update);
}

// helpers

void Incremental::dispatch_market_by_price(
    auto &trace_info,
    auto security_id,
    auto &security,
    auto exchange_sequence,
    auto exchange_time_utc,
    auto sending_time_utc,
    std::span<MBPUpdate const> const &bids,
    std::span<MBPUpdate const> const &asks,
    bool is_snapshot) {
  auto &sequencer = security.mbp.sequencer;
  try {
    auto last_exchange_sequence = sequencer.last_sequence();  // note! the protocol doesn't tell us
    auto create_update = [&](auto &bids, auto &asks, auto update_type) -> MarketByPriceUpdate {
      return {
          .stream_id = stream_id,
          .exchange = security.exchange,
          .symbol = security.symbol,
          .bids = bids,
          .asks = asks,
          .update_type = update_type,
          .exchange_time_utc = exchange_time_utc,
          .exchange_sequence = exchange_sequence,
          .sending_time_utc = sending_time_utc,
          .price_precision = {},
          .quantity_precision = {},
          .checksum = {},
      };
    };
    auto publish_update = [&](auto &bids, auto &asks) {
      auto market_by_price_update = create_update(bids, asks, UpdateType::INCREMENTAL);
      auto callback = []([[maybe_unused]] auto &market_by_price) {};
      create_trace_and_dispatch(shared_, trace_info, market_by_price_update, true, shared_.cache.bids, shared_.cache.asks, callback);
    };
    auto publish_snapshot = [&](auto &bids, auto &asks, auto exchange_sequence, auto retries, auto delay) {
      log::info(
          R"(PUBLISH MBP SNAPSHOT exchange="{}", symbol="{}", security_id={}, exchange_sequence={}, retries={}, delay={})"sv,
          security.exchange,
          security.symbol,
          security_id,
          exchange_sequence,
          retries,
          std::chrono::duration_cast<std::chrono::milliseconds>(delay));
      auto market_by_price_update = create_update(bids, asks, UpdateType::SNAPSHOT);
      auto callback = []([[maybe_unused]] auto &market_by_price) {};
      create_trace_and_dispatch(shared_, trace_info, market_by_price_update, true, shared_.cache.bids, shared_.cache.asks, callback);
      security.mbp.resubscribe = {};
    };
    auto request_snapshot = [&]([[maybe_unused]] auto retries) {
      log::info(
          R"(REQUEST MBP SNAPSHOT exchange="{}", symbol="{}", security_id={}, exchange_sequence={})"sv,
          security.exchange,
          security.symbol,
          security_id,
          exchange_sequence);
      security.mbp.resubscribe = exchange_sequence;
    };
    if (is_snapshot) {
      auto force = true;
      sequencer(bids, asks, exchange_sequence, force, publish_snapshot, request_snapshot);
    } else {
      sequencer(bids, asks, exchange_sequence, exchange_sequence, last_exchange_sequence, publish_update, publish_snapshot, request_snapshot);
    }
  } catch (BadState &) {
    log::warn(
        R"(RESUBSCRIBE MBP exchange="{}", symbol="{}", security_id={}, exchange_sequene={})"sv,
        security.exchange,
        security.symbol,
        security_id,
        exchange_sequence);
    // XXX HANS publish stale
    sequencer.clear();
    security.mbp.resubscribe = exchange_sequence;
  }
}

void Incremental::dispatch_market_by_price_stale(auto &trace_info, auto &security, auto exchange_sequence, auto exchange_time_utc, auto sending_time_utc) {
  auto market_by_price_update = MarketByPriceUpdate{
      .stream_id = stream_id,
      .exchange = security.exchange,
      .symbol = security.symbol,
      .bids = {},
      .asks = {},
      .update_type = UpdateType::STALE,
      .exchange_time_utc = exchange_time_utc,
      .exchange_sequence = exchange_sequence,
      .sending_time_utc = sending_time_utc,
      .price_precision = {},
      .quantity_precision = {},
      .checksum = {},
  };
  auto callback = []([[maybe_unused]] auto &market_by_price) {};
  create_trace_and_dispatch(shared_, trace_info, market_by_price_update, true, shared_.cache.bids, shared_.cache.asks, callback);
}

void Incremental::dispatch_market_by_order(
    auto &trace_info,
    auto security_id,
    auto &security,
    auto exchange_sequence,
    auto exchange_time_utc,
    auto sending_time_utc,
    std::span<MBOUpdate const> const &orders,
    bool is_snapshot) {
  auto &sequencer = security.mbo.sequencer;
  try {
    auto last_exchange_sequence = sequencer.last_sequence();  // note! the protocol doesn't tell us
    auto create_update = [&](auto &orders, auto update_type) -> MarketByOrderUpdate {
      return {
          .stream_id = stream_id,
          .exchange = security.exchange,
          .symbol = security.symbol,
          .orders = orders,
          .update_type = update_type,
          .exchange_time_utc = exchange_time_utc,
          .exchange_sequence = exchange_sequence,
          .sending_time_utc = sending_time_utc,
          .price_precision = {},
          .quantity_precision = {},
          .max_depth = {},
          .checksum = {},
      };
    };
    auto publish_update = [&](auto &orders) {
      auto market_by_order_update = create_update(orders, UpdateType::INCREMENTAL);
      auto callback = []([[maybe_unused]] auto &market_by_order) {};
      create_trace_and_dispatch(shared_, trace_info, market_by_order_update, true, shared_.cache.orders, callback);
    };
    auto publish_snapshot = [&](auto &orders, auto exchange_sequence, auto retries, auto delay) {
      log::info(
          R"(PUBLISH MBO SNAPSHOT exchange={}, symbol="{}", security_id={}, exchange_sequence={}, retries={}, delay={})"sv,
          security.exchange,
          security.symbol,
          security_id,
          exchange_sequence,
          retries,
          std::chrono::duration_cast<std::chrono::milliseconds>(delay));
      auto market_by_order_update = create_update(orders, UpdateType::SNAPSHOT);
      auto callback = []([[maybe_unused]] auto &market_by_order) {};
      create_trace_and_dispatch(shared_, trace_info, market_by_order_update, true, shared_.cache.orders, callback);
      security.mbo.resubscribe = {};
    };
    auto request_snapshot = [&]([[maybe_unused]] auto retries) {
      log::info(
          R"(REQUEST MBO SNAPSHOT exchange="{}", symbol="{}", security_id={}, exchange_sequence={}, retries={})"sv,
          security.exchange,
          security.symbol,
          security_id,
          exchange_sequence,
          retries);
      security.mbo.resubscribe = exchange_sequence;
    };
    log::info<5>(
        R"(DEBUG UPDATE exchange="{}", symbol="{}", orders=[{}], security_id={}, exchange_sequence={}, last_exchange_sequence={}, is_snapshot={})"sv,
        security.exchange,
        security.symbol,
        fmt::join(orders, ", "sv),
        security_id,
        exchange_sequence,
        last_exchange_sequence,
        is_snapshot);
    if (is_snapshot) {
      auto force = true;
      sequencer(orders, exchange_sequence, force, publish_snapshot, request_snapshot);
    } else {
      sequencer(orders, exchange_sequence, exchange_sequence, last_exchange_sequence, publish_update, publish_snapshot, request_snapshot);
    }
  } catch (BadState &) {
    log::warn(
        R"(RESUBSCRIBE MBO exchange="{}", symbol="{}",  security_id={}, exchange_sequence={})"sv,
        security.exchange,
        security.symbol,
        security_id,
        exchange_sequence);
    // XXX HANS publish stale
    sequencer.clear();
    security.mbo.resubscribe = exchange_sequence;
  }
}

void Incremental::dispatch_market_by_order_stale(auto &trace_info, auto &security, auto exchange_sequence, auto exchange_time_utc, auto sending_time_utc) {
  auto market_by_order_update = MarketByOrderUpdate{
      .stream_id = stream_id,
      .exchange = security.exchange,
      .symbol = security.symbol,
      .orders = {},
      .update_type = UpdateType::STALE,
      .exchange_time_utc = exchange_time_utc,
      .exchange_sequence = exchange_sequence,
      .sending_time_utc = sending_time_utc,
      .price_precision = {},
      .quantity_precision = {},
      .max_depth = {},
      .checksum = {},
  };
  auto callback = []([[maybe_unused]] auto &market_by_order) {};
  create_trace_and_dispatch(shared_, trace_info, market_by_order_update, true, shared_.cache.orders, callback);
}

template <typename T>
void Incremental::dispatch_trade_summary(Trace<T> const &event, mdp::Frame const &frame) {
  auto &trace_info = event.trace_info;
  using value_type = typename std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  value.sbeRewind();                                    // note!
  auto exchange_time_utc = std::chrono::nanoseconds{value.transactTime()};
  auto exchange_sequence = frame.sequence_number;
  auto clear_state = [&]() {
    shared_.security_ids_.clear();
    shared_.trade_summary_.clear();
    shared_.orders_.clear();
    shared_.total_number_of_orders_ = 0;
  };
  // log::warn("DEBUG exchange_time_utc={}, transact_time_={}"sv, exchange_time_utc, shared_.transact_time_);
  auto fragmented = exchange_time_utc == shared_.transact_time_;
  if (!fragmented) {
    // log::warn("DEBUG CLEAR_STATE exchange_time_utc={}, transact_time={}"sv, exchange_time_utc, transact_time_);
    shared_.transact_time_ = exchange_time_utc;
    clear_state();
    // log::warn("DEBUG exchange_time_utc={}, transact_time_={}"sv, exchange_time_utc, shared_.transact_time_);
  }
  auto insert_security_id = [&](auto security_id) {
    for (auto iter : shared_.security_ids_) {
      if (iter == security_id)
        return;
    }
    shared_.security_ids_.emplace_back(security_id);
  };
  value.noMDEntries().forEach([&]<typename U>(U &item) {
    auto security_id = item.securityID();
    auto aggressor_side = item.aggressorSide();
    auto price = map(const_cast<U &>(item).mDEntryPx()).template get<double>();
    auto size = item.mDEntrySize();
    auto number_of_orders = item.numberOfOrders();
    auto trade_id = mdp::get_int(item.mDTradeEntryID(), item.mDTradeEntryIDNullValue());
    insert_security_id(security_id);
    auto side = map(aggressor_side).template get<Side>();
    shared_.trade_summary_.emplace_back(security_id, side, price, size, number_of_orders, trade_id);
    shared_.total_number_of_orders_ += number_of_orders;
    shared_.security_definitions.get_security(security_id, [&](auto &security) { check_report_sequence(security, item, frame); });
  });
  value.noOrderIDEntries().forEach([&](auto &item) {
    auto order_id = item.orderID();
    auto last_qty = item.lastQty();
    shared_.orders_.emplace_back(order_id, last_qty);
  });
  if (std::size(shared_.orders_) < shared_.total_number_of_orders_) {
    log::warn<5>(
        "Message is fragmented: sequence={}, exchange_time_utc={}, len(orders)={}, expected={}"sv,
        exchange_sequence,
        exchange_time_utc,
        std::size(shared_.orders_),
        shared_.total_number_of_orders_);
    shared_.transact_time_ = exchange_time_utc;
    // log::warn("DEBUG exchange_time_utc={}, transact_time_={}"sv, exchange_time_utc, shared_.transact_time_);
    return;  // note!
  }
  if (std::size(shared_.orders_) > shared_.total_number_of_orders_) {
    log::warn(
        "Unexpected: sequence={}, exchange_time_utc={}, len(orders)={}, expected={}"sv,
        exchange_sequence,
        exchange_time_utc,
        std::size(shared_.orders_),
        shared_.total_number_of_orders_);
  } else if (fragmented) {
    log::info<5>(
        "DEBUG Message was fragmented and now fully assembled: sequence={}, exchange_time_utc={}, len(orders)={}, expected={}"sv,
        exchange_sequence,
        exchange_time_utc,
        std::size(shared_.orders_),
        shared_.total_number_of_orders_);
  }
  // mbp
  /*
  auto &mbp = shared_.get_mbp();
  auto dispatch_market_by_price_2 = [&](auto security_id, auto &security) {
    if (std::empty(mbp))
      return;
    dispatch_market_by_price(
        trace_info,
        security_id,
        security,
        exchange_sequence,
        exchange_time_utc,
        frame.sending_time,
        mbp.bids,
        mbp.asks);
    mbp.clear();
  };
  for (auto security_id : security_ids_) {
    shared_.security_definitions.get_security(security_id, [&](auto &security) {
      size_t offset = 0;
      for (auto [security_id_2, aggressor_side, price, size, number_of_orders, trade_id] : trade_summary_) {
        auto side = aggressor_side;
        if (security_id == security_id_2) {
          if (aggressor_side != Side::UNDEFINED) {
            auto &[order_id, last_qty] = orders_[offset];
            if (last_qty == size) {
              side = utils::invert(side);
              auto result = MBPUpdate{
                  .price = price,
                  .quantity = static_cast<double>(size),
                  .implied_quantity = NaN,
                  .number_of_orders = utils::safe_cast(number_of_orders),
                  .update_action = UpdateAction::TRADE,
                  .price_level = {},
              };
              switch (side) {
                using enum Side;
                case UNDEFINED:
                  assert(false);
                  break;
                case BUY:
                  mbp.asks.emplace_back(std::move(result));
                  break;
                case SELL:
                  mbp.bids.emplace_back(std::move(result));
                  break;
              }
            }
          }
          offset += number_of_orders;
        }
      }
      dispatch_market_by_price_2(security_id, security);
    });
  }
  */
  // mbo
  auto &mbo = shared_.get_mbo();
  auto dispatch_market_by_order_2 = [&](auto security_id, auto &security) {
    if (std::empty(mbo))
      return;
    dispatch_market_by_order(trace_info, security_id, security, exchange_sequence, exchange_time_utc, frame.sending_time, mbo.orders, false);
    mbo.clear();
  };
  for (auto security_id : shared_.security_ids_) {
    shared_.security_definitions.get_security(security_id, [&](auto &security) {
      size_t offset = 0;
      for (auto [security_id_2, aggressor_side, price, size, number_of_orders, trade_id] : shared_.trade_summary_) {
        auto side = aggressor_side;
        if (security_id == security_id_2) {
          size_t offset_2 = 0;
          if (aggressor_side != Side::UNDEFINED) {
            auto &[order_id, last_qty] = shared_.orders_[offset + offset_2];
            if (last_qty == size) {
              side = utils::invert(side);
              // XXX HANS are we missing the part of an aggressive order that remains in the book?
            }
            ++offset_2;
          }
          for (; offset_2 < number_of_orders; ++offset_2) {
            auto &[order_id, last_qty] = shared_.orders_[offset + offset_2];
            auto result = MBOUpdate{
                .price = price * security.display_factor,
                .quantity = static_cast<double>(last_qty),
                .priority = {},
                .order_id = {},
                .side = side,
                .action = UpdateAction::TRADE,
                .reason = {},
            };
            fmt::format_to(std::back_inserter(result.order_id), "{}"sv, order_id);
            if (!last_qty) [[unlikely]]  // DEBUG
              log::warn("Unexpected: update={}"sv, result);
            mbo.orders.emplace_back(std::move(result));
          }
        }
        offset += number_of_orders;
      }
      dispatch_market_by_order_2(security_id, security);
    });
  }
  // trades
  auto &trades = shared_.get_trades();
  auto dispatch_trade_summary = [&](auto &security) {
    if (std::empty(trades)) [[unlikely]] {  // DEBUG
      log::warn("DEBUG EMPTY TRADES"sv);
      return;
    }
    auto trade_summary = TradeSummary{
        .stream_id = stream_id,
        .exchange = security.exchange,
        .symbol = security.symbol,
        .trades = trades,
        .exchange_time_utc = exchange_time_utc,
        .exchange_sequence = frame.sequence_number,
        .sending_time_utc = frame.sending_time,
    };
    create_trace_and_dispatch(shared_, trace_info, trade_summary, true);
    trades.clear();
  };
  for (auto security_id : shared_.security_ids_) {
    shared_.security_definitions.get_security(security_id, [&](auto &security) {
      size_t offset = 0;
      for (auto [security_id_2, aggressor_side, price, size, number_of_orders, trade_id] : shared_.trade_summary_) {
        if (security_id == security_id_2) {
          size_t offset_2 = 0;
          auto trade_id_2 = fmt::format("{}"sv, trade_id);
          std::string taker_order_id;
          if (aggressor_side != Side::UNDEFINED) {
            auto &[order_id, last_qty] = shared_.orders_[offset + offset_2];
            if (last_qty == size) {
              taker_order_id = fmt::format("{}"sv, order_id);
              if (number_of_orders == 1) {
                auto trade = Trade{
                    .side = aggressor_side,
                    .price = price * security.display_factor,
                    .quantity = utils::safe_cast(last_qty),
                    .trade_id = trade_id_2,
                    .taker_order_id = {},
                    .maker_order_id = {},
                };
                fmt::format_to(std::back_inserter(trade.taker_order_id), "{}"sv, order_id);
                trades.emplace_back(std::move(trade));
              }
            } else {  // join
              auto trade = Trade{
                  .side = aggressor_side,
                  .price = price * security.display_factor,
                  .quantity = utils::safe_cast(last_qty),
                  .trade_id = trade_id_2,
                  .taker_order_id = {},
                  .maker_order_id = {},
              };
              fmt::format_to(std::back_inserter(trade.taker_order_id), "{}"sv, order_id);
              trades.emplace_back(std::move(trade));
            }
            ++offset_2;
          }
          for (; offset_2 < number_of_orders; ++offset_2) {
            auto &[order_id, last_qty] = shared_.orders_[offset + offset_2];
            auto trade = Trade{
                .side = aggressor_side,
                .price = price * security.display_factor,
                .quantity = utils::safe_cast(last_qty),
                .trade_id = trade_id_2,
                .taker_order_id = taker_order_id,
                .maker_order_id = {},
            };
            fmt::format_to(std::back_inserter(trade.maker_order_id), "{}"sv, order_id);
            trades.emplace_back(std::move(trade));
          }
        }
        offset += number_of_orders;
      }
      dispatch_trade_summary(security);
    });
  }
  // done
  if (std::size(shared_.orders_) < shared_.total_number_of_orders_)
    log::warn("Unexpected: len(orders)={}, total_number_of_orders={}"sv, std::size(shared_.orders_), shared_.total_number_of_orders_);
  shared_.transact_time_ = {};
  // log::warn("DEBUG exchange_time_utc={}, transact_time_={}"sv, exchange_time_utc, shared_.transact_time_);
  clear_state();
}

template <typename T, typename Callback>
void Incremental::dispatch_statistics(Trace<T> const &event, mdp::Frame const &frame, Callback callback) {
  auto &trace_info = event.trace_info;
  using value_type = typename std::remove_cvref<decltype(event)>::type::value_type;
  auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
  value.sbeRewind();                                    // note!
  auto exchange_time_utc = std::chrono::nanoseconds{value.transactTime()};
  auto &statistics = shared_.get_statistics();
  auto dispatch = [&]([[maybe_unused]] auto security_id, auto &security) {
    if (std::empty(statistics))
      return;
    auto statistics_update = StatisticsUpdate{
        .stream_id = stream_id,
        .exchange = security.exchange,
        .symbol = security.symbol,
        .statistics = statistics,
        .update_type = UpdateType::INCREMENTAL,
        .exchange_time_utc = exchange_time_utc,
        .exchange_sequence = frame.sequence_number,
        .sending_time_utc = frame.sending_time,
    };
    log::info<3>("statistics_update={}"sv, statistics_update);
    create_trace_and_dispatch(shared_, trace_info, statistics_update, true);
    statistics.clear();
  };
  auto update = [&](auto security_id, auto &security, auto &item) {
    check_report_sequence(security, item, frame);
    callback(statistics, item, security);
    // initializing?
    auto rpt_seq = item.rptSeq();
    if (rpt_seq != 1)
      return;
    auto exchange_sequence = frame.sequence_number;
    auto exchange_time_utc = std::chrono::nanoseconds{value.transactTime()};
    dispatch_market_by_price(trace_info, security_id, security, exchange_sequence, exchange_time_utc, frame.sending_time, {}, {}, true);
    dispatch_market_by_order(trace_info, security_id, security, exchange_sequence, exchange_time_utc, frame.sending_time, {}, true);
  };
  SecurityIterator{shared_}(value.noMDEntries(), dispatch, update);
}

void Incremental::check_report_sequence(tools::Security &security, auto const &value, mdp::Frame const &frame) {
  auto rpt_seq = value.rptSeq();
  /*
  auto diff = static_cast<int64_t>(rpt_seq) - static_cast<int64_t>(security.rpt_seq);
  log::info(
      R"(sequence_number={}, symbol="{}", rpt_seq={}/{}/{})"sv,
      frame.sequence_number,
      security.symbol,
      rpt_seq,
      security.rpt_seq,
      diff);
  */
  if (!security.update_rpt_seq(rpt_seq))
    return;
  log::warn(R"(RESUBSCRIBE exchange="{}", symbol="{}", rpt_seq={})"sv, security.exchange, security.symbol, rpt_seq);
  TraceInfo trace_info;
  security.mbp.sequencer.clear();
  security.mbp.resubscribe = frame.sequence_number;
  dispatch_market_by_price_stale(trace_info, security, frame.sequence_number, std::chrono::nanoseconds{}, frame.sending_time);
  security.mbo.sequencer.clear();
  security.mbo.resubscribe = frame.sequence_number;
  dispatch_market_by_order_stale(trace_info, security, frame.sequence_number, std::chrono::nanoseconds{}, frame.sending_time);
}

void Incremental::on_sequence_reset() {
  log::warn<0>("*** SEQUENCE RESET ***"sv);  // XXX should be log level 1
  // ++counter_.sequence_reset;
  channel_.sequence = {};
  // cache
  cache_.bids.clear();
  cache_.asks.clear();
  cache_.orders.clear();
  cache_.orders_47.clear();
  cache_.security_ids_47.clear();
}

void Incremental::publish_stream_status(TraceInfo const &trace_info, ConnectionStatus connection_status) {
  if (!utils::update(connection_status_, connection_status))
    return;
  auto stream_status = StreamStatus{
      .stream_id = stream_id,
      .account = {},
      .supports = get_supports(shared_.options.enable_market_by_order),
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
