/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <cstdint>
#include <span>

#include "roq/trace.hpp"

#include "roq/cme/ilink/frame.hpp"

#include "roq/cme/ilink/business.hpp"
#include "roq/cme/ilink/execution_report.hpp"
#include "roq/cme/ilink/order.hpp"
#include "roq/cme/ilink/security_definition.hpp"
#include "roq/cme/ilink/session.hpp"

namespace roq {
namespace cme {
namespace ilink {

struct Parser final {
  struct Handler {
    virtual void operator()(Frame const &) = 0;
    // session
    virtual void operator()(Trace<cme_ilink::NegotiationResponse501> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_ilink::NegotiationReject502> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_ilink::EstablishmentAck504> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_ilink::EstablishmentReject505> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_ilink::Sequence506> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_ilink::Terminate507> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_ilink::Retransmission509> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_ilink::RetransmitReject510> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_ilink::NotApplied513> const &, Frame const &) = 0;
    // business
    virtual void operator()(Trace<cme_ilink::BusinessReject521> const &, Frame const &) = 0;
    // execution report
    virtual void operator()(Trace<cme_ilink::ExecutionReportNew522> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_ilink::ExecutionReportReject523> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_ilink::ExecutionReportTradeOutright525> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_ilink::ExecutionReportTradeSpread526> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_ilink::ExecutionReportTradeSpreadLeg527> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_ilink::ExecutionReportModify531> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_ilink::ExecutionReportStatus532> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_ilink::ExecutionReportCancel534> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_ilink::ExecutionReportPendingCancel564> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_ilink::ExecutionReportPendingReplace565> const &, Frame const &) = 0;
    // order
    virtual void operator()(Trace<cme_ilink::OrderCancelReject535> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_ilink::OrderCancelReplaceReject536> const &, Frame const &) = 0;
    // security definition
    virtual void operator()(Trace<cme_ilink::SecurityDefinitionResponse561> const &, Frame const &) = 0;
  };

  static bool dispatch(Handler &, std::span<std::byte const> const &buffer, TraceInfo const &);
};

}  // namespace ilink
}  // namespace cme
}  // namespace roq
