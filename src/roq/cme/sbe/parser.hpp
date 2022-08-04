/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <cstdint>
#include <span>

#include "roq/trace.hpp"

#include "roq/cme/sbe/frame.hpp"

#include "roq/cme/sbe/md_incremental_refresh.hpp"
#include "roq/cme/sbe/md_instrument_definition.hpp"
#include "roq/cme/sbe/snapshot_full_refresh.hpp"

namespace roq {
namespace cme {
namespace sbe {

struct Parser final {
  struct Handler {
    // - MDInstrumentDefinition
    virtual void operator()(Trace<cme_mdp::MDInstrumentDefinitionFuture54> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_mdp::MDInstrumentDefinitionOption55> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_mdp::MDInstrumentDefinitionSpread56> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_mdp::MDInstrumentDefinitionFixedIncome57> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_mdp::MDInstrumentDefinitionRepo58> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_mdp::MDInstrumentDefinitionFX63> const &, Frame const &) = 0;
    // - SnapshotFullRefresh
    virtual void operator()(Trace<cme_mdp::SnapshotFullRefresh52> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_mdp::SnapshotFullRefreshOrderBook53> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_mdp::SnapshotFullRefreshLongQty69> const &, Frame const &) = 0;
    // - MDIncrementalRefresh
    virtual void operator()(Trace<cme_mdp::MDIncrementalRefreshVolume37> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_mdp::MDIncrementalRefreshBook46> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_mdp::MDIncrementalRefreshOrderBook47> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_mdp::MDIncrementalRefreshTradeSummary48> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_mdp::MDIncrementalRefreshDailyStatistics49> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_mdp::MDIncrementalRefreshLimitsBanding50> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_mdp::MDIncrementalRefreshSessionStatistics51> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_mdp::MDIncrementalRefreshBookLongQty64> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_mdp::MDIncrementalRefreshTradeSummaryLongQty65> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_mdp::MDIncrementalRefreshVolumeLongQty66> const &, Frame const &) = 0;
    virtual void operator()(Trace<cme_mdp::MDIncrementalRefreshSessionStatisticsLongQty67> const &, Frame const &) = 0;
  };

  static bool dispatch(Handler &, std::span<std::byte const> const &buffer, TraceInfo const &);
};

}  // namespace sbe
}  // namespace cme
}  // namespace roq
