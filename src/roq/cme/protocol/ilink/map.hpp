/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include "cme/sbe/ilink/ExecInst.h"
#include "cme/sbe/ilink/OrdStatusTrd.h"
#include "cme/sbe/ilink/OrderStatus.h"
#include "cme/sbe/ilink/OrderType.h"
#include "cme/sbe/ilink/OrderTypeReq.h"
#include "cme/sbe/ilink/SideReq.h"
#include "cme/sbe/ilink/TimeInForce.h"

#include "roq/execution_instruction.hpp"
#include "roq/order_status.hpp"
#include "roq/order_type.hpp"
#include "roq/side.hpp"
#include "roq/time_in_force.hpp"

#include "roq/mask.hpp"

#include "roq/map.hpp"

namespace roq {

// cme ==> roq

template <>
template <>
std::optional<OrderStatus> Map<::cme::sbe::ilink::OrderStatus::Value>::helper() const;

template <>
template <>
std::optional<OrderStatus> Map<::cme::sbe::ilink::OrdStatusTrd::Value>::helper() const;

template <>
template <>
std::optional<OrderType> Map<::cme::sbe::ilink::OrderType::Value>::helper() const;

template <>
template <>
std::optional<OrderType> Map<::cme::sbe::ilink::OrderTypeReq::Value>::helper() const;

template <>
template <>
std::optional<Side> Map<::cme::sbe::ilink::SideReq::Value>::helper() const;

template <>
template <>
std::optional<TimeInForce> Map<::cme::sbe::ilink::TimeInForce::Value>::helper() const;

// roq ==> cme

template <>
template <>
std::optional<::cme::sbe::ilink::ExecInst> Map<Mask<roq::ExecutionInstruction>>::helper() const;

template <>
template <>
std::optional<::cme::sbe::ilink::OrderTypeReq::Value> Map<OrderType>::helper() const;

template <>
template <>
std::optional<::cme::sbe::ilink::SideReq::Value> Map<Side>::helper() const;

template <>
template <>
std::optional<::cme::sbe::ilink::TimeInForce::Value> Map<TimeInForce>::helper() const;

}  // namespace roq
