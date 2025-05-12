/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include <fmt/format.h>

#include <cmath>
#include <span>

#include "roq/cme/mdp/parser.hpp"

using namespace std::literals;

using namespace Catch::literals;

using namespace roq;
using namespace roq::cme;

TEST_CASE("simple", "[md_incremental_refresh_book]") {
  constexpr auto const message =
      "\xca\xb3\xef\x03"
      "\xc0\x48\x09\x86\x43\xcf\x48\x17"
      "\x58\x00"
      "\x0b\x00"
      "\x2e\x00"  // md_incremental_refresh_book_46
      "\x01\x00"
      "\x09\x00"
      "\x75\x96\x07\x86\x43\xcf\x48\x17\x04\x00\x00\x20\x00\x01\x50\x2d\x15\x17\x1f\x00\x00\x00\x49\x00\x00\x00\x03\xef"
      "\x0a\x00\xf8\x6d\xb4\x00\x1a\x00\x00\x00\x02\x01\x30\x00\x00\x00\x00\x00\x18\x00\x00\x00\x00\x00\x00\x01\x78\xd1"
      "\x1e\x88\xa6\x07\x00\x00\xb3\xff\x8a\xd3\x04\x00\x00\x00\x02\x00\x00\x00\x01\x00\x00\x00"
      "\x20\x00"
      "\x0b\x00"
      "\x2e\x00"  // md_incremental_refresh_book_46
      "\x01\x00"
      "\x09\x00"
      "\x75\x96\x07\x86\x43\xcf\x48\x17\x80\x00\x00\x20\x00\x00\x18\x00\x00\x00\x00\x00\x00\x00"sv;
  static_assert(std::size(message) == 132);
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
    void operator()(Trace<cme_mdp::MDInstrumentDefinitionFixedIncome57> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDInstrumentDefinitionRepo58> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDInstrumentDefinitionFX63> const &, mdp::Frame const &) override { FAIL(); }
    // - SnapshotFullRefresh
    void operator()(Trace<cme_mdp::SnapshotFullRefresh52> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::SnapshotFullRefreshOrderBook53> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::SnapshotFullRefreshLongQty69> const &, mdp::Frame const &) override { FAIL(); }
    // - MDIncrementalRefresh
    void operator()(Trace<cme_mdp::MDIncrementalRefreshVolume37> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshBook46> const &event, mdp::Frame const &) override {
      using value_type = std::remove_cvref_t<decltype(event)>::value_type;
      [[maybe_unused]] auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
      ++counter;
    }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshOrderBook47> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshTradeSummary48> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshDailyStatistics49> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshLimitsBanding50> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshSessionStatistics51> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshBookLongQty64> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshTradeSummaryLongQty65> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshVolumeLongQty66> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshSessionStatisticsLongQty67> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::QuoteRequest39> const &, mdp::Frame const &) override { FAIL(); }
  } handler;
  std::span buffer{reinterpret_cast<std::byte const *>(std::data(message)), std::size(message)};
  TraceInfo trace_info;
  auto res = mdp::Parser::dispatch(handler, buffer, trace_info);
  CHECK(res);
  CHECK(handler.counter == 2);
}

TEST_CASE("implied", "[md_incremental_refresh_book]") {
  constexpr auto const message =
      "\xcb\xb3\xef\x03"
      "\xff\x49\x2e\x91\x43\xcf\x48\x17"
      "\x78\x00"
      "\x0b\x00"
      "\x2e\x00"  // md_incremental_refresh_book_46
      "\x01\x00"
      "\x09\x00"
      "\x4d\xa9\x2c\x91\x43\xcf\x48\x17\x04\x00\x00\x20\x00\x02\xb0\x32\x08\xca\x18\x00\x00\x00\x13\x00\x00\x00\x5c\x7f"
      "\x0a\x00\xb1\x4f\x91\x00\x0d\x00\x00\x00\x01\x01\x31\x00\x00\x00\x00\x00\x44\x68\x7f\xca\x18\x00\x00\x00\xd8\x00"
      "\x00\x00\x5c\x7f\x0a\x00\xb2\x4f\x91\x00\x2d\x00\x00\x00\x02\x01\x31\x00\x00\x00\x00\x00\x18\x00\x00\x00\x00\x00"
      "\x00\x01\xae\xce\x1e\x88\xa6\x07\x00\x00\xb4\xff\x8a\xd3\x04\x00\x00\x00\x01\x00\x00\x00\x01\x01\x00\x00"
      "\x40\x01"
      "\x0b\x00"
      "\x2e\x00"  // md_incremental_refresh_book_46
      "\x01\x00"
      "\x09\x00"
      "\x4d\xa9\x2c\x91\x43\xcf\x48\x17\x90\x00\x00\x20\x00\x09\xa2\x2f\x4d\xff\xff\xff\xff\xff\x13\x00\x00\x00\xd7\x46"
      "\x00\x00\x81\xe9\xc3\x00\xff\xff\xff\x7f\x01\x01\x45\x00\x00\x00\x00\x00\xd8\x94\x11\xff\xff\xff\xff\xff\xd8\x00"
      "\x00\x00\xd7\x46\x00\x00\x82\xe9\xc3\x00\xff\xff\xff\x7f\x02\x01\x45\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x0a\x00\x00\x00\xc3\x61\x00\x00\xee\x76\x8a\x00\xff\xff\xff\x7f\x01\x01\x45\x00\x00\x00\x00\x00\x36\x65"
      "\xc4\xff\xff\xff\xff\xff\xe0\x00\x00\x00\xc3\x61\x00\x00\xef\x76\x8a\x00\xff\xff\xff\x7f\x02\x01\x45\x00\x00\x00"
      "\x00\x00\x36\x65\xc4\xff\xff\xff\xff\xff\x05\x00\x00\x00\x43\x63\x00\x00\x46\x20\x14\x00\xff\xff\xff\x7f\x01\x01"
      "\x45\x00\x00\x00\x00\x00\x6c\xca\x88\xff\xff\xff\xff\xff\x01\x00\x00\x00\x43\x63\x00\x00\x47\x20\x14\x00\xff\xff"
      "\xff\x7f\x02\x02\x45\x00\x00\x00\x00\x00\xd8\x94\x11\xff\xff\xff\xff\xff\x01\x00\x00\x00\x43\x63\x00\x00\x48\x20"
      "\x14\x00\xff\xff\xff\x7f\x02\x00\x45\x00\x00\x00\x00\x00\xb0\x29\x23\xfe\xff\xff\xff\xff\x13\x00\x00\x00\x36\x67"
      "\x00\x00\xc7\x80\x01\x01\xff\xff\xff\x7f\x01\x01\x46\x00\x00\x00\x00\x00\x44\x5f\x9a\xfe\xff\xff\xff\xff\xd8\x00"
      "\x00\x00\x36\x67\x00\x00\xc8\x80\x01\x01\xff\xff\xff\x7f\x02\x01\x46\x00\x00\x00\x00\x00\x18\x00\x00\x00\x00\x00"
      "\x00\x00"sv;
  static_assert(std::size(message) == 452);
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
    void operator()(Trace<cme_mdp::MDInstrumentDefinitionFixedIncome57> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDInstrumentDefinitionRepo58> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDInstrumentDefinitionFX63> const &, mdp::Frame const &) override { FAIL(); }
    // - SnapshotFullRefresh
    void operator()(Trace<cme_mdp::SnapshotFullRefresh52> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::SnapshotFullRefreshOrderBook53> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::SnapshotFullRefreshLongQty69> const &, mdp::Frame const &) override { FAIL(); }
    // - MDIncrementalRefresh
    void operator()(Trace<cme_mdp::MDIncrementalRefreshVolume37> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshBook46> const &event, mdp::Frame const &) override {
      using value_type = std::remove_cvref_t<decltype(event)>::value_type;
      [[maybe_unused]] auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
      ++counter;
    }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshOrderBook47> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshTradeSummary48> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshDailyStatistics49> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshLimitsBanding50> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshSessionStatistics51> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshBookLongQty64> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshTradeSummaryLongQty65> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshVolumeLongQty66> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshSessionStatisticsLongQty67> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::QuoteRequest39> const &, mdp::Frame const &) override { FAIL(); }
  } handler;
  std::span buffer{reinterpret_cast<std::byte const *>(std::data(message)), std::size(message)};
  TraceInfo trace_info;
  auto res = mdp::Parser::dispatch(handler, buffer, trace_info);
  CHECK(res);
  CHECK(handler.counter == 2);
}

TEST_CASE("multiple_implied_securities", "[md_incremental_refresh_book]") {
  constexpr auto const message =
      "\xd7\xb3\xef\x03"
      "\x00\xdd\xd3\x56\x46\xcf\x48\x17"
      "\x58\x00"
      "\x0b\x00"
      "\x2e\x00"  // md_incremental_refresh_book_46
      "\x01\x00"
      "\x09\x00"
      "\xcd\x70\xd1\x56\x46\xcf\x48\x17\x04\x00\x00\x20\x00\x01\x20\x6d\xff\xb5\x1c\x00\x00\x00\x11\x00\x00\x00\xc5\xfc"
      "\x01\x00\xee\x09\x9d\x00\x0d\x00\x00\x00\x01\x01\x31\x00\x00\x00\x00\x00\x18\x00\x00\x00\x00\x00\x00\x01\x1d\xd1"
      "\x1e\x88\xa6\x07\x00\x00\x19\xff\x8a\xd3\x04\x00\x00\x00\x01\x00\x00\x00\x01\x02\x00\x00"
      "\x60\x02"
      "\x0b\x00"
      "\x2e\x00"  // md_incremental_refresh_book_46
      "\x01\x00"
      "\x09\x00"
      "\xcd\x70\xd1\x56\x46\xcf\x48\x17\x90\x00\x00\x20\x00\x12\x40\xf4\xa5\xe9\xff\xff\xff\xff\x11\x00\x00\x00\x0f\x19"
      "\x00\x00\xaf\xdf\x99\x00\xff\xff\xff\x7f\x01\x01\x46\x00\x00\x00\x00\x00\x90\xca\x82\xeb\xff\xff\xff\xff\x31\x00"
      "\x00\x00\x0f\x19\x00\x00\xb0\xdf\x99\x00\xff\xff\xff\x7f\x02\x01\x46\x00\x00\x00\x00\x00\x94\x35\x77\x00\x00\x00"
      "\x00\x00\x11\x00\x00\x00\x31\x2a\x00\x00\xe7\x67\xd9\x00\xff\xff\xff\x7f\x01\x01\x45\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x26\x00\x00\x00\x31\x2a\x00\x00\xe8\x67\xd9\x00\xff\xff\xff\x7f\x02\x01\x45\x00\x00\x00"
      "\x00\x00\x60\x53\x46\xfc\xff\xff\xff\xff\x02\x00\x00\x00\xb2\x2d\x00\x00\x11\x3f\x08\x00\xff\xff\xff\x7f\x01\x01"
      "\x46\x00\x00\x00\x00\x00\xb0\x29\x23\xfe\xff\xff\xff\xff\x01\x00\x00\x00\xb2\x2d\x00\x00\x12\x3f\x08\x00\xff\xff"
      "\xff\x7f\x02\x00\x46\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x11\x00\x00\x00\x0a\x6d\x00\x00\x90\x48"
      "\x78\x00\xff\xff\xff\x7f\x01\x01\x45\x00\x00\x00\x00\x00\x36\x65\xc4\xff\xff\xff\xff\xff\x7c\x00\x00\x00\x0a\x6d"
      "\x00\x00\x91\x48\x78\x00\xff\xff\xff\x7f\x02\x01\x45\x00\x00\x00\x00\x00\x28\x6b\xee\x00\x00\x00\x00\x00\x08\x00"
      "\x00\x00\x76\x70\x00\x00\xbf\x9a\xca\x00\xff\xff\xff\x7f\x01\x01\x45\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x06\x00\x00\x00\x76\x70\x00\x00\xc0\x9a\xca\x00\xff\xff\xff\x7f\x02\x01\x45\x00\x00\x00\x00\x00\x98\x3b"
      "\x9e\xf7\xff\xff\xff\xff\x11\x00\x00\x00\xa1\x71\x00\x00\xa8\x66\xa4\x00\xff\xff\xff\x7f\x01\x01\x45\x00\x00\x00"
      "\x00\x00\x28\x6b\xee\x00\x00\x00\x00\x00\x9b\x00\x00\x00\x99\x72\x00\x00\x89\x5c\xf4\x00\xff\xff\xff\x7f\x01\x01"
      "\x45\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x14\x00\x00\x00\x99\x72\x00\x00\x8a\x5c\xf4\x00\xff\xff"
      "\xff\x7f\x02\x01\x45\x00\x00\x00\x00\x00\xd8\x94\x11\xff\xff\xff\xff\xff\x11\x00\x00\x00\x3b\x76\x00\x00\xae\xcd"
      "\xd6\x00\xff\xff\xff\x7f\x01\x01\x45\x00\x00\x00\x00\x00\x60\x53\x46\xfc\xff\xff\xff\xff\x08\x00\x00\x00\xbb\x80"
      "\x00\x00\xe3\x56\xbf\x00\xff\xff\xff\x7f\x01\x01\x46\x00\x00\x00\x00\x00\xb0\x29\x23\xfe\xff\xff\xff\xff\x3a\x00"
      "\x00\x00\xbb\x80\x00\x00\xe4\x56\xbf\x00\xff\xff\xff\x7f\x02\x01\x46\x00\x00\x00\x00\x00\x40\xb0\xab\x1c\x1f\x00"
      "\x00\x00\x08\x00\x00\x00\x03\xef\x0a\x00\xf9\x6d\xb4\x00\xff\xff\xff\x7f\x01\x01\x46\x00\x00\x00\x00\x00\x90\x86"
      "\x88\x1e\x1f\x00\x00\x00\x01\x00\x00\x00\x03\xef\x0a\x00\xfa\x6d\xb4\x00\xff\xff\xff\x7f\x02\x00\x46\x00\x00\x00"
      "\x00\x00\x18\x00\x00\x00\x00\x00\x00\x00"sv;
  static_assert(std::size(message) == 708);
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
    void operator()(Trace<cme_mdp::MDInstrumentDefinitionFixedIncome57> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDInstrumentDefinitionRepo58> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDInstrumentDefinitionFX63> const &, mdp::Frame const &) override { FAIL(); }
    // - SnapshotFullRefresh
    void operator()(Trace<cme_mdp::SnapshotFullRefresh52> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::SnapshotFullRefreshOrderBook53> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::SnapshotFullRefreshLongQty69> const &, mdp::Frame const &) override { FAIL(); }
    // - MDIncrementalRefresh
    void operator()(Trace<cme_mdp::MDIncrementalRefreshVolume37> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshBook46> const &event, mdp::Frame const &) override {
      using value_type = std::remove_cvref_t<decltype(event)>::value_type;
      [[maybe_unused]] auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
      ++counter;
    }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshOrderBook47> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshTradeSummary48> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshDailyStatistics49> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshLimitsBanding50> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshSessionStatistics51> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshBookLongQty64> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshTradeSummaryLongQty65> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshVolumeLongQty66> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshSessionStatisticsLongQty67> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::QuoteRequest39> const &, mdp::Frame const &) override { FAIL(); }
  } handler;
  std::span buffer{reinterpret_cast<std::byte const *>(std::data(message)), std::size(message)};
  TraceInfo trace_info;
  auto res = mdp::Parser::dispatch(handler, buffer, trace_info);
  CHECK(res);
  CHECK(handler.counter == 2);
}

// XXX need MDIncrementalRefreshBookLongQty64
