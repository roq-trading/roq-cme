/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/cme/protocol/ilink/map.hpp"

#include <algorithm>

using namespace std::literals;

namespace roq {

namespace {
template <typename... Args>
using Helper = detail::MapHelper<Args...>;
}

// cme ==> roq

// cme::sbe::ilink::OrderStatus::Value ==> roq::OrderStatus

template <>
template <>
constexpr Helper<cme::sbe::ilink::OrderStatus::Value>::operator std::optional<roq::OrderStatus>() const {
  switch (std::get<0>(args_)) {
    using enum cme::sbe::ilink::OrderStatus::Value;
    case New:
      return OrderStatus::WORKING;  // ???
    case PartiallyFilled:
      return OrderStatus::WORKING;
    case Filled:
      return OrderStatus::COMPLETED;
    case Cancelled:
      return OrderStatus::CANCELED;
    case Replaced:
      return OrderStatus::WORKING;
    case PendingCancel:
      return OrderStatus::WORKING;
    case Rejected:
      return OrderStatus::REJECTED;
    case Expired:
      return OrderStatus::EXPIRED;
    case PendingReplace:
      return OrderStatus::ACCEPTED;
    case Undefined:
    case NULL_VALUE:
      return roq::OrderStatus::UNDEFINED;
  }
  return {};
}

static_assert(Helper{cme::sbe::ilink::OrderStatus::NULL_VALUE} == roq::OrderStatus::UNDEFINED);
static_assert(Helper{cme::sbe::ilink::OrderStatus::New} == roq::OrderStatus::WORKING);
static_assert(Helper{cme::sbe::ilink::OrderStatus::PartiallyFilled} == roq::OrderStatus::WORKING);
static_assert(Helper{cme::sbe::ilink::OrderStatus::Filled} == roq::OrderStatus::COMPLETED);
static_assert(Helper{cme::sbe::ilink::OrderStatus::Cancelled} == roq::OrderStatus::CANCELED);
static_assert(Helper{cme::sbe::ilink::OrderStatus::Replaced} == roq::OrderStatus::WORKING);
static_assert(Helper{cme::sbe::ilink::OrderStatus::PendingCancel} == roq::OrderStatus::WORKING);
static_assert(Helper{cme::sbe::ilink::OrderStatus::Rejected} == roq::OrderStatus::REJECTED);
static_assert(Helper{cme::sbe::ilink::OrderStatus::Expired} == roq::OrderStatus::EXPIRED);
static_assert(Helper{cme::sbe::ilink::OrderStatus::PendingReplace} == roq::OrderStatus::ACCEPTED);
static_assert(Helper{cme::sbe::ilink::OrderStatus::Undefined} == roq::OrderStatus::UNDEFINED);
static_assert(Helper{cme::sbe::ilink::OrderStatus::NULL_VALUE} == roq::OrderStatus::UNDEFINED);

template <>
template <>
std::optional<roq::OrderStatus> Map<cme::sbe::ilink::OrderStatus::Value>::helper() const {
  return Helper{args_};
}

// cme::sbe::ilink::OrdStatusTrd::Value ==> roq::OrderStatus

template <>
template <>
constexpr Helper<cme::sbe::ilink::OrdStatusTrd::Value>::operator std::optional<roq::OrderStatus>() const {
  switch (std::get<0>(args_)) {
    using enum cme::sbe::ilink::OrdStatusTrd::Value;
    case PartiallyFilled:
      return OrderStatus::WORKING;
    case Filled:
      return OrderStatus::COMPLETED;
    case NULL_VALUE:
      return roq::OrderStatus::UNDEFINED;
  }
  return {};
}

static_assert(Helper{cme::sbe::ilink::OrdStatusTrd::NULL_VALUE} == roq::OrderStatus::UNDEFINED);
static_assert(Helper{cme::sbe::ilink::OrdStatusTrd::PartiallyFilled} == roq::OrderStatus::WORKING);
static_assert(Helper{cme::sbe::ilink::OrdStatusTrd::Filled} == roq::OrderStatus::COMPLETED);
static_assert(Helper{cme::sbe::ilink::OrdStatusTrd::NULL_VALUE} == roq::OrderStatus::UNDEFINED);

template <>
template <>
std::optional<roq::OrderStatus> Map<cme::sbe::ilink::OrdStatusTrd::Value>::helper() const {
  return Helper{args_};
}

// cme::sbe::ilink::OrderType::Value ==> roq::OrderType

template <>
template <>
constexpr Helper<cme::sbe::ilink::OrderType::Value>::operator std::optional<roq::OrderType>() const {
  switch (std::get<0>(args_)) {
    using enum cme::sbe::ilink::OrderType::Value;
    case MarketWithProtection:
      return roq::OrderType::MARKET;
    case Limit:
      return roq::OrderType::LIMIT;
    case StopLimit:
      return roq::OrderType::LIMIT;
    case MarketWithLeftoverAsLimit:
      return roq::OrderType::MARKET;  // ???
    case NULL_VALUE:
      return roq::OrderType::UNDEFINED;
  }
  return {};
}

static_assert(Helper{cme::sbe::ilink::OrderType::NULL_VALUE} == roq::OrderType::UNDEFINED);
static_assert(Helper{cme::sbe::ilink::OrderType::MarketWithProtection} == roq::OrderType::MARKET);
static_assert(Helper{cme::sbe::ilink::OrderType::Limit} == roq::OrderType::LIMIT);
static_assert(Helper{cme::sbe::ilink::OrderType::StopLimit} == roq::OrderType::LIMIT);
static_assert(Helper{cme::sbe::ilink::OrderType::MarketWithLeftoverAsLimit} == roq::OrderType::MARKET);

template <>
template <>
std::optional<roq::OrderType> Map<cme::sbe::ilink::OrderType::Value>::helper() const {
  return Helper{args_};
}

// cme::sbe::ilink::OrderTypeReq::Value ==> roq::OrderType

template <>
template <>
constexpr Helper<cme::sbe::ilink::OrderTypeReq::Value>::operator std::optional<roq::OrderType>() const {
  switch (std::get<0>(args_)) {
    using enum cme::sbe::ilink::OrderTypeReq::Value;
    case MarketwithProtection:
      return roq::OrderType::MARKET;
    case Limit:
      return roq::OrderType::LIMIT;
    case MarketWithLeftoverAsLimit:
    case NULL_VALUE:
      return roq::OrderType::UNDEFINED;
  }
  return {};
}

static_assert(Helper{cme::sbe::ilink::OrderTypeReq::NULL_VALUE} == roq::OrderType::UNDEFINED);
static_assert(Helper{cme::sbe::ilink::OrderTypeReq::MarketwithProtection} == roq::OrderType::MARKET);
static_assert(Helper{cme::sbe::ilink::OrderTypeReq::Limit} == roq::OrderType::LIMIT);

template <>
template <>
std::optional<roq::OrderType> Map<cme::sbe::ilink::OrderTypeReq::Value>::helper() const {
  return Helper{args_};
}

// cme::sbe::ilink::SideReq::Value ==> roq::Side

template <>
template <>
constexpr Helper<cme::sbe::ilink::SideReq::Value>::operator std::optional<roq::Side>() const {
  switch (std::get<0>(args_)) {
    using enum cme::sbe::ilink::SideReq::Value;
    case Buy:
      return roq::Side::BUY;
    case Sell:
      return roq::Side::SELL;
    case Undisclosed:
    case NULL_VALUE:
      return roq::Side::UNDEFINED;
  }
  return {};
}

static_assert(Helper{cme::sbe::ilink::SideReq::NULL_VALUE} == roq::Side::UNDEFINED);
static_assert(Helper{cme::sbe::ilink::SideReq::Buy} == roq::Side::BUY);
static_assert(Helper{cme::sbe::ilink::SideReq::Sell} == roq::Side::SELL);

template <>
template <>
std::optional<roq::Side> Map<cme::sbe::ilink::SideReq::Value>::helper() const {
  return Helper{args_};
}

// cme::sbe::ilink::TimeInForce::Value ==> roq::TimeInForce

template <>
template <>
constexpr Helper<cme::sbe::ilink::TimeInForce::Value>::operator std::optional<roq::TimeInForce>() const {
  switch (std::get<0>(args_)) {
    using enum cme::sbe::ilink::TimeInForce::Value;
    case Day:
      return TimeInForce::GFD;
    case GoodTillCancel:
      return TimeInForce::GTC;
    case FillAndKill:
      return TimeInForce::IOC;
    case FillOrKill:
      return TimeInForce::FOK;
    case GoodTillDate:
      return TimeInForce::GTD;
    case GoodForSession:
      return roq::TimeInForce::UNDEFINED;  // ???
    case NULL_VALUE:
      return roq::TimeInForce::UNDEFINED;
  }
  return {};
}

static_assert(Helper{cme::sbe::ilink::TimeInForce::NULL_VALUE} == roq::TimeInForce::UNDEFINED);
static_assert(Helper{cme::sbe::ilink::TimeInForce::Day} == roq::TimeInForce::GFD);
static_assert(Helper{cme::sbe::ilink::TimeInForce::GoodTillCancel} == roq::TimeInForce::GTC);
static_assert(Helper{cme::sbe::ilink::TimeInForce::FillAndKill} == roq::TimeInForce::IOC);
static_assert(Helper{cme::sbe::ilink::TimeInForce::FillOrKill} == roq::TimeInForce::FOK);
static_assert(Helper{cme::sbe::ilink::TimeInForce::GoodTillDate} == roq::TimeInForce::GTD);
static_assert(Helper{cme::sbe::ilink::TimeInForce::GoodForSession} == roq::TimeInForce::UNDEFINED);
static_assert(Helper{cme::sbe::ilink::TimeInForce::NULL_VALUE} == roq::TimeInForce::UNDEFINED);

template <>
template <>
std::optional<roq::TimeInForce> Map<cme::sbe::ilink::TimeInForce::Value>::helper() const {
  return Helper{args_};
}

// roq ==> cme

// Mask<roq::ExecutionInstruction> ==> uint8_t

template <>
template <>
constexpr Helper<roq::Mask<roq::ExecutionInstruction>>::operator std::optional<cme::sbe::ilink::ExecInst>() const {
  cme::sbe::ilink::ExecInst result;
  auto value = std::get<0>(args_);
  // if (value.has(ExecutionInstruction::PARTICIPATE_DO_NOT_INITIATE)) { }
  if (value.has(ExecutionInstruction::CANCEL_IF_NOT_BEST)) {
    result.oB(true);
  }
  // if (value.has(ExecutionInstruction::DO_NOT_INCREASE)) { }
  // if (value.has(ExecutionInstruction::DO_NOT_REDUCE)) { }
  return result;
}

// XXX FIXME TODO sbe doesn't do constexpr...
/// static_assert(Helper{Mask<ExecutionInstruction>{}} == cme::sbe::ilink::ExecInst{}.rawValue(0));
/// static_assert(Helper{Mask{ExecutionInstruction::CANCEL_IF_NOT_BEST}} == cme::sbe::ilink::ExecInst{}.oB(true));

template <>
template <>
std::optional<cme::sbe::ilink::ExecInst> Map<roq::Mask<roq::ExecutionInstruction>>::helper() const {
  return Helper{args_};
}

// roq::OrderType ==> cme::sbe::ilink::OrderTypeReq::Value

template <>
template <>
constexpr Helper<roq::OrderType>::operator std::optional<cme::sbe::ilink::OrderTypeReq::Value>() const {
  switch (std::get<0>(args_)) {
    using enum roq::OrderType;
    case UNDEFINED:
      return cme::sbe::ilink::OrderTypeReq::NULL_VALUE;
    case MARKET:
      return cme::sbe::ilink::OrderTypeReq::MarketwithProtection;
    case LIMIT:
      return cme::sbe::ilink::OrderTypeReq::Limit;
  }
  return {};
}

static_assert(Helper{roq::OrderType::UNDEFINED} == cme::sbe::ilink::OrderTypeReq::NULL_VALUE);
static_assert(Helper{roq::OrderType::MARKET} == cme::sbe::ilink::OrderTypeReq::MarketwithProtection);
static_assert(Helper{roq::OrderType::LIMIT} == cme::sbe::ilink::OrderTypeReq::Limit);

template <>
template <>
std::optional<cme::sbe::ilink::OrderTypeReq::Value> Map<roq::OrderType>::helper() const {
  return Helper{args_};
}

// roq::Side ==> cme::sbe::ilink::SideReq::Value

template <>
template <>
constexpr Helper<roq::Side>::operator std::optional<cme::sbe::ilink::SideReq::Value>() const {
  switch (std::get<0>(args_)) {
    using enum roq::Side;
    case UNDEFINED:
      return cme::sbe::ilink::SideReq::NULL_VALUE;
    case BUY:
      return cme::sbe::ilink::SideReq::Buy;
    case SELL:
      return cme::sbe::ilink::SideReq::Sell;
  }
  return {};
}

static_assert(Helper{roq::Side::UNDEFINED} == cme::sbe::ilink::SideReq::NULL_VALUE);
static_assert(Helper{roq::Side::BUY} == cme::sbe::ilink::SideReq::Buy);
static_assert(Helper{roq::Side::SELL} == cme::sbe::ilink::SideReq::Sell);

template <>
template <>
std::optional<cme::sbe::ilink::SideReq::Value> Map<roq::Side>::helper() const {
  return Helper{args_};
}

// roq::TimeInForce ==> cme::sbe::ilink::TimeInForceReq::Value

template <>
template <>
constexpr Helper<roq::TimeInForce>::operator std::optional<cme::sbe::ilink::TimeInForce::Value>() const {
  switch (std::get<0>(args_)) {
    using enum roq::TimeInForce;
    case UNDEFINED:
      return cme::sbe::ilink::TimeInForce::NULL_VALUE;
    case GFD:
      return ::cme::sbe::ilink::TimeInForce::Day;
    case GTC:
      return ::cme::sbe::ilink::TimeInForce::GoodTillCancel;
    case OPG:
      return cme::sbe::ilink::TimeInForce::NULL_VALUE;
    case IOC:
      return ::cme::sbe::ilink::TimeInForce::FillAndKill;
    case FOK:
      return ::cme::sbe::ilink::TimeInForce::FillOrKill;  // btec and ebs
    case GTX:
      return cme::sbe::ilink::TimeInForce::NULL_VALUE;
    case GTD:
      return ::cme::sbe::ilink::TimeInForce::GoodTillDate;
    case AT_THE_CLOSE:
      return cme::sbe::ilink::TimeInForce::NULL_VALUE;
    case GOOD_THROUGH_CROSSING:
      return cme::sbe::ilink::TimeInForce::NULL_VALUE;
    case AT_CROSSING:
      return cme::sbe::ilink::TimeInForce::NULL_VALUE;
    case GOOD_FOR_TIME:
      return cme::sbe::ilink::TimeInForce::NULL_VALUE;
    case GFA:
      return cme::sbe::ilink::TimeInForce::NULL_VALUE;
    case GFM:
      return cme::sbe::ilink::TimeInForce::NULL_VALUE;
      break;
  }
  return {};
}

static_assert(Helper{roq::TimeInForce::UNDEFINED} == cme::sbe::ilink::TimeInForce::NULL_VALUE);
static_assert(Helper{roq::TimeInForce::GFD} == cme::sbe::ilink::TimeInForce::Day);
static_assert(Helper{roq::TimeInForce::GTC} == cme::sbe::ilink::TimeInForce::GoodTillCancel);
static_assert(Helper{roq::TimeInForce::OPG} == cme::sbe::ilink::TimeInForce::NULL_VALUE);
static_assert(Helper{roq::TimeInForce::IOC} == cme::sbe::ilink::TimeInForce::FillAndKill);
static_assert(Helper{roq::TimeInForce::FOK} == cme::sbe::ilink::TimeInForce::FillOrKill);
static_assert(Helper{roq::TimeInForce::GTX} == cme::sbe::ilink::TimeInForce::NULL_VALUE);
static_assert(Helper{roq::TimeInForce::GTD} == cme::sbe::ilink::TimeInForce::GoodTillDate);
static_assert(Helper{roq::TimeInForce::AT_THE_CLOSE} == cme::sbe::ilink::TimeInForce::NULL_VALUE);
static_assert(Helper{roq::TimeInForce::GOOD_THROUGH_CROSSING} == cme::sbe::ilink::TimeInForce::NULL_VALUE);
static_assert(Helper{roq::TimeInForce::AT_CROSSING} == cme::sbe::ilink::TimeInForce::NULL_VALUE);
static_assert(Helper{roq::TimeInForce::GOOD_FOR_TIME} == cme::sbe::ilink::TimeInForce::NULL_VALUE);
static_assert(Helper{roq::TimeInForce::GFA} == cme::sbe::ilink::TimeInForce::NULL_VALUE);
static_assert(Helper{roq::TimeInForce::GFM} == cme::sbe::ilink::TimeInForce::NULL_VALUE);

template <>
template <>
std::optional<cme::sbe::ilink::TimeInForce::Value> Map<roq::TimeInForce>::helper() const {
  return Helper{args_};
}

}  // namespace roq
