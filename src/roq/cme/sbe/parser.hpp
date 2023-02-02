/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <cstdint>
#include <span>

#include "roq/trace.hpp"

#include "roq/cme/sbe/frame.hpp"

#include "roq/cme/sbe/admin.hpp"
#include "roq/cme/sbe/md_incremental_refresh.hpp"
#include "roq/cme/sbe/md_instrument_definition.hpp"
#include "roq/cme/sbe/quote_request.hpp"
#include "roq/cme/sbe/security_status.hpp"
#include "roq/cme/sbe/snapshot_full_refresh.hpp"

namespace roq {
namespace cme {
namespace sbe {

struct Parser final {
  struct Handler {
    virtual void operator()(Frame const &) = 0;
    // admin
    virtual void operator()(Trace<cme_mdp::AdminHeartbeat12> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_mdp::ChannelReset4> const &, Frame const &) = 0;
    // instrument definitions
    virtual void operator()(Trace<cme_mdp::MDInstrumentDefinitionFuture54> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_mdp::MDInstrumentDefinitionOption55> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_mdp::MDInstrumentDefinitionSpread56> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_mdp::MDInstrumentDefinitionFixedIncome57> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_mdp::MDInstrumentDefinitionRepo58> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_mdp::MDInstrumentDefinitionFX63> const &, Frame const &) = 0;
    // status
    virtual void operator()(Trace<cme_mdp::SecurityStatus30> const &, Frame const &) = 0;
    // market by price
    virtual void operator()(Trace<cme_mdp::SnapshotFullRefresh52> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_mdp::SnapshotFullRefreshLongQty69> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_mdp::MDIncrementalRefreshBook46> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_mdp::MDIncrementalRefreshBookLongQty64> const &, Frame const &) = 0;
    // market by order
    virtual void operator()(Trace<cme_mdp::SnapshotFullRefreshOrderBook53> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_mdp::MDIncrementalRefreshOrderBook47> const &, Frame const &) = 0;
    // trade summary
    virtual void operator()(Trace<cme_mdp::MDIncrementalRefreshTradeSummary48> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_mdp::MDIncrementalRefreshTradeSummaryLongQty65> const &, Frame const &) = 0;
    // statistics
    virtual void operator()(Trace<cme_mdp::MDIncrementalRefreshDailyStatistics49> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_mdp::MDIncrementalRefreshSessionStatistics51> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_mdp::MDIncrementalRefreshSessionStatisticsLongQty67> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_mdp::MDIncrementalRefreshVolume37> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_mdp::MDIncrementalRefreshVolumeLongQty66> const &, Frame const &) = 0;
    // misc
    virtual void operator()(Trace<cme_mdp::MDIncrementalRefreshLimitsBanding50> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_mdp::QuoteRequest39> const &, Frame const &) = 0;
  };

  static bool dispatch(Handler &, std::span<std::byte const> const &buffer, TraceInfo const &);
};

}  // namespace sbe
}  // namespace cme
}  // namespace roq
