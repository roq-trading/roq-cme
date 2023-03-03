/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include <fmt/format.h>

#include <cmath>
#include <span>

#include "roq/cme/mdp/parser.hpp"

using namespace std::literals;

using namespace Catch::literals;

using namespace roq;
using namespace roq::cme;

TEST_CASE("simple", "[md_incremental_refresh_trade_summary]") {
  constexpr auto const message =
      "\x08\xb4\xef\x03"
      "\x51\x9a\x5b\x31\x4b\xcf\x48\x17"
      "\x70\x00"
      "\x0b\x00"
      "\x30\x00"  // md_incremental_refresh_trade_summary_48
      "\x01\x00"
      "\x09\x00"
      "\x29\xb6\x55\x31\x4b\xcf\x48\x17\x01\x00\x00\x20\x00\x01\x5a\xe1\x6a\xab\x17\x00\x00\x00\x02\x00\x00\x00\x5e\xac"
      "\x0a\x00\xe7\x7c\x3e\x00\x03\x00\x00\x00\x01\x00\x76\x85\x43\x04\x00\x00\x10\x00\x00\x00\x00\x00\x00\x03\x92\xd1"
      "\x1e\x88\xa6\x07\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x46\xb3\x1e\x88\xa6\x07\x00\x00\x01\x00\x00\x00\x00\x00"
      "\x00\x00\x64\xd1\x1e\x88\xa6\x07\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00"sv;
  static_assert(std::size(message) == 124);
  struct MyHandler final : public mdp::Parser::Handler {
    int counter = 0;
    void operator()(mdp::Frame const &) override {}
    void operator()(Trace<cme_mdp::ChannelReset4> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::AdminHeartbeat12> const &, mdp::Frame const &) override { FAIL(); }
    // - security status
    void operator()(Trace<cme_mdp::SecurityStatus30> const &, mdp::Frame const &) override { FAIL(); }
    // - instrument definition
    void operator()(Trace<cme_mdp::MDInstrumentDefinitionFuture54> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDInstrumentDefinitionOption55> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDInstrumentDefinitionSpread56> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDInstrumentDefinitionFixedIncome57> const &, mdp::Frame const &) override {
      FAIL();
    }
    void operator()(Trace<cme_mdp::MDInstrumentDefinitionRepo58> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDInstrumentDefinitionFX63> const &, mdp::Frame const &) override { FAIL(); }
    // - SnapshotFullRefresh
    void operator()(Trace<cme_mdp::SnapshotFullRefresh52> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::SnapshotFullRefreshOrderBook53> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::SnapshotFullRefreshLongQty69> const &, mdp::Frame const &) override { FAIL(); }
    // - MDIncrementalRefresh
    void operator()(Trace<cme_mdp::MDIncrementalRefreshVolume37> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshBook46> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshOrderBook47> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshTradeSummary48> const &event, mdp::Frame const &) override {
      using value_type = std::remove_cvref<decltype(event)>::type::value_type;
      auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
      ++counter;
    }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshDailyStatistics49> const &, mdp::Frame const &) override {
      FAIL();
    }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshLimitsBanding50> const &, mdp::Frame const &) override {
      FAIL();
    }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshSessionStatistics51> const &, mdp::Frame const &) override {
      FAIL();
    }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshBookLongQty64> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshTradeSummaryLongQty65> const &, mdp::Frame const &) override {
      FAIL();
    }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshVolumeLongQty66> const &, mdp::Frame const &) override {
      FAIL();
    }
    void operator()(
        Trace<cme_mdp::MDIncrementalRefreshSessionStatisticsLongQty67> const &, mdp::Frame const &) override {
      FAIL();
    }
    void operator()(Trace<cme_mdp::QuoteRequest39> const &, mdp::Frame const &) override { FAIL(); }
  } handler;
  std::span buffer{reinterpret_cast<std::byte const *>(std::data(message)), std::size(message)};
  TraceInfo trace_info;
  auto res = mdp::Parser::dispatch(handler, buffer, trace_info);
  CHECK(res);
  CHECK(handler.counter == 1);
}

TEST_CASE("multiple_fills", "[md_incremental_refresh_trade_summary]") {
  constexpr auto const message =
      "\x9c\xb4\xef\x03"
      "\x63\xdf\x87\x81\x54\xcf\x48\x17"
      "\x90\x00"
      "\x0b\x00"
      "\x30\x00"  // md_incremental_refresh_trade_summary_48
      "\x01\x00"
      "\x09\x00"
      "\xcb\x2c\x83\x81\x54\xcf\x48\x17\x01\x00\x00\x20\x00\x01\x5a\xe1\x6a\xab\x17\x00\x00\x00\x05\x00\x00\x00\x5e\xac"
      "\x0a\x00\xee\x7c\x3e\x00\x05\x00\x00\x00\x01\x00\x7f\x85\x43\x04\x00\x00\x10\x00\x00\x00\x00\x00\x00\x05\xd1\xd1"
      "\x1e\x88\xa6\x07\x00\x00\x05\x00\x00\x00\x00\x00\x00\x00\xd2\xb5\x1e\x88\xa6\x07\x00\x00\x02\x00\x00\x00\x00\x00"
      "\x00\x00\x64\xd1\x1e\x88\xa6\x07\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\xee\xad\x1e\x88\xa6\x07\x00\x00\x01\x00"
      "\x00\x00\x00\x00\x00\x00\xb3\xd1\x1e\x88\xa6\x07\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00"sv;
  static_assert(std::size(message) == 156);
  struct MyHandler final : public mdp::Parser::Handler {
    int counter = 0;
    void operator()(mdp::Frame const &) override {}
    void operator()(Trace<cme_mdp::ChannelReset4> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::AdminHeartbeat12> const &, mdp::Frame const &) override { FAIL(); }
    // - security status
    void operator()(Trace<cme_mdp::SecurityStatus30> const &, mdp::Frame const &) override { FAIL(); }
    // - instrument definition
    void operator()(Trace<cme_mdp::MDInstrumentDefinitionFuture54> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDInstrumentDefinitionOption55> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDInstrumentDefinitionSpread56> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDInstrumentDefinitionFixedIncome57> const &, mdp::Frame const &) override {
      FAIL();
    }
    void operator()(Trace<cme_mdp::MDInstrumentDefinitionRepo58> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDInstrumentDefinitionFX63> const &, mdp::Frame const &) override { FAIL(); }
    // - SnapshotFullRefresh
    void operator()(Trace<cme_mdp::SnapshotFullRefresh52> const &, mdp::Frame const &) override {}
    void operator()(Trace<cme_mdp::SnapshotFullRefreshOrderBook53> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::SnapshotFullRefreshLongQty69> const &, mdp::Frame const &) override { FAIL(); }
    // - MDIncrementalRefresh
    void operator()(Trace<cme_mdp::MDIncrementalRefreshVolume37> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshBook46> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshOrderBook47> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshTradeSummary48> const &event, mdp::Frame const &) override {
      using value_type = std::remove_cvref<decltype(event)>::type::value_type;
      auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
      ++counter;
    }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshDailyStatistics49> const &, mdp::Frame const &) override {
      FAIL();
    }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshLimitsBanding50> const &, mdp::Frame const &) override {
      FAIL();
    }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshSessionStatistics51> const &, mdp::Frame const &) override {
      FAIL();
    }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshBookLongQty64> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshTradeSummaryLongQty65> const &, mdp::Frame const &) override {
      FAIL();
    }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshVolumeLongQty66> const &, mdp::Frame const &) override {
      FAIL();
    }
    void operator()(
        Trace<cme_mdp::MDIncrementalRefreshSessionStatisticsLongQty67> const &, mdp::Frame const &) override {
      FAIL();
    }
    void operator()(Trace<cme_mdp::QuoteRequest39> const &, mdp::Frame const &) override { FAIL(); }
  } handler;
  std::span buffer{reinterpret_cast<std::byte const *>(std::data(message)), std::size(message)};
  TraceInfo trace_info;
  auto res = mdp::Parser::dispatch(handler, buffer, trace_info);
  CHECK(res);
  CHECK(handler.counter == 1);
}

// XXX need example with trading through and leaving order
// XXX need example with multiple securities
// XXX need example with _65
