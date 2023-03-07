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

TEST_CASE("span_multiple messages", "[md_incremental_refresh_trade_summary]") {
  constexpr auto const message_1 =
      "\x8b\x8e\xc9\x00"
      "\x39\xa8\x5e\xac\x99\x11\x4a\x17"
      "\x70\x05"
      "\x0b\x00"
      "\x30\x00"  // md_incremental_refresh_trade_summary_48
      "\x01\x00"
      "\x09\x00"
      "\x59\x81\x4e\xac\x99\x11\x4a\x17\x00\x00\x00\x20\x00\x05\x08\x07\x39\xe4\x19\x00\x00\x00\xe8\x03\x00\x00\xd6\x0d"
      "\x01\x00\x80\x21\x37\x00\x4f\x00\x00\x00\x01\x00\x13\x3b\x6f\x00\x00\x00\xf0\x24\x2b\x20\x1d\x00\x00\x00\x08\x00"
      "\x00\x00\xc5\xfc\x01\x00\x03\x0f\x28\x00\x05\x00\x00\x00\x00\x00\x14\x3b\x6f\x00\x00\x00\xd8\x94\x11\xff\xff\xff"
      "\xff\xff\x08\x00\x00\x00\x3b\x76\x00\x00\xc2\x8d\x34\x00\x03\x00\x00\x00\x00\x00\x15\x3b\x6f\x00\x00\x00\x20\x4c"
      "\x91\x32\x1b\x00\x00\x00\x0c\x00\x00\x00\xb1\xec\x06\x00\x83\x72\x27\x00\x05\x00\x00\x00\x00\x00\x16\x3b\x6f\x00"
      "\x00\x00\xa0\xac\xb9\x03\x00\x00\x00\x00\x0c\x00\x00\x00\xb7\x74\x00\x00\x7f\x3f\x41\x00\x06\x00\x00\x00\x00\x00"
      "\x17\x3b\x6f\x00\x00\x00\x10\x00\x00\x00\x00\x00\x00\x4b\xf4\x05\xd5\x88\xa6\x07\x00\x00\xe8\x03\x00\x00\x00\x00"
      "\x00\x00\x23\x4c\xd4\x88\xa6\x07\x00\x00\x24\x00\x00\x00\x00\x00\x00\x00\x29\x4c\xd4\x88\xa6\x07\x00\x00\x01\x00"
      "\x00\x00\x00\x00\x00\x00\x2b\x4c\xd4\x88\xa6\x07\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x33\x4c\xd4\x88\xa6\x07"
      "\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x3e\x4c\xd4\x88\xa6\x07\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x40\x4c"
      "\xd4\x88\xa6\x07\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x41\x4c\xd4\x88\xa6\x07\x00\x00\x01\x00\x00\x00\x00\x00"
      "\x00\x00\x46\x4c\xd4\x88\xa6\x07\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x70\x4c\xd4\x88\xa6\x07\x00\x00\x02\x00"
      "\x00\x00\x00\x00\x00\x00\x87\x4c\xd4\x88\xa6\x07\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x98\x4c\xd4\x88\xa6\x07"
      "\x00\x00\x1a\x00\x00\x00\x00\x00\x00\x00\x99\x4c\xd4\x88\xa6\x07\x00\x00\x31\x00\x00\x00\x00\x00\x00\x00\xcd\x3f"
      "\xd4\x88\xa6\x07\x00\x00\x08\x00\x00\x00\x00\x00\x00\x00\xed\x4c\xd4\x88\xa6\x07\x00\x00\x01\x00\x00\x00\x00\x00"
      "\x00\x00\x88\x3d\xd4\x88\xa6\x07\x00\x00\x0c\x00\x00\x00\x00\x00\x00\x00\xfc\x4c\xd4\x88\xa6\x07\x00\x00\x01\x00"
      "\x00\x00\x00\x00\x00\x00\x04\x4d\xd4\x88\xa6\x07\x00\x00\x17\x00\x00\x00\x00\x00\x00\x00\xfe\x4c\xd4\x88\xa6\x07"
      "\x00\x00\x09\x00\x00\x00\x00\x00\x00\x00\x38\x4d\xd4\x88\xa6\x07\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x45\x4d"
      "\xd4\x88\xa6\x07\x00\x00\x28\x00\x00\x00\x00\x00\x00\x00\x46\x4d\xd4\x88\xa6\x07\x00\x00\x01\x00\x00\x00\x00\x00"
      "\x00\x00\x4c\x4d\xd4\x88\xa6\x07\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x12\x4d\xd4\x88\xa6\x07\x00\x00\x0d\x00"
      "\x00\x00\x00\x00\x00\x00\x3f\x0c\xd4\x88\xa6\x07\x00\x00\x18\x00\x00\x00\x00\x00\x00\x00\x7c\x4d\xd4\x88\xa6\x07"
      "\x00\x00\x38\x00\x00\x00\x00\x00\x00\x00\x95\x4e\xd4\x88\xa6\x07\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x96\x4e"
      "\xd4\x88\xa6\x07\x00\x00\x19\x00\x00\x00\x00\x00\x00\x00\x82\x52\xd4\x88\xa6\x07\x00\x00\x11\x00\x00\x00\x00\x00"
      "\x00\x00\xbd\x54\xd4\x88\xa6\x07\x00\x00\x27\x00\x00\x00\x00\x00\x00\x00\x8d\x5b\xd4\x88\xa6\x07\x00\x00\x02\x00"
      "\x00\x00\x00\x00\x00\x00\xab\x5d\xd4\x88\xa6\x07\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x16\x61\xd4\x88\xa6\x07"
      "\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x31\x62\xd4\x88\xa6\x07\x00\x00\x32\x00\x00\x00\x00\x00\x00\x00\x78\x62"
      "\xd4\x88\xa6\x07\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\xa5\x63\xd4\x88\xa6\x07\x00\x00\x01\x00\x00\x00\x00\x00"
      "\x00\x00\x59\x65\xd4\x88\xa6\x07\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x9e\x67\xd4\x88\xa6\x07\x00\x00\x02\x00"
      "\x00\x00\x00\x00\x00\x00\x35\x6e\xd4\x88\xa6\x07\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x3b\x72\xd4\x88\xa6\x07"
      "\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\xbc\x76\xd4\x88\xa6\x07\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x7d\x80"
      "\xd4\x88\xa6\x07\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x8c\x80\xd4\x88\xa6\x07\x00\x00\x01\x00\x00\x00\x00\x00"
      "\x00\x00\x26\x83\xd4\x88\xa6\x07\x00\x00\x05\x00\x00\x00\x00\x00\x00\x00\x8a\x87\xd4\x88\xa6\x07\x00\x00\x1e\x00"
      "\x00\x00\x00\x00\x00\x00\x6d\x9f\xd4\x88\xa6\x07\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x2d\xaa\xd4\x88\xa6\x07"
      "\x00\x00\x0a\x00\x00\x00\x00\x00\x00\x00\x39\xaa\xd4\x88\xa6\x07\x00\x00\x0a\x00\x00\x00\x00\x00\x00\x00\x55\xaa"
      "\xd4\x88\xa6\x07\x00\x00\x0a\x00\x00\x00\x00\x00\x00\x00\x62\xab\xd4\x88\xa6\x07\x00\x00\x0a\x00\x00\x00\x00\x00"
      "\x00\x00\xbc\xab\xd4\x88\xa6\x07\x00\x00\x07\x00\x00\x00\x00\x00\x00\x00\xdd\xab\xd4\x88\xa6\x07\x00\x00\x02\x00"
      "\x00\x00\x00\x00\x00\x00\x2a\xb0\xd4\x88\xa6\x07\x00\x00\x29\x00\x00\x00\x00\x00\x00\x00\xcc\xb0\xd4\x88\xa6\x07"
      "\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x7e\xcc\xd4\x88\xa6\x07\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x2d\xe1"
      "\xd4\x88\xa6\x07\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x2f\xe1\xd4\x88\xa6\x07\x00\x00\x05\x00\x00\x00\x00\x00"
      "\x00\x00\x33\xe1\xd4\x88\xa6\x07\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x1e\xe2\xd4\x88\xa6\x07\x00\x00\x19\x00"
      "\x00\x00\x00\x00\x00\x00\x0d\xe3\xd4\x88\xa6\x07\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x8a\xe4\xd4\x88\xa6\x07"
      "\x00\x00\x03\x00\x00\x00\x00\x00\x00\x00\xad\xe8\xd4\x88\xa6\x07\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\xbd\xed"
      "\xd4\x88\xa6\x07\x00\x00\x32\x00\x00\x00\x00\x00\x00\x00\x38\xf0\xd4\x88\xa6\x07\x00\x00\x65\x00\x00\x00\x00\x00"
      "\x00\x00\x39\xf0\xd4\x88\xa6\x07\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\xa6\xf3\xd4\x88\xa6\x07\x00\x00\x05\x00"
      "\x00\x00\x00\x00\x00\x00\x4e\xf6\xd4\x88\xa6\x07\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\xcd\xf9\xd4\x88\xa6\x07"
      "\x00\x00\x2d\x00\x00\x00\x00\x00\x00\x00\x0f\x01\xd5\x88\xa6\x07\x00\x00\x32\x00\x00\x00\x00\x00\x00\x00\x4d\x01"
      "\xd5\x88\xa6\x07\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x9c\x02\xd5\x88\xa6\x07\x00\x00\x01\x00\x00\x00\x00\x00"
      "\x00\x00\xe9\x02\xd5\x88\xa6\x07\x00\x00\x24\x00\x00\x00\x00\x00\x00\x00\x03\x03\xd5\x88\xa6\x07\x00\x00\x02\x00"
      "\x00\x00\x00\x00\x00\x00\x05\x03\xd5\x88\xa6\x07\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x3c\x03\xd5\x88\xa6\x07"
      "\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00"sv;
  static_assert(std::size(message_1) == 1404);
  constexpr auto const message_2 =
      "\x8c\x8e\xc9\x00"
      "\x10\x07\x5f\xac\x99\x11\x4a\x17"
      "\x90\x01"
      "\x0b\x00"
      "\x30\x00"  // md_incremental_refresh_trade_summary_48
      "\x01\x00"
      "\x09\x00"
      "\x59\x81\x4e\xac\x99\x11\x4a\x17\x01\x00\x00\x20\x00\x00\x10\x00\x00\x00\x00\x00\x00\x17\x8f\x03\xd5\x88\xa6\x07"
      "\x00\x00\x03\x00\x00\x00\x00\x00\x00\x00\xed\x04\xd5\x88\xa6\x07\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x92\x05"
      "\xd5\x88\xa6\x07\x00\x00\x05\x00\x00\x00\x00\x00\x00\x00\x93\x05\xd5\x88\xa6\x07\x00\x00\x11\x00\x00\x00\x00\x00"
      "\x00\x00\xa0\x05\xd5\x88\xa6\x07\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\xd4\x05\xd5\x88\xa6\x07\x00\x00\x02\x00"
      "\x00\x00\x00\x00\x00\x00\xd5\x05\xd5\x88\xa6\x07\x00\x00\x03\x00\x00\x00\x00\x00\x00\x00\xf1\x05\xd5\x88\xa6\x07"
      "\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\xf2\x05\xd5\x88\xa6\x07\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x50\x1f"
      "\xd4\x88\xa6\x07\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x52\x1f\xd4\x88\xa6\x07\x00\x00\x03\x00\x00\x00\x00\x00"
      "\x00\x00\x54\x1f\xd4\x88\xa6\x07\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x7e\xf2\xd4\x88\xa6\x07\x00\x00\x03\x00"
      "\x00\x00\x00\x00\x00\x00\x7f\xf2\xd4\x88\xa6\x07\x00\x00\x03\x00\x00\x00\x00\x00\x00\x00\x80\xf2\xd4\x88\xa6\x07"
      "\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x81\xf2\xd4\x88\xa6\x07\x00\x00\x03\x00\x00\x00\x00\x00\x00\x00\x4d\xe4"
      "\xd4\x88\xa6\x07\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x25\x98\xce\x88\xa6\x07\x00\x00\x01\x00\x00\x00\x00\x00"
      "\x00\x00\x75\x98\xce\x88\xa6\x07\x00\x00\x03\x00\x00\x00\x00\x00\x00\x00\xe6\x98\xce\x88\xa6\x07\x00\x00\x03\x00"
      "\x00\x00\x00\x00\x00\x00\x2c\xa1\xce\x88\xa6\x07\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x54\xa1\xce\x88\xa6\x07"
      "\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x7c\xa1\xce\x88\xa6\x07\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00"sv;
  static_assert(std::size(message_2) == 412);
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
      fmt::print("{}"sv, value);
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
  TraceInfo trace_info;
  std::span buffer_1{reinterpret_cast<std::byte const *>(std::data(message_1)), std::size(message_1)};
  auto res = mdp::Parser::dispatch(handler, buffer_1, trace_info);
  CHECK(res);
  CHECK(handler.counter == 1);
  std::span buffer_2{reinterpret_cast<std::byte const *>(std::data(message_2)), std::size(message_2)};
  res = mdp::Parser::dispatch(handler, buffer_2, trace_info);
  CHECK(res);
  CHECK(handler.counter == 2);
}
