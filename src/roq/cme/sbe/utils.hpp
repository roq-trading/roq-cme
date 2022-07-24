/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <fmt/chrono.h>
#include <fmt/format.h>

#include <deribit_multicast/BookChange.h>
#include <deribit_multicast/BookSide.h>
#include <deribit_multicast/Direction.h>
#include <deribit_multicast/InstrumentState.h>
#include <deribit_multicast/Liquidation.h>
#include <deribit_multicast/YesNo.h>

#include <deribit_multicast/Book.h>
#include <deribit_multicast/Instrument.h>
#include <deribit_multicast/Snapshot.h>
#include <deribit_multicast/Ticker.h>
#include <deribit_multicast/Trades.h>

#include "roq/api.hpp"

#include "roq/core/sbe/iterator.hpp"

#include "roq/logging.hpp"

namespace roq {
namespace deribit {
namespace sbe {

inline Side map_book_side(deribit_multicast::BookSide::Value value) {
  switch (value) {
    using enum deribit_multicast::BookSide::Value;
    case ask:
      return Side::SELL;
    case bid:
      return Side::BUY;
    case NULL_VALUE:
      return Side::UNDEFINED;
  }
  return Side::UNDEFINED;
}

inline Side map_direction(deribit_multicast::Direction::Value value) {
  switch (value) {
    using enum deribit_multicast::Direction::Value;
    case buy:
      return Side::BUY;
    case sell:
      return Side::SELL;
    case NULL_VALUE:
      return Side::UNDEFINED;
  }
  return Side::UNDEFINED;
}

inline TradingStatus map_instrument_state(deribit_multicast::InstrumentState::Value value) {
  switch (value) {
    using enum deribit_multicast::InstrumentState::Value;
    case created:
      return TradingStatus::OPEN;  // ???
    case open:
      return TradingStatus::OPEN;
    case closed:
      return TradingStatus::CLOSE;
    case settled:
      break;  // ???
    case NULL_VALUE:
      return TradingStatus::UNDEFINED;
  }
  return TradingStatus::UNDEFINED;
}

inline Liquidity map_liquidation(deribit_multicast::Liquidation::Value value) {
  switch (value) {
    using enum deribit_multicast::Liquidation::Value;
    case none:
      return Liquidity::UNDEFINED;
    case maker:
      return Liquidity::MAKER;
    case taker:
      return Liquidity::TAKER;
    case both:
      return Liquidity::UNDEFINED;  // ???
    case NULL_VALUE:
      return Liquidity::UNDEFINED;
  }
  return Liquidity::UNDEFINED;
}

inline bool map_yes_no(deribit_multicast::YesNo::Value value) {
  switch (value) {
    using enum deribit_multicast::YesNo::Value;
    case no:
      return false;
    case yes:
      return true;
    case NULL_VALUE:
      return false;
  }
  return false;
}

template <typename T>
size_t compute_length(T &);

template <>
inline size_t compute_length(deribit_multicast::MessageHeader &value) {
  return value.encodedLength();
}

template <>
inline size_t compute_length(deribit_multicast::Instrument &value) {
  auto instrument_name_length = value.instrumentNameLength();
  return value.computeLength(instrument_name_length);
}

template <>
inline size_t compute_length(deribit_multicast::Book &value) {
  auto changes_list_length = value.changesList().count();
  return value.computeLength(changes_list_length);
  /*
  value.sbeRewind();  // wtf!
  value.levelsList().forEach([](auto &e) { e.skip(); });
  */
}

template <>
inline size_t compute_length(deribit_multicast::Trades &value) {
  auto trades_list_length = value.tradesList().count();
  return value.computeLength(trades_list_length);
}

template <>
inline size_t compute_length(deribit_multicast::Ticker &value) {
  return value.computeLength();
}

template <>
inline size_t compute_length(deribit_multicast::Snapshot &value) {
  auto levels_list_length = value.levelsList().count();
  value.sbeRewind();  // wtf!
  value.levelsList().forEach([](auto &e) { e.skip(); });
  return value.computeLength(levels_list_length);
}

// this is just so... wtf!

template <typename T>
std::string get_instrument_name(T &);

template <>
inline std::string get_instrument_name(deribit_multicast::Instrument &value) {
  value.sbeRewind();
  auto length = value.instrumentNameLength();  // must fetch before getting name
  return {value.instrumentName(), length};
}

}  // namespace sbe
}  // namespace deribit
}  // namespace roq

// header

template <>
struct fmt::formatter<deribit_multicast::MessageHeader> {
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(deribit_multicast::MessageHeader const &value, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(blockLength={}, )"
        R"(templateId={}, )"
        R"(schemaId={}, )"
        R"(version={}, )"
        R"(numGroups={}, )"
        R"(numVarDataFields={})"
        R"(}})"sv,
        value.blockLength(),
        value.templateId(),
        value.schemaId(),
        value.version(),
        value.numGroups(),
        value.numVarDataFields());
  }
};

// helper

template <>
struct fmt::formatter<deribit_multicast::Book::ChangesList> {
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(deribit_multicast::Book::ChangesList const &value, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(side={}, )"
        R"(change={}, )"
        R"(price={}, )"
        R"(amount={})"
        R"(}})"sv,
        deribit_multicast::BookSide::c_str(value.side()),
        deribit_multicast::BookChange::c_str(value.change()),
        value.price(),
        value.amount());
  }
};

template <>
struct fmt::formatter<deribit_multicast::Trades::TradesList> {
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(deribit_multicast::Trades::TradesList const &value, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(direction={}, )"
        R"(price={}, )"
        R"(amount={}, )"
        R"(timestampMs={}, )"
        R"(markPrice={}, )"
        R"(indexPrice={}, )"
        R"(tradeSeq={}, )"
        R"(tradeId={}, )"
        R"(tickDirection={}, )"
        R"(liquidation={}, )"
        R"(iv={}, )"
        R"(blockTradeId={}, )"
        R"(comboTradeId={})"
        R"(}})"sv,
        deribit_multicast::Direction::c_str(value.direction()),
        value.price(),
        value.amount(),
        std::chrono::milliseconds{value.timestampMs()},
        value.markPrice(),
        value.indexPrice(),
        value.tradeSeq(),
        value.tradeId(),
        deribit_multicast::TickDirection::c_str(value.tickDirection()),
        deribit_multicast::Liquidation::c_str(value.liquidation()),
        value.iv(),
        value.blockTradeId(),
        value.comboTradeId());
  }
};

template <>
struct fmt::formatter<deribit_multicast::Snapshot::LevelsList> {
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(deribit_multicast::Snapshot::LevelsList const &value, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(side={}, )"
        R"(price={}, )"
        R"(amount={})"
        R"(}})"sv,
        deribit_multicast::BookSide::c_str(value.side()),
        value.price(),
        value.amount());
  }
};

// messages
//
// note! some nested objects (lists) imply non-const due to positional information

template <>
struct fmt::formatter<deribit_multicast::Book> {
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(deribit_multicast::Book &value, Context &context) const {
    using namespace std::literals;
    value.sbeRewind();
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(header={}, )"
        R"(instrumentId={}, )"
        R"(timestampMs={}, )"
        R"(prevChangeId={}, )"
        R"(changeId={}, )"
        R"(isLast={}, )"
        R"(changesList=[{}])"
        R"(}})"sv,
        value.header(),
        value.instrumentId(),
        std::chrono::milliseconds{value.timestampMs()},
        value.prevChangeId(),
        value.changeId(),
        roq::deribit::sbe::map_yes_no(value.isLast()),
        fmt::join(roq::core::sbe::iterator{value.changesList()}, roq::core::sbe::sentinel{}, ", "sv));
  }
};

template <>
struct fmt::formatter<deribit_multicast::Instrument> {
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(deribit_multicast::Instrument &value, Context &context) const {
    using namespace std::literals;
    auto instrument_name = roq::deribit::sbe::get_instrument_name(value);
    value.sbeRewind();
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(header={}, )"
        R"(instrumentId={}, )"
        R"(instrumentState={}, )"
        R"(kind={}, )"
        R"(futureType={}, )"
        R"(optionType={}, )"
        R"(rfq={}, )"
        R"(settlementPeriod={}, )"
        R"(settlementPeriodCount={}, )"
        R"(baseCurrency="{}", )"
        R"(quoteCurrency="{}", )"
        R"(counterCurrency="{}", )"
        R"(settlementCurrency="{}", )"
        R"(sizeCurrency="{}", )"
        R"(creationTimestampMs={}, )"
        R"(expirationTimestampMs={}, )"
        R"(strikePrice={}, )"
        R"(contractSize={}, )"
        R"(minTradeAmount={}, )"
        R"(tickSize={}, )"
        R"(makerCommission={}, )"
        R"(takerCommission={}, )"
        R"(blockTradeCommission={}, )"
        R"(maxLiquidationCommission={}, )"
        R"(maxLeverage={}, )"
        R"(instrumentName="{}")"
        R"(}})"sv,
        value.header(),
        value.instrumentId(),
        deribit_multicast::InstrumentState::c_str(value.instrumentState()),
        deribit_multicast::InstrumentKind::c_str(value.kind()),
        deribit_multicast::FutureType::c_str(value.futureType()),
        deribit_multicast::OptionType::c_str(value.optionType()),
        roq::deribit::sbe::map_yes_no(value.rfq()),
        deribit_multicast::Period::c_str(value.settlementPeriod()),
        value.settlementPeriodCount(),
        value.baseCurrency(),
        value.quoteCurrency(),
        value.counterCurrency(),
        value.settlementCurrency(),
        value.sizeCurrency(),
        std::chrono::milliseconds{value.creationTimestampMs()},
        std::chrono::milliseconds{value.expirationTimestampMs()},
        value.strikePrice(),
        value.contractSize(),
        value.minTradeAmount(),
        value.tickSize(),
        value.makerCommission(),
        value.takerCommission(),
        value.blockTradeCommission(),
        value.maxLiquidationCommission(),
        value.maxLeverage(),
        instrument_name);
  }
};

template <>
struct fmt::formatter<deribit_multicast::Ticker> {
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(deribit_multicast::Ticker &value, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(header={}, )"
        R"(instrumentId={}, )"
        R"(instrumentState={}, )"
        R"(timestampMs={}, )"
        R"(openInterest={}, )"
        R"(minSellPrice={}, )"
        R"(maxBuyPrice={}, )"
        R"(lastPrice={}, )"
        R"(indexPrice={}, )"
        R"(markPrice={}, )"
        R"(bestBidPrice={}, )"
        R"(bestBidAmount={}, )"
        R"(bestAskPrice={}, )"
        R"(bestAskAmount={}, )"
        R"(currentFunding={}, )"
        R"(funding8h={}, )"
        R"(estimatedDeliveryPrice={}, )"
        R"(deliveryPrice={}, )"
        R"(settlementPrice={})"
        R"(}})"sv,
        value.header(),
        value.instrumentId(),
        deribit_multicast::InstrumentState::c_str(value.instrumentState()),
        std::chrono::milliseconds{value.timestampMs()},
        value.openInterest(),
        value.minSellPrice(),
        value.maxBuyPrice(),
        value.lastPrice(),
        value.indexPrice(),
        value.markPrice(),
        value.bestBidPrice(),
        value.bestBidAmount(),
        value.bestAskPrice(),
        value.bestAskAmount(),
        value.currentFunding(),
        value.funding8h(),
        value.estimatedDeliveryPrice(),
        value.deliveryPrice(),
        value.settlementPrice());
  }
};

template <>
struct fmt::formatter<deribit_multicast::Snapshot> {
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(deribit_multicast::Snapshot &value, Context &context) const {
    using namespace std::literals;
    value.sbeRewind();
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(header={}, )"
        R"(instrumentId={}, )"
        R"(timestampMs={}, )"
        R"(changeId={}, )"
        R"(isBookComplete={}, )"
        R"(isLastInBook={}, )"
        R"(levelsList=[{}])"
        R"(}})"sv,
        value.header(),
        value.instrumentId(),
        std::chrono::milliseconds{value.timestampMs()},
        value.changeId(),
        roq::deribit::sbe::map_yes_no(value.isBookComplete()),
        roq::deribit::sbe::map_yes_no(value.isLastInBook()),
        fmt::join(roq::core::sbe::iterator{value.levelsList()}, roq::core::sbe::sentinel{}, ", "sv));
  }
};

template <>
struct fmt::formatter<deribit_multicast::Trades> {
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(deribit_multicast::Trades &value, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(header={}, )"
        R"(instrumentId={}, )"
        R"(tradesList=[{}])"
        R"(}})"sv,
        value.header(),
        value.instrumentId(),
        fmt::join(roq::core::sbe::iterator{value.tradesList()}, roq::core::sbe::sentinel{}, ", "sv));
  }
};
