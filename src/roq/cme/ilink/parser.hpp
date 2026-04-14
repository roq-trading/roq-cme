/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <cstdint>
#include <span>

#include "roq/trace.hpp"

#include "roq/cme/ilink/business.hpp"
#include "roq/cme/ilink/execution_report.hpp"
#include "roq/cme/ilink/order.hpp"
#include "roq/cme/ilink/order_mass_action.hpp"
#include "roq/cme/ilink/security_definition.hpp"
#include "roq/cme/ilink/session.hpp"

namespace roq {
namespace cme {
namespace ilink {

struct Parser final {
  struct Handler {
    // session
    virtual void operator()(Trace<::cme::sbe::ilink::NegotiationResponse501> const &) = 0;
    virtual void operator()(Trace<::cme::sbe::ilink::NegotiationReject502> const &) = 0;
    virtual void operator()(Trace<::cme::sbe::ilink::EstablishmentAck504> const &) = 0;
    virtual void operator()(Trace<::cme::sbe::ilink::EstablishmentReject505> const &) = 0;
    virtual void operator()(Trace<::cme::sbe::ilink::Sequence506> const &) = 0;
    virtual void operator()(Trace<::cme::sbe::ilink::Terminate507> const &) = 0;
    virtual void operator()(Trace<::cme::sbe::ilink::Retransmission509> const &) = 0;
    virtual void operator()(Trace<::cme::sbe::ilink::RetransmitReject510> const &) = 0;
    virtual void operator()(Trace<::cme::sbe::ilink::NotApplied513> const &) = 0;
    // business
    virtual void operator()(Trace<::cme::sbe::ilink::PartyDetailsDefinitionRequestAck519> const &) = 0;
    virtual void operator()(Trace<::cme::sbe::ilink::BusinessReject521> const &) = 0;
    // execution report
    virtual void operator()(Trace<::cme::sbe::ilink::ExecutionReportNew522> const &) = 0;
    virtual void operator()(Trace<::cme::sbe::ilink::ExecutionReportReject523> const &) = 0;
    virtual void operator()(Trace<::cme::sbe::ilink::ExecutionReportTradeOutright525> const &) = 0;
    virtual void operator()(Trace<::cme::sbe::ilink::ExecutionReportTradeSpread526> const &) = 0;
    virtual void operator()(Trace<::cme::sbe::ilink::ExecutionReportTradeSpreadLeg527> const &) = 0;
    virtual void operator()(Trace<::cme::sbe::ilink::ExecutionReportModify531> const &) = 0;
    virtual void operator()(Trace<::cme::sbe::ilink::ExecutionReportStatus532> const &) = 0;
    virtual void operator()(Trace<::cme::sbe::ilink::ExecutionReportCancel534> const &) = 0;
    virtual void operator()(Trace<::cme::sbe::ilink::ExecutionReportPendingCancel564> const &) = 0;
    virtual void operator()(Trace<::cme::sbe::ilink::ExecutionReportPendingReplace565> const &) = 0;
    // order
    virtual void operator()(Trace<::cme::sbe::ilink::OrderCancelReject535> const &) = 0;
    virtual void operator()(Trace<::cme::sbe::ilink::OrderCancelReplaceReject536> const &) = 0;
    // order mass action
    virtual void operator()(Trace<::cme::sbe::ilink::OrderMassActionReport562> const &) = 0;
    // security definition
    virtual void operator()(Trace<::cme::sbe::ilink::SecurityDefinitionResponse561> const &) = 0;
  };

  static size_t dispatch(Handler &, std::span<std::byte const> const &buffer, TraceInfo const &);
};

}  // namespace ilink
}  // namespace cme
}  // namespace roq
