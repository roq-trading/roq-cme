/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include <fmt/format.h>

#include <cmath>
#include <span>

#include "roq/cme/sbe/parser.hpp"

using namespace std::literals;

using namespace Catch::literals;

using namespace roq;
using namespace roq::cme;

TEST_CASE("md_incremental_refresh_book_46_test_1", "[sbe]") {
  auto message =
      // [x00] technical header (UDP)
      "\xc6\xef\x0f\x00"                  // sequence number
      "\x05\x81\x88\x3d\x49\x42\x07\x17"  // sending time
      // [x0c] message header
      "\x58\x00"  // message size (88)
      "\x0b\x00"  // block length (11)
      "\x2e\x00"  // template id (46 = MDIncrementalRefreshBook46)
      "\x01\x00"  // schema id (1)
      "\x09\x00"  // version (9)
      // [x16] MDIncrementalRefreshBook46
      "\x11\x34\x87\x3d\x49\x42\x07\x17"  // transact time (1659367870041633809ns)
      "\x04"                              // match event indicator (4 = lastQuoteMsg ???)
      "\x00\x00"                          // padding
      // [x21] - NoMDEntries (32)
      "\x20\x00"  // block length (32)
      "\x01"      // num in group (1)
      // [x24] --- #1 ---
      "\x00\x16\xf8\xf3\xb1\x3c\x00\x00"  // price (66735.0)
      "\x02\x00\x00\x00"                  // size (2)
      "\xff\x78\x00\x00"                  // security id (30975)
      "\x5e\x3b\x01\x00"                  // report sequence (80734)
      "\x01\x00\x00\x00"                  // number of orders (1)
      "\x02"                              // price level (2)
      "\x00"                              // update action (0 = new)
      "\x31"                              // entry type ('1' = offer)
      "\x00\x00\x00\x00"                  // tradeable size
      "\x00"                              // padding
      // [x44] - NoOrderIDEntries (24)
      "\x18\x00"              // block length (24)
      "\x00\x00\x00\x00\x00"  // padding
      "\x01"                  // num in group (1)
      // [x4c] --- #1 ---
      "\x9b\x5f\x5e\xa4\xa4\x00\x00\x00"  // order id (707132284827) ???
      "\x7e\x44\x46\xee\x03\x00\x00\x00"  // order priority (16882484350) ???
      "\x02\x00\x00\x00"                  // display quantity (2)
      "\x01"                              // reference id (1)
      "\x00"                              // order update action (0=new)
      "\x00\x00"                          // padding
      // [x64] message #2
      "\x20\x00"  // message size (32)
      "\x0b\x00"  // block length (11)
      "\x2e\x00"  // template id (46 = MDIncrementalRefreshBook46)
      "\x01\x00"  // schema id (1)
      "\x09\x00"  // version (9)
      // [??] MDIncrementalRefreshBook46
      "\x11\x34\x87\x3d\x49\x42\x07\x17"  // transact time
      "\x80"                              // match event indicator
      "\x00\x00"                          // padding
      // [??] - NoMDEntries (32)
      "\x20\x00"  // block length
      "\x00"      // num in group (0)
      // [??] - NoOrderIDEntries (24)
      "\x18\x00"              // block length (24)
      "\x00\x00\x00\x00\x00"  // padding
      "\x00"sv;               // num in group (0)
  CHECK(std::size(message) == 132);
  struct MyHandler final : public sbe::Parser::Handler {
    int counter = 0;
    // - MDInstrumentDefinition
    void operator()(Trace<cme_mdp::MDInstrumentDefinitionFuture54> const &, sbe::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDInstrumentDefinitionOption55> const &, sbe::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDInstrumentDefinitionSpread56> const &, sbe::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDInstrumentDefinitionFixedIncome57> const &, sbe::Frame const &) override {
      FAIL();
    }
    void operator()(Trace<cme_mdp::MDInstrumentDefinitionRepo58> const &, sbe::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDInstrumentDefinitionFX63> const &, sbe::Frame const &) override { FAIL(); }
    // - SnapshotFullRefresh
    void operator()(Trace<cme_mdp::SnapshotFullRefresh52> const &, sbe::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::SnapshotFullRefreshOrderBook53> const &, sbe::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::SnapshotFullRefreshLongQty69> const &, sbe::Frame const &) override { FAIL(); }
    // - MDIncrementalRefresh
    void operator()(Trace<cme_mdp::MDIncrementalRefreshVolume37> const &, sbe::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshBook46> const &event, sbe::Frame const &frame) override {
      CHECK(frame.sequence_number == 1044422);
      CHECK(frame.sending_time == 1659367870041719045ns);
      auto &[trace_info, book] = event;
      switch (++counter) {
        case 1: {
          CHECK(book.transactTime() == 1659367870041633809);
          book.sbeRewind();  // wtf!
          auto no_md_entries_rows = 0;
          book.noMDEntries().forEach([&no_md_entries_rows](auto &item) {
            ++no_md_entries_rows;
            CHECK(sbe::map(item.mDEntryPx()) == 66735.0_a);
            CHECK(item.mDEntrySize() == 2);
            CHECK(item.securityID() == 30975);
            CHECK(item.rptSeq() == 80734);
            CHECK(item.numberOfOrders() == 1);
            CHECK(item.mDPriceLevel() == 2);
            CHECK(item.mDUpdateAction() == cme_mdp::MDUpdateAction::Value::New);
            CHECK(item.mDEntryType() == cme_mdp::MDEntryTypeBook::Value::Offer);
            CHECK(item.tradeableSize() == 0);
          });
          CHECK(no_md_entries_rows == 1);
          auto no_order_id_intries_rows = 0;
          book.noOrderIDEntries().forEach([&no_order_id_intries_rows](auto &item) {
            ++no_order_id_intries_rows;
            CHECK(item.orderID() == 707132284827);
            CHECK(item.mDOrderPriority() == 16882484350);
            CHECK(item.mDDisplayQty() == 2);
            CHECK(item.referenceID() == 1);
            CHECK(item.orderUpdateAction() == cme_mdp::OrderUpdateAction::Value::New);
          });
          CHECK(no_order_id_intries_rows == 1);
          break;
        }
        case 2: {
          CHECK(book.transactTime() == 1659367870041633809);
          book.sbeRewind();  // wtf!
          book.noMDEntries().forEach([](auto &) { FAIL(); });
          book.noOrderIDEntries().forEach([](auto &) { FAIL(); });
          break;
        }
      }
    }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshOrderBook47> const &, sbe::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshTradeSummary48> const &, sbe::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshDailyStatistics49> const &, sbe::Frame const &) override {
      FAIL();
    }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshLimitsBanding50> const &, sbe::Frame const &) override {
      FAIL();
    }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshSessionStatistics51> const &, sbe::Frame const &) override {
      FAIL();
    }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshBookLongQty64> const &, sbe::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshTradeSummaryLongQty65> const &, sbe::Frame const &) override {
      FAIL();
    }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshVolumeLongQty66> const &, sbe::Frame const &) override {
      FAIL();
    }
    void operator()(
        Trace<cme_mdp::MDIncrementalRefreshSessionStatisticsLongQty67> const &, sbe::Frame const &) override {
      FAIL();
    }
  } handler;
  std::span buffer{reinterpret_cast<std::byte const *>(std::data(message)), std::size(message)};
  sbe::Parser::dispatch(handler, buffer, {});
  CHECK(handler.counter == 2);
}

TEST_CASE("md_incremental_refresh_book_46_test_2", "[sbe]") {
  auto message =
      "\xc9\xef\x0f\x00"
      "\x3e\x21\x5f\x40\x49\x42\x07\x17"
      // message #1
      "\x58\x00"
      "\x0b\x00"
      "\x2e\x00"
      "\x01\x00"
      "\x09\x00"
      "\x07\x93\x5d\x40\x49\x42\x07\x17"
      "\x04"
      "\x00\x00"
      "\x20\x00"
      "\x01"  // num in group (1)
      "\x00\xfe\x46\x07\x40\x00\x00\x00"
      "\x02\x00\x00\x00"
      "\x46\xd7\x04\x00"
      "\x92\x37\x00\x00"
      "\x02\x00\x00\x00"
      "\x01"
      "\x01"
      "\x30"  // entry type ('0' = bid)
      "\x00\x00\x00\x00"
      "\x00"
      "\x18\x00"
      "\x00\x00\x00\x00\x00"
      "\x01"
      "\x9e\x5f\x5e\xa4\xa4\x00\x00\x00"
      "\x83\x44\x46\xee\x03\x00\x00\x00"
      "\x01\x00\x00\x00"
      "\x01"
      "\x00\x00\x00"
      // message #2
      "\xe0\x00"
      "\x0b\x00"
      "\x2e\x00"
      "\x01\x00"
      "\x09\x00"
      "\x07\x93\x5d\x40\x49\x42\x07\x17"
      "\x90"
      "\x00\x00"
      "\x20\x00"
      "\x06"  // num in group (6)
      "\x00\x22\x49\x84\xbf\xfb\xff\xff"
      "\x01\x00\x00\x00"
      "\x1f\x37\x00\x00"
      "\x5e\x16\x00\x00"
      "\xff\xff\xff\x7f"
      "\x02"
      "\x00"
      "\x46"  // entry type ('F' = implied offer)
      "\x00\x00\x00\x00"
      "\x00"
      "\x00\x50\x7f\x09\xba\xfe\xff\xff"
      "\x01\x00\x00\x00"
      "\x7a\x63\x00\x00"
      "\x16\x0b\x00\x00"
      "\xff\xff\xff\x7f"
      "\x02"
      "\x00"
      "\x45"  // entry type ('E' = implied bid)
      "\x00\x00\x00\x00"
      "\x00"
      "\x00\xf0\xfa\x06\x39\x5c\x00\x00"
      "\x02\x00\x00\x00"
      "\xf6\x66\x00\x00"
      "\x73\x57\x00\x00"
      "\xff\xff\xff\x7f"
      "\x01"
      "\x01"
      "\x46"  // entry type ('F' = implied offer)
      "\x00\x00\x00\x00"
      "\x00"
      "\x00\xaa\x18\xd9\x3e\x5c\x00\x00"
      "\x01\x00\x00\x00"
      "\xf6\x66\x00\x00"
      "\x74\x57\x00\x00"
      "\xff\xff\xff\x7f"
      "\x02"
      "\x02"
      "\x46"  // entry type ('F' = implied offer)
      "\x00\x00\x00\x00"
      "\x00"
      "\x00\x64\x36\xab\x44\x5c\x00\x00"
      "\x05\x00\x00\x00"
      "\xf6\x66\x00\x00"
      "\x75\x57\x00\x00"
      "\xff\xff\xff\x7f"
      "\x02"
      "\x00"
      "\x46"  // entry type ('F' = implied offer)
      "\x00\x00\x00\x00"
      "\x00"
      "\x00\x24\xbc\x9a\x51\x01\x00\x00"
      "\x01\x00\x00\x00"
      "\x37\x22\x05\x00"
      "\x75\x08\x00\x00"
      "\xff\xff\xff\x7f"
      "\x02"
      "\x00"
      "\x45"  // entry type ('E' = implied bid)
      "\x00\x00\x00\x00"
      "\x00"
      "\x18\x00"
      "\x00\x00\x00\x00\x00"
      "\x00"sv;
  CHECK(std::size(message) == 324);
  struct MyHandler final : public sbe::Parser::Handler {
    int counter = 0;
    // - MDInstrumentDefinition
    void operator()(Trace<cme_mdp::MDInstrumentDefinitionFuture54> const &, sbe::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDInstrumentDefinitionOption55> const &, sbe::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDInstrumentDefinitionSpread56> const &, sbe::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDInstrumentDefinitionFixedIncome57> const &, sbe::Frame const &) override {
      FAIL();
    }
    void operator()(Trace<cme_mdp::MDInstrumentDefinitionRepo58> const &, sbe::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDInstrumentDefinitionFX63> const &, sbe::Frame const &) override { FAIL(); }
    // - SnapshotFullRefresh
    void operator()(Trace<cme_mdp::SnapshotFullRefresh52> const &, sbe::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::SnapshotFullRefreshOrderBook53> const &, sbe::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::SnapshotFullRefreshLongQty69> const &, sbe::Frame const &) override { FAIL(); }
    // - MDIncrementalRefresh
    void operator()(Trace<cme_mdp::MDIncrementalRefreshVolume37> const &, sbe::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshBook46> const &, sbe::Frame const &) override { ++counter; }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshOrderBook47> const &, sbe::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshTradeSummary48> const &, sbe::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshDailyStatistics49> const &, sbe::Frame const &) override {
      FAIL();
    }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshLimitsBanding50> const &, sbe::Frame const &) override {
      FAIL();
    }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshSessionStatistics51> const &, sbe::Frame const &) override {
      FAIL();
    }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshBookLongQty64> const &, sbe::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshTradeSummaryLongQty65> const &, sbe::Frame const &) override {
      FAIL();
    }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshVolumeLongQty66> const &, sbe::Frame const &) override {
      FAIL();
    }
    void operator()(
        Trace<cme_mdp::MDIncrementalRefreshSessionStatisticsLongQty67> const &, sbe::Frame const &) override {
      FAIL();
    }
  } handler;
  std::span buffer{reinterpret_cast<std::byte const *>(std::data(message)), std::size(message)};
  sbe::Parser::dispatch(handler, buffer, {});
  CHECK(handler.counter == 2);
}

TEST_CASE("md_instrument_definition_spread_56", "[sbe]") {
  // 10.25
  auto message =
      // seqno 9818
      "\x5a\x26\x00\x00"
      "\x27\x86\x0b\x6f\x86\x1f\x08\x17"
      // len 287, tid 56
      "\x1f\x01\xc3\x00\x38\x00\x01\x00\x09\x00\x00\xb8\xda\x00\x00\x41\x80\x72\x47\x21\x91\xf5\x06\x17\x11\x82\x01\x4e"
      "\x10\x58\x4e\x59\x4d\x50\x41\x00\x00\x00\x00\x33\x58\x57\x00\x00\x00\x33\x58\x57\x58\x34\x2d\x4f\x4d\x4c\x58\x34"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x78\x3d\x00\x00\x46\x55\x54\x00\x00\x00\x46\x4d\x4d\x58\x53\x58\xe8\x07\x0b"
      "\xff\xff\x55\x53\x44\x49\x53\x00\x00\x00\x4e\x4b\x01\x00\x00\x00\xe7\x03\x00\x00\x00\xca\x9a\x3b\x00\x00\x00\x00"
      "\x80\x96\x98\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\x7f\x7f\x4d\x57\x48\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x50\x99\x7a\xd2\x00\x00\x00"
      "\x01\xff\xff\xff\x7f\xff\xff\xff\x7f\xff\xff\xff\xff\xff\xff\xff\x7f\xff\xff\xff\xff\xff\xff\xff\x7f\x00\xca\x9a"
      "\x3b\x00\x00\x00\x00\xff\xff\x02\x4b\x09\x00\x02\x05\x00\x70\x91\xd8\x1c\x4f\xb2\x16\x07\x00\x20\x0b\xfd\xb0\x56"
      "\x03\x18\x04\x00\x01\x47\x42\x58\x0a\x04\x00\x01\x03\x24\x05\x00\x05\x00\x01\x04\x90\xd0\x03\x00\x12\x00\x02\xf8"
      "\x50\x00\x00\x01\x01\xff\xff\xff\xff\xff\xff\xff\x7f\xff\xff\xff\x7f\x09\x54\x00\x00\x02\x01\xff\xff\xff\xff\xff"
      "\xff\xff\x7f\xff\xff\xff\x7f"
      // len 286, tid 56
      "\x1e\x01\xc3\x00\x38\x00\x01\x00\x09\x00\x00\xb8\xda\x00\x00\x41\x80\x72\x47\x21\x91\xf5\x06\x17\x11\x82\x01\x4e"
      "\x10\x58\x4e\x59\x4d\x47\x55\x00\x00\x00\x00\x4a\x4b\x4d\x00\x00\x00\x4a\x4b\x4d\x4b\x33\x2d\x4a\x4b\x4d\x4b\x34"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\xb2\x6e\x04\x00\x46\x55\x54\x00\x00\x00\x46\x4d\x4d\x58\x53\x58\xe7\x07\x05"
      "\xff\xff\x55\x53\x44\x53\x50\x00\x00\x00\x4e\x46\x01\x00\x00\x00\xe7\x03\x00\x00\x00\xf2\x05\x2a\x01\x00\x00\x00"
      "\x40\x42\x0f\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\x7f\x7f\x43\x54\x52\x43\x54\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x38\x39\xa2\x88\x0e\x00\x00"
      "\x01\xff\xff\xff\x7f\xff\xff\xff\x7f\xff\xff\xff\xff\xff\xff\xff\x7f\xff\xff\xff\xff\xff\xff\xff\x7f\x00\xe8\x76"
      "\x48\x17\x00\x00\x00\xff\xff\x02\x4b\x09\x00\x02\x05\x00\x70\xf7\x6b\x0e\x67\xeb\x16\x07\x00\x90\xe2\xcf\x9c\xe0"
      "\x55\x17\x04\x00\x02\x47\x42\x58\x0a\x47\x42\x49\x02\x04\x00\x01\x4b\x64\x0c\x00\x05\x00\x00\x12\x00\x02\xf2\x3e"
      "\x00\x00\x01\x01\xff\xff\xff\xff\xff\xff\xff\x7f\xff\xff\xff\x7f\x72\x43\x00\x00\x02\x01\xff\xff\xff\xff\xff\xff"
      "\xff\x7f\xff\xff\xff\x7f"
      // len 282, tid 56
      "\x1a\x01\xc3\x00\x38\x00\x01\x00\x09\x00\x00\xb8\xda\x00\x00\x41\x80\x72\x47\x21\x91\xf5\x06\x17\x11\x82\x01\x4e"
      "\x10\x58\x4e\x59\x4d\x46\x44\x00\x00\x00\x00\x54\x54\x42\x00\x00\x00\x54\x54\x42\x5a\x32\x2d\x54\x54\x42\x51\x36"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\xd7\x74\x07\x00\x46\x55\x54\x00\x00\x00\x46\x4d\x4d\x58\x53\x58\xe6\x07\x0c"
      "\xff\xff\x55\x53\x44\x53\x50\x00\x00\x00\x4e\x46\x01\x00\x00\x00\xe7\x03\x00\x00\x00\xca\x9a\x3b\x00\x00\x00\x00"
      "\x40\x42\x0f\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\x7f\x7f\x4d\x4d\x42\x54\x55\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x30\x21\x42\x25\x29\x00\x00"
      "\x03\xff\xff\xff\x7f\xff\xff\xff\x7f\xff\xff\xff\xff\xff\xff\xff\x7f\xff\xff\xff\xff\xff\xff\xff\x7f\x00\xb6\x17"
      "\xab\x0e\x00\x00\x00\xff\xff\x07\x4b\x09\x00\x02\x05\x00\x70\x50\xd7\xb1\x3c\x90\x16\x07\x00\x60\x29\x79\x6b\x30"
      "\x2c\x17\x04\x00\x01\x47\x42\x58\x0a\x04\x00\x01\x4b\x64\x04\x00\x05\x00\x00\x12\x00\x02\xf9\xe6\x06\x00\x01\x01"
      "\xff\xff\xff\xff\xff\xff\xff\x7f\xff\xff\xff\x7f\x5c\xb4\x03\x00\x02\x01\xff\xff\xff\xff\xff\xff\xff\x7f\xff\xff"
      "\xff\x7f"
      // len 282, tid 56
      "\x1a\x01\xc3\x00\x38\x00\x01\x00\x09\x00\x00\xb8\xda\x00\x00\x41\x80\x72\x47\x21\x91\xf5\x06\x17\x11\x82\x01\x4e"
      "\x0c\x58\x4e\x59\x4d\x5a\x5a\x00\x00\x00\x00\x41\x37\x51\x00\x00\x00\x41\x37\x51\x46\x33\x2d\x41\x37\x51\x4d\x34"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\xfb\x66\x05\x00\x46\x55\x54\x00\x00\x00\x46\x4d\x58\x58\x53\x58\xe7\x07\x01"
      "\xff\xff\x55\x53\x44\x53\x50\x00\x00\x00\x4e\x46\x01\x00\x00\x00\xe7\x03\x00\x00\x00\xca\x9a\x3b\x00\x00\x00\x00"
      "\x10\x27\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\x7f\x7f\x47\x41\x4c\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x90\x12\x4d\x7a\x13\x00\x00"
      "\x03\xff\xff\xff\x7f\xff\xff\xff\x7f\xff\xff\xff\xff\xff\xff\xff\x7f\xff\xff\xff\xff\xff\xff\xff\x7f\x00\x98\xf7"
      "\x3e\x5d\x01\x00\x00\xff\xff\x07\x4b\x09\x00\x02\x05\x00\x70\xea\xe1\xe2\x4d\xf4\x16\x07\x00\x30\x14\x91\x85\x7b"
      "\x3f\x17\x04\x00\x01\x47\x42\x58\x0a\x04\x00\x01\x03\x24\x04\x00\x05\x00\x00\x12\x00\x02\x4a\xe4\x06\x00\x01\x01"
      "\xff\xff\xff\xff\xff\xff\xff\x7f\xff\xff\xff\x7f\x54\xc2\x01\x00\x02\x01\xff\xff\xff\xff\xff\xff\xff\x7f\xff\xff"
      "\xff\x7f"sv;
  CHECK(std::size(message) == 1149);
  struct MyHandler final : public sbe::Parser::Handler {
    int counter = 0;
    // - MDInstrumentDefinition
    void operator()(Trace<cme_mdp::MDInstrumentDefinitionFuture54> const &, sbe::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDInstrumentDefinitionOption55> const &, sbe::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDInstrumentDefinitionSpread56> const &, sbe::Frame const &) override { ++counter; }
    void operator()(Trace<cme_mdp::MDInstrumentDefinitionFixedIncome57> const &, sbe::Frame const &) override {
      FAIL();
    }
    void operator()(Trace<cme_mdp::MDInstrumentDefinitionRepo58> const &, sbe::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDInstrumentDefinitionFX63> const &, sbe::Frame const &) override { FAIL(); }
    // - SnapshotFullRefresh
    void operator()(Trace<cme_mdp::SnapshotFullRefresh52> const &, sbe::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::SnapshotFullRefreshOrderBook53> const &, sbe::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::SnapshotFullRefreshLongQty69> const &, sbe::Frame const &) override { FAIL(); }
    // - MDIncrementalRefresh
    void operator()(Trace<cme_mdp::MDIncrementalRefreshVolume37> const &, sbe::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshBook46> const &, sbe::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshOrderBook47> const &, sbe::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshTradeSummary48> const &, sbe::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshDailyStatistics49> const &, sbe::Frame const &) override {
      FAIL();
    }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshLimitsBanding50> const &, sbe::Frame const &) override {
      FAIL();
    }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshSessionStatistics51> const &, sbe::Frame const &) override {
      FAIL();
    }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshBookLongQty64> const &, sbe::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshTradeSummaryLongQty65> const &, sbe::Frame const &) override {
      FAIL();
    }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshVolumeLongQty66> const &, sbe::Frame const &) override {
      FAIL();
    }
    void operator()(
        Trace<cme_mdp::MDIncrementalRefreshSessionStatisticsLongQty67> const &, sbe::Frame const &) override {
      FAIL();
    }
  } handler;
  std::span buffer{reinterpret_cast<std::byte const *>(std::data(message)), std::size(message)};
  auto result = sbe::Parser::dispatch(handler, buffer, {});
  CHECK(result == true);
  CHECK(handler.counter == 4);
}
