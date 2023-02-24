/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <cstdint>
#include <span>

#include "roq/trace.hpp"

#include "roq/cme/mdp/frame.hpp"

#include "roq/cme/mdp/admin.hpp"
#include "roq/cme/mdp/md_incremental_refresh.hpp"
#include "roq/cme/mdp/md_instrument_definition.hpp"
#include "roq/cme/mdp/quote_request.hpp"
#include "roq/cme/mdp/security_status.hpp"
#include "roq/cme/mdp/snapshot_full_refresh.hpp"

namespace roq {
namespace cme {
namespace mdp {

struct Parser final {
  struct Handler {
    virtual void operator()(Frame const &) = 0;
    // admin
    virtual void operator()(Trace<cme_mdp3::AdminHeartbeat12> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_mdp3::ChannelReset4> const &, Frame const &) = 0;
    // instrument definitions
    virtual void operator()(Trace<cme_mdp3::MDInstrumentDefinitionFuture54> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_mdp3::MDInstrumentDefinitionOption55> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_mdp3::MDInstrumentDefinitionSpread56> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_mdp3::MDInstrumentDefinitionFixedIncome57> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_mdp3::MDInstrumentDefinitionRepo58> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_mdp3::MDInstrumentDefinitionFX63> const &, Frame const &) = 0;
    // status
    virtual void operator()(Trace<cme_mdp3::SecurityStatus30> const &, Frame const &) = 0;
    // market by price
    virtual void operator()(Trace<cme_mdp3::SnapshotFullRefresh52> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_mdp3::SnapshotFullRefreshLongQty69> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_mdp3::MDIncrementalRefreshBook46> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_mdp3::MDIncrementalRefreshBookLongQty64> const &, Frame const &) = 0;
    // market by order
    virtual void operator()(Trace<cme_mdp3::SnapshotFullRefreshOrderBook53> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_mdp3::MDIncrementalRefreshOrderBook47> const &, Frame const &) = 0;
    // trade summary
    virtual void operator()(Trace<cme_mdp3::MDIncrementalRefreshTradeSummary48> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_mdp3::MDIncrementalRefreshTradeSummaryLongQty65> const &, Frame const &) = 0;
    // statistics
    virtual void operator()(Trace<cme_mdp3::MDIncrementalRefreshDailyStatistics49> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_mdp3::MDIncrementalRefreshSessionStatistics51> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_mdp3::MDIncrementalRefreshSessionStatisticsLongQty67> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_mdp3::MDIncrementalRefreshVolume37> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_mdp3::MDIncrementalRefreshVolumeLongQty66> const &, Frame const &) = 0;
    // misc
    virtual void operator()(Trace<cme_mdp3::MDIncrementalRefreshLimitsBanding50> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_mdp3::QuoteRequest39> const &, Frame const &) = 0;
  };

  static bool dispatch(Handler &, std::span<std::byte const> const &buffer, TraceInfo const &);
};

}  // namespace mdp
}  // namespace cme
}  // namespace roq
