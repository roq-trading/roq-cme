/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/cme/mdp/map.hpp"

using namespace std::literals;

namespace roq {

namespace {
template <typename... Args>
using Helper = detail::MapHelper<Args...>;
}

// deribit => roq

// ::cme::sbe::mdp::AggressorSide::Value => roq::Side

template <>
template <>
constexpr Helper<::cme::sbe::mdp::AggressorSide::Value>::operator std::optional<Side>() const {
  switch (std::get<0>(args_)) {
    using enum ::cme::sbe::mdp::AggressorSide::Value;
    case NoAggressor:
      return Side::UNDEFINED;
    case Buy:
      return Side::BUY;
    case Sell:
      return Side::SELL;
    case NULL_VALUE:
      return Side::UNDEFINED;
  }
  return {};
}

static_assert(Helper{::cme::sbe::mdp::AggressorSide::Value{::cme::sbe::mdp::AggressorSide::NoAggressor}} == roq::Side::UNDEFINED);
static_assert(Helper{::cme::sbe::mdp::AggressorSide::Value{::cme::sbe::mdp::AggressorSide::Buy}} == roq::Side::BUY);
static_assert(Helper{::cme::sbe::mdp::AggressorSide::Value{::cme::sbe::mdp::AggressorSide::Sell}} == roq::Side::SELL);
static_assert(Helper{::cme::sbe::mdp::AggressorSide::Value{::cme::sbe::mdp::AggressorSide::NULL_VALUE}} == roq::Side::UNDEFINED);

template <>
template <>
std::optional<Side> Map<::cme::sbe::mdp::AggressorSide::Value>::helper() const {
  return Helper{args_};
}

// ::cme::sbe::mdp::MDEntryTypeBook::Value => roq::Side

template <>
template <>
constexpr Helper<::cme::sbe::mdp::MDEntryTypeBook::Value>::operator std::optional<Side>() const {
  switch (std::get<0>(args_)) {
    using enum ::cme::sbe::mdp::MDEntryTypeBook::Value;
    case Bid:
      return Side::BUY;
    case Offer:
      return Side::SELL;
    case ImpliedBid:
      return Side::UNDEFINED;
    case ImpliedOffer:
      return Side::UNDEFINED;
    case BookReset:
      return Side::UNDEFINED;
    case MarketBestOffer:
      return Side::UNDEFINED;
    case MarketBestBid:
      return Side::UNDEFINED;
    case NULL_VALUE:
      return Side::UNDEFINED;
  }
  return {};
}

static_assert(Helper{::cme::sbe::mdp::MDEntryTypeBook::Value{::cme::sbe::mdp::MDEntryTypeBook::Bid}} == roq::Side::BUY);
static_assert(Helper{::cme::sbe::mdp::MDEntryTypeBook::Value{::cme::sbe::mdp::MDEntryTypeBook::Offer}} == roq::Side::SELL);
static_assert(Helper{::cme::sbe::mdp::MDEntryTypeBook::Value{::cme::sbe::mdp::MDEntryTypeBook::ImpliedBid}} == roq::Side::UNDEFINED);
static_assert(Helper{::cme::sbe::mdp::MDEntryTypeBook::Value{::cme::sbe::mdp::MDEntryTypeBook::ImpliedOffer}} == roq::Side::UNDEFINED);
static_assert(Helper{::cme::sbe::mdp::MDEntryTypeBook::Value{::cme::sbe::mdp::MDEntryTypeBook::BookReset}} == roq::Side::UNDEFINED);
static_assert(Helper{::cme::sbe::mdp::MDEntryTypeBook::Value{::cme::sbe::mdp::MDEntryTypeBook::MarketBestOffer}} == roq::Side::UNDEFINED);
static_assert(Helper{::cme::sbe::mdp::MDEntryTypeBook::Value{::cme::sbe::mdp::MDEntryTypeBook::MarketBestBid}} == roq::Side::UNDEFINED);
static_assert(Helper{::cme::sbe::mdp::MDEntryTypeBook::Value{::cme::sbe::mdp::MDEntryTypeBook::NULL_VALUE}} == roq::Side::UNDEFINED);

template <>
template <>
std::optional<Side> Map<::cme::sbe::mdp::MDEntryTypeBook::Value>::helper() const {
  return Helper{args_};
}

// ::cme::sbe::mdp::MDEntryTypeDailyStatistics::Value => roq::StatisticsType

template <>
template <>
constexpr Helper<::cme::sbe::mdp::MDEntryTypeDailyStatistics::Value>::operator std::optional<StatisticsType>() const {
  switch (std::get<0>(args_)) {
    using enum ::cme::sbe::mdp::MDEntryTypeDailyStatistics::Value;
    case SettlementPrice:
      return StatisticsType::SETTLEMENT_PRICE;
    case ClearedVolume:
      return StatisticsType::UNDEFINED;
    case OpenInterest:
      return StatisticsType::OPEN_INTEREST;
    case FixingPrice:
      return StatisticsType::CLOSE_PRICE;
    case NULL_VALUE:
      return StatisticsType::UNDEFINED;
  }
  return {};
}

static_assert(
    Helper{::cme::sbe::mdp::MDEntryTypeDailyStatistics::Value{::cme::sbe::mdp::MDEntryTypeDailyStatistics::SettlementPrice}} ==
    roq::StatisticsType::SETTLEMENT_PRICE);
static_assert(
    Helper{::cme::sbe::mdp::MDEntryTypeDailyStatistics::Value{::cme::sbe::mdp::MDEntryTypeDailyStatistics::ClearedVolume}} == roq::StatisticsType::UNDEFINED);
static_assert(
    Helper{::cme::sbe::mdp::MDEntryTypeDailyStatistics::Value{::cme::sbe::mdp::MDEntryTypeDailyStatistics::OpenInterest}} ==
    roq::StatisticsType::OPEN_INTEREST);
static_assert(
    Helper{::cme::sbe::mdp::MDEntryTypeDailyStatistics::Value{::cme::sbe::mdp::MDEntryTypeDailyStatistics::FixingPrice}} == roq::StatisticsType::CLOSE_PRICE);
static_assert(
    Helper{::cme::sbe::mdp::MDEntryTypeDailyStatistics::Value{::cme::sbe::mdp::MDEntryTypeDailyStatistics::NULL_VALUE}} == roq::StatisticsType::UNDEFINED);

template <>
template <>
std::optional<StatisticsType> Map<::cme::sbe::mdp::MDEntryTypeDailyStatistics::Value>::helper() const {
  return Helper{args_};
}

// ::cme::sbe::mdp::MDEntryTypeStatistics::Value => roq::StatisticsType

template <>
template <>
constexpr Helper<::cme::sbe::mdp::MDEntryTypeStatistics::Value>::operator std::optional<StatisticsType>() const {
  switch (std::get<0>(args_)) {
    using enum ::cme::sbe::mdp::MDEntryTypeStatistics::Value;
    case OpenPrice:
      return StatisticsType::OPEN_PRICE;
    case HighTrade:
      return StatisticsType::HIGHEST_TRADED_PRICE;
    case LowTrade:
      return StatisticsType::LOWEST_TRADED_PRICE;
    case VWAP:
      return StatisticsType::UNDEFINED;
    case HighestBid:
      return StatisticsType::UNDEFINED;
    case LowestOffer:
      return StatisticsType::UNDEFINED;
    case NULL_VALUE:
      return StatisticsType::UNDEFINED;
  }
  return {};
}

static_assert(Helper{::cme::sbe::mdp::MDEntryTypeStatistics::Value{::cme::sbe::mdp::MDEntryTypeStatistics::OpenPrice}} == roq::StatisticsType::OPEN_PRICE);
static_assert(
    Helper{::cme::sbe::mdp::MDEntryTypeStatistics::Value{::cme::sbe::mdp::MDEntryTypeStatistics::HighTrade}} == roq::StatisticsType::HIGHEST_TRADED_PRICE);
static_assert(
    Helper{::cme::sbe::mdp::MDEntryTypeStatistics::Value{::cme::sbe::mdp::MDEntryTypeStatistics::LowTrade}} == roq::StatisticsType::LOWEST_TRADED_PRICE);
static_assert(Helper{::cme::sbe::mdp::MDEntryTypeStatistics::Value{::cme::sbe::mdp::MDEntryTypeStatistics::VWAP}} == roq::StatisticsType::UNDEFINED);
static_assert(Helper{::cme::sbe::mdp::MDEntryTypeStatistics::Value{::cme::sbe::mdp::MDEntryTypeStatistics::HighestBid}} == roq::StatisticsType::UNDEFINED);
static_assert(Helper{::cme::sbe::mdp::MDEntryTypeStatistics::Value{::cme::sbe::mdp::MDEntryTypeStatistics::LowestOffer}} == roq::StatisticsType::UNDEFINED);
static_assert(Helper{::cme::sbe::mdp::MDEntryTypeStatistics::Value{::cme::sbe::mdp::MDEntryTypeStatistics::NULL_VALUE}} == roq::StatisticsType::UNDEFINED);

template <>
template <>
std::optional<StatisticsType> Map<::cme::sbe::mdp::MDEntryTypeStatistics::Value>::helper() const {
  return Helper{args_};
}

// ::cme::sbe::mdp::MDUpdateAction::Value => roq::UpdateAction

template <>
template <>
constexpr Helper<::cme::sbe::mdp::MDUpdateAction::Value>::operator std::optional<UpdateAction>() const {
  switch (std::get<0>(args_)) {
    using enum ::cme::sbe::mdp::MDUpdateAction::Value;
    case New:
      return UpdateAction::NEW;
    case Change:
      return UpdateAction::CHANGE;
    case Delete:
      return UpdateAction::DELETE;
    case DeleteThru:
      return UpdateAction::UNDEFINED;
    case DeleteFrom:
      return UpdateAction::UNDEFINED;
    case Overlay:
      return UpdateAction::UNDEFINED;
    case NULL_VALUE:
      return UpdateAction::UNDEFINED;
  }
  return {};
}

static_assert(Helper{::cme::sbe::mdp::MDUpdateAction::Value{::cme::sbe::mdp::MDUpdateAction::New}} == roq::UpdateAction::NEW);
static_assert(Helper{::cme::sbe::mdp::MDUpdateAction::Value{::cme::sbe::mdp::MDUpdateAction::Change}} == roq::UpdateAction::CHANGE);
static_assert(Helper{::cme::sbe::mdp::MDUpdateAction::Value{::cme::sbe::mdp::MDUpdateAction::Delete}} == roq::UpdateAction::DELETE);
static_assert(Helper{::cme::sbe::mdp::MDUpdateAction::Value{::cme::sbe::mdp::MDUpdateAction::DeleteThru}} == roq::UpdateAction::UNDEFINED);
static_assert(Helper{::cme::sbe::mdp::MDUpdateAction::Value{::cme::sbe::mdp::MDUpdateAction::DeleteFrom}} == roq::UpdateAction::UNDEFINED);
static_assert(Helper{::cme::sbe::mdp::MDUpdateAction::Value{::cme::sbe::mdp::MDUpdateAction::Overlay}} == roq::UpdateAction::UNDEFINED);
static_assert(Helper{::cme::sbe::mdp::MDUpdateAction::Value{::cme::sbe::mdp::MDUpdateAction::NULL_VALUE}} == roq::UpdateAction::UNDEFINED);

template <>
template <>
std::optional<UpdateAction> Map<::cme::sbe::mdp::MDUpdateAction::Value>::helper() const {
  return Helper{args_};
}

// ::cme::sbe::mdp::OrderUpdateAction::Value => roq::UpdateAction

template <>
template <>
constexpr Helper<::cme::sbe::mdp::OrderUpdateAction::Value>::operator std::optional<UpdateAction>() const {
  switch (std::get<0>(args_)) {
    using enum ::cme::sbe::mdp::OrderUpdateAction::Value;
    case New:
      return UpdateAction::NEW;
    case Update:
      return UpdateAction::CHANGE;
    case Delete:
      return UpdateAction::DELETE;
    case NULL_VALUE:
      return UpdateAction::UNDEFINED;
  }
  return {};
}

static_assert(Helper{::cme::sbe::mdp::OrderUpdateAction::Value{::cme::sbe::mdp::OrderUpdateAction::New}} == roq::UpdateAction::NEW);
static_assert(Helper{::cme::sbe::mdp::OrderUpdateAction::Value{::cme::sbe::mdp::OrderUpdateAction::Update}} == roq::UpdateAction::CHANGE);
static_assert(Helper{::cme::sbe::mdp::OrderUpdateAction::Value{::cme::sbe::mdp::OrderUpdateAction::Delete}} == roq::UpdateAction::DELETE);
static_assert(Helper{::cme::sbe::mdp::OrderUpdateAction::Value{::cme::sbe::mdp::OrderUpdateAction::NULL_VALUE}} == roq::UpdateAction::UNDEFINED);

template <>
template <>
std::optional<UpdateAction> Map<::cme::sbe::mdp::OrderUpdateAction::Value>::helper() const {
  return Helper{args_};
}

// ::cme::sbe::mdp::SecurityTradingStatus::Value => roq::TradingStatus

/*
START_OF_DAY,         //!< No matching, no order actions
PRE_OPEN,             //!< No matching, all order actions
PRE_OPEN_NO_CANCEL,   //!< No matching, only new orders
PRE_OPEN_FREEZE,      //!< Matching, no order actions
OPEN,                 //!< Matching, all order actions
FAST_MARKET,          //!< Same as Open, some settings could be relaxed by the exchange
HALT,                 //!< No matching, only order cancellation
CLOSE_NOT_FINAL,      //!< Same as Close, state required to support mid-session PreOpen
PRE_CLOSE,            //!< No matching, all order actions
PRE_CLOSE_NO_CANCEL,  //!< No matching, only new orders
PRE_CLOSE_FREEZE,     //!< Matching, no order actions
CLOSE,                //!< No matching, no order actions, good-for-day orders automatically canceled
POST_CLOSE,           //!< No matching, all order actions (only with next-trading-day validity)
END_OF_DAY,           //!< No matching, no order actions
*/

template <>
template <>
constexpr Helper<::cme::sbe::mdp::SecurityTradingStatus::Value>::operator std::optional<TradingStatus>() const {
  switch (std::get<0>(args_)) {
    using enum ::cme::sbe::mdp::SecurityTradingStatus::Value;
    case TradingHalt:
      return TradingStatus::HALT;
    case Close:
      return TradingStatus::CLOSE;
    case NewPriceIndication:  // ???
      return TradingStatus::UNDEFINED;
    case ReadyToTrade:
      return TradingStatus::OPEN;
    case NotAvailableForTrading:
      return TradingStatus::CLOSE;  // ???
    case UnknownorInvalid:
      return TradingStatus::UNDEFINED;
    case PreOpen:
      return TradingStatus::PRE_OPEN;
    case PreCross:
      return TradingStatus::UNDEFINED;
    case Cross:
      return TradingStatus::UNDEFINED;
    case PostClose:
      return TradingStatus::POST_CLOSE;
    case NoChange:
      return TradingStatus::START_OF_DAY;  // ???
    case PrivateWorkup:
      return TradingStatus::UNDEFINED;
    case PublicWorkup:
      return TradingStatus::UNDEFINED;
    case NULL_VALUE:
      return TradingStatus::UNDEFINED;
  }
  return {};
}

static_assert(Helper{::cme::sbe::mdp::SecurityTradingStatus::Value{::cme::sbe::mdp::SecurityTradingStatus::TradingHalt}} == roq::TradingStatus::HALT);
static_assert(Helper{::cme::sbe::mdp::SecurityTradingStatus::Value{::cme::sbe::mdp::SecurityTradingStatus::Close}} == roq::TradingStatus::CLOSE);
static_assert(
    Helper{::cme::sbe::mdp::SecurityTradingStatus::Value{::cme::sbe::mdp::SecurityTradingStatus::NewPriceIndication}} == roq::TradingStatus::UNDEFINED);
static_assert(Helper{::cme::sbe::mdp::SecurityTradingStatus::Value{::cme::sbe::mdp::SecurityTradingStatus::ReadyToTrade}} == roq::TradingStatus::OPEN);
static_assert(
    Helper{::cme::sbe::mdp::SecurityTradingStatus::Value{::cme::sbe::mdp::SecurityTradingStatus::NotAvailableForTrading}} == roq::TradingStatus::CLOSE);
static_assert(Helper{::cme::sbe::mdp::SecurityTradingStatus::Value{::cme::sbe::mdp::SecurityTradingStatus::UnknownorInvalid}} == roq::TradingStatus::UNDEFINED);
static_assert(Helper{::cme::sbe::mdp::SecurityTradingStatus::Value{::cme::sbe::mdp::SecurityTradingStatus::PreOpen}} == roq::TradingStatus::PRE_OPEN);
static_assert(Helper{::cme::sbe::mdp::SecurityTradingStatus::Value{::cme::sbe::mdp::SecurityTradingStatus::PreCross}} == roq::TradingStatus::UNDEFINED);
static_assert(Helper{::cme::sbe::mdp::SecurityTradingStatus::Value{::cme::sbe::mdp::SecurityTradingStatus::Cross}} == roq::TradingStatus::UNDEFINED);
static_assert(Helper{::cme::sbe::mdp::SecurityTradingStatus::Value{::cme::sbe::mdp::SecurityTradingStatus::PostClose}} == roq::TradingStatus::POST_CLOSE);
static_assert(Helper{::cme::sbe::mdp::SecurityTradingStatus::Value{::cme::sbe::mdp::SecurityTradingStatus::NoChange}} == roq::TradingStatus::START_OF_DAY);
static_assert(Helper{::cme::sbe::mdp::SecurityTradingStatus::Value{::cme::sbe::mdp::SecurityTradingStatus::PrivateWorkup}} == roq::TradingStatus::UNDEFINED);
static_assert(Helper{::cme::sbe::mdp::SecurityTradingStatus::Value{::cme::sbe::mdp::SecurityTradingStatus::PublicWorkup}} == roq::TradingStatus::UNDEFINED);
static_assert(Helper{::cme::sbe::mdp::SecurityTradingStatus::Value{::cme::sbe::mdp::SecurityTradingStatus::NULL_VALUE}} == roq::TradingStatus::UNDEFINED);

template <>
template <>
std::optional<TradingStatus> Map<::cme::sbe::mdp::SecurityTradingStatus::Value>::helper() const {
  return Helper{args_};
}

// ::cme::sbe::mdp::Side::Value => roq::Side

template <>
template <>
constexpr Helper<::cme::sbe::mdp::Side::Value>::operator std::optional<Side>() const {
  switch (std::get<0>(args_)) {
    using enum ::cme::sbe::mdp::Side::Value;
    case Buy:
      return Side::BUY;
    case Sell:
      return Side::SELL;
    case NULL_VALUE:
      return Side::UNDEFINED;
  }
  return {};
}

static_assert(Helper{::cme::sbe::mdp::Side::Value{::cme::sbe::mdp::Side::Buy}} == roq::Side::BUY);
static_assert(Helper{::cme::sbe::mdp::Side::Value{::cme::sbe::mdp::Side::Sell}} == roq::Side::SELL);
static_assert(Helper{::cme::sbe::mdp::Side::Value{::cme::sbe::mdp::Side::NULL_VALUE}} == roq::Side::UNDEFINED);

template <>
template <>
std::optional<Side> Map<::cme::sbe::mdp::Side::Value>::helper() const {
  return Helper{args_};
}

}  // namespace roq
