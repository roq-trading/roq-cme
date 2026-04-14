/* Copyright (c) 2017-2026, Hans Erik Thrane */

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
    virtual void operator()(Trace<::cme::sbe::mdp::AdminHeartbeat12> const &, Frame const &) = 0;
    virtual void operator()(Trace<::cme::sbe::mdp::ChannelReset4> const &, Frame const &) = 0;
    // instrument definitions
    virtual void operator()(Trace<::cme::sbe::mdp::MDInstrumentDefinitionFuture54> const &, Frame const &) = 0;
    virtual void operator()(Trace<::cme::sbe::mdp::MDInstrumentDefinitionOption55> const &, Frame const &) = 0;
    virtual void operator()(Trace<::cme::sbe::mdp::MDInstrumentDefinitionSpread56> const &, Frame const &) = 0;
    virtual void operator()(Trace<::cme::sbe::mdp::MDInstrumentDefinitionFixedIncome57> const &, Frame const &) = 0;
    virtual void operator()(Trace<::cme::sbe::mdp::MDInstrumentDefinitionRepo58> const &, Frame const &) = 0;
    virtual void operator()(Trace<::cme::sbe::mdp::MDInstrumentDefinitionFX63> const &, Frame const &) = 0;
    // status
    virtual void operator()(Trace<::cme::sbe::mdp::SecurityStatus30> const &, Frame const &) = 0;
    // market by price
    virtual void operator()(Trace<::cme::sbe::mdp::SnapshotFullRefresh52> const &, Frame const &) = 0;
    virtual void operator()(Trace<::cme::sbe::mdp::SnapshotFullRefreshLongQty69> const &, Frame const &) = 0;
    virtual void operator()(Trace<::cme::sbe::mdp::MDIncrementalRefreshBook46> const &, Frame const &) = 0;
    virtual void operator()(Trace<::cme::sbe::mdp::MDIncrementalRefreshBookLongQty64> const &, Frame const &) = 0;
    // market by order
    virtual void operator()(Trace<::cme::sbe::mdp::SnapshotFullRefreshOrderBook53> const &, Frame const &) = 0;
    virtual void operator()(Trace<::cme::sbe::mdp::MDIncrementalRefreshOrderBook47> const &, Frame const &) = 0;
    // trade summary
    virtual void operator()(Trace<::cme::sbe::mdp::MDIncrementalRefreshTradeSummary48> const &, Frame const &) = 0;
    virtual void operator()(Trace<::cme::sbe::mdp::MDIncrementalRefreshTradeSummaryLongQty65> const &, Frame const &) = 0;
    // statistics
    virtual void operator()(Trace<::cme::sbe::mdp::MDIncrementalRefreshDailyStatistics49> const &, Frame const &) = 0;
    virtual void operator()(Trace<::cme::sbe::mdp::MDIncrementalRefreshSessionStatistics51> const &, Frame const &) = 0;
    virtual void operator()(Trace<::cme::sbe::mdp::MDIncrementalRefreshSessionStatisticsLongQty67> const &, Frame const &) = 0;
    virtual void operator()(Trace<::cme::sbe::mdp::MDIncrementalRefreshVolume37> const &, Frame const &) = 0;
    virtual void operator()(Trace<::cme::sbe::mdp::MDIncrementalRefreshVolumeLongQty66> const &, Frame const &) = 0;
    // misc
    virtual void operator()(Trace<::cme::sbe::mdp::MDIncrementalRefreshLimitsBanding50> const &, Frame const &) = 0;
    virtual void operator()(Trace<::cme::sbe::mdp::QuoteRequest39> const &, Frame const &) = 0;
  };

  static bool dispatch(Handler &, std::span<std::byte const> const &buffer, TraceInfo const &);
};

}  // namespace mdp
}  // namespace cme
}  // namespace roq
