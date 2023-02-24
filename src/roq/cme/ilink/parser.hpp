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
    virtual void operator()(Trace<cme_ilink3::NegotiationResponse501> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_ilink3::NegotiationReject502> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_ilink3::EstablishmentAck504> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_ilink3::EstablishmentReject505> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_ilink3::Sequence506> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_ilink3::Terminate507> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_ilink3::Retransmission509> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_ilink3::RetransmitReject510> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_ilink3::NotApplied513> const &, Frame const &) = 0;
    // business
    virtual void operator()(Trace<cme_ilink3::BusinessReject521> const &, Frame const &) = 0;
    // execution report
    virtual void operator()(Trace<cme_ilink3::ExecutionReportNew522> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_ilink3::ExecutionReportReject523> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_ilink3::ExecutionReportTradeOutright525> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_ilink3::ExecutionReportTradeSpread526> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_ilink3::ExecutionReportTradeSpreadLeg527> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_ilink3::ExecutionReportModify531> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_ilink3::ExecutionReportStatus532> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_ilink3::ExecutionReportCancel534> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_ilink3::ExecutionReportPendingCancel564> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_ilink3::ExecutionReportPendingReplace565> const &, Frame const &) = 0;
    // order
    virtual void operator()(Trace<cme_ilink3::OrderCancelReject535> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_ilink3::OrderCancelReplaceReject536> const &, Frame const &) = 0;
    // security definition
    virtual void operator()(Trace<cme_ilink3::SecurityDefinitionResponse561> const &, Frame const &) = 0;
  };

  static bool dispatch(Handler &, std::span<std::byte const> const &buffer, TraceInfo const &);
};

}  // namespace ilink
}  // namespace cme
}  // namespace roq
