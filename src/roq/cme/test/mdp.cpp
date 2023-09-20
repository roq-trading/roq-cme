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

TEST_CASE("md_incremental_refresh_book_46_test_1", "[mdp3]") {
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
    void operator()(Trace<cme_mdp::MDIncrementalRefreshBook46> const &event, mdp::Frame const &frame) override {
      CHECK(frame.sequence_number == 1044422);
      CHECK(frame.sending_time == 1659367870041719045ns);
      using value_type = std::remove_cvref<decltype(event)>::type::value_type;
      auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
      switch (++counter) {
        case 1: {
          CHECK(value.transactTime() == 1659367870041633809);
          value.sbeRewind();  // wtf!
          auto no_md_entries_rows = 0;
          value.noMDEntries().forEach([&no_md_entries_rows](auto &item) {
            ++no_md_entries_rows;
            CHECK(mdp::get_double(item.mDEntryPx()) == 66735.0_a);
            CHECK(item.mDEntrySize() == 2);
            CHECK(item.securityID() == 30975);
            CHECK(item.rptSeq() == 80734);
            CHECK(item.numberOfOrders() == 1);
            CHECK(item.mDPriceLevel() == 2);
            CHECK(item.mDUpdateAction() == cme_mdp::MDUpdateAction::Value::New);
            CHECK(item.mDEntryType() == cme_mdp::MDEntryTypeBook::Value::Offer);
            CHECK(item.tradeableSize() == cme_mdp::MDIncrementalRefreshBook46::NoMDEntries::tradeableSizeNullValue());
          });
          CHECK(no_md_entries_rows == 1);
          auto no_order_id_intries_rows = 0;
          value.noOrderIDEntries().forEach([&no_order_id_intries_rows](auto &item) {
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
          CHECK(value.transactTime() == 1659367870041633809);
          value.sbeRewind();  // wtf!
          value.noMDEntries().forEach([](auto &) { FAIL(); });
          value.noOrderIDEntries().forEach([](auto &) { FAIL(); });
          break;
        }
      }
    }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshOrderBook47> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshTradeSummary48> const &, mdp::Frame const &) override { FAIL(); }
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
  mdp::Parser::dispatch(handler, buffer, trace_info);
  CHECK(handler.counter == 2);
}

TEST_CASE("md_incremental_refresh_book_46_test_2", "[mdp3]") {
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
    void operator()(Trace<cme_mdp::MDIncrementalRefreshBook46> const &, mdp::Frame const &) override { ++counter; }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshOrderBook47> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshTradeSummary48> const &, mdp::Frame const &) override { FAIL(); }
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
  mdp::Parser::dispatch(handler, buffer, trace_info);
  CHECK(handler.counter == 2);
}

TEST_CASE("md_instrument_definition_future_54", "[mdp3]") {
  auto message =
      "\xb0\x09\x00\x00"
      "\x2e\xc2\x7d\x1a\xcc\x24\x08\x17"
      // len 269, tid 54
      // message header
      "\x0d\x01"  // message size (269)
      "\xd8\x00"  // block length (224)
      "\x36\x00"  // template id (54)
      "\x01\x00"  // schema id (1)
      "\x09\x00"  // version (9)
      // MDInstrumentDefinitionFuture54
      "\x00"                              // match event indicator
      "\xb8\xda\x00\x00"                  // total number of reports
      "\x41"                              // security update action
      "\x00\x32\x76\x41\x8f\xf5\x06\x17"  // last update time
      "\x11"                              // md security trading status (17 = ready to trade)
      "\x82\x01"                          // appl id
      "\x4e"                              // market segment id
      "\x10"                              // underlying product
      "\x58\x4e\x59\x4d"                  // security exchange
      "\x5a\x5a\x00\x00\x00\x00"          // security group
      "\x41\x34\x4d\x00\x00\x00"          // asset
      "\x41\x34\x4d\x4e\x36\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"  // symbol
      "\x7d\x68\x02\x00"                                                                  // security id
                          // note! security id source is a constant
      "\x46\x55\x54\x00\x00\x00"          // security type
      "\x46\x43\x4d\x58\x53\x58"          // CFI code
      "\xea\x07\x07\xff\xff"              // maturity month year
      "\x55\x53\x44"                      // currency
      "\x00\x00\x00"                      // settl currency
      "\x46"                              // match algorithm
      "\x01\x00\x00\x00"                  // min trade vol
      "\xe7\x03\x00\x00"                  // max trade vol
      "\x00\xf2\x05\x2a\x01\x00\x00\x00"  // min price increment
      "\x80\x96\x98\x00\x00\x00\x00\x00"  // display factor
      "\xff"                              // main fraction
      "\xff"                              // sub fraction
      "\xff"                              // price display format
      "\x4d\x57\x48\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00"                          // unit of measure
      "\x00\xf2\x05\x2a\x01\x00\x00\x00"  // unit of measure qty
      "\x00\x96\x1b\x55\x86\x04\x00\x00"  // trading reference price
      "\x05"                              // settl price type
      "\xff\xff\xff\x7f"                  // open interest qty
      "\xff\xff\xff\x7f"                  // cleared volume
      "\xff\xff\xff\xff\xff\xff\xff\x7f"  // high limit price
      "\x00\xf2\x05\x2a\x01\x00\x00\x00"  // low limit price
      "\x00\xf2\x05\x2a\x01\x00\x00\x00"  // max price variation
      "\xff\xff\xff\x7f"                  // decay quantity
      "\xff\xff"                          // decay start date
      "\x05\x00\x00\x00"                  // original contract size
      "\xb0\x0e\x00\x00"                  // contract multiplier
      "\x01"                              // contract multiplier unit
      "\x7f"                              // flow schedule type
      "\x00\x94\x35\x77\x00\x00\x00\x00"  // min price increment amount
      "\x4e"                              // user defined instrument (N)
      "\x02\x4b"                          // trading reference date
      "\x09\x00\x02\x05\x00\x10\x67\x2f"  // instrument guid
      // - NoEvents ? ... doesn't look right
      "\xb4\xed\x55"
      "\x16"
      "\x07\x00\x20\xab\x18\x8a\xf7\xbd"
      "\x18\x04\x00\x01\x47\x42"
      "\x58\x0a\x04\x00\x01\x07\x20\x01\x00\x05\x00\x01\x04\x20\xa1\x07\x00"
      // len 265, tid 54
      "\x08\x01\xd8\x00\x36\x00\x01\x00\x09\x00\x00\xb8\xda\x00\x00\x41\x00\x32\x76\x41\x8f\xf5\x06\x17\x11\x82\x01\x4e"
      "\x0c\x58\x4e\x59\x4d\x5a\x5a\x00\x00\x00\x00\x41\x33\x4d\x00\x00\x00\x41\x33\x4d\x56\x32\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x44\x99\x00\x00\x46\x55\x54\x00\x00\x00\x46\x43\x58\x58\x53\x58\xe6\x07\x0a"
      "\xff\xff\x55\x53\x44\x00\x00\x00\x46\x01\x00\x00\x00\xe7\x03\x00\x00\x00\xca\x9a\x3b\x00\x00\x00\x00\x10\x27\x00"
      "\x00\x00\x00\x00\x00\xff\xff\xff\x47\x41\x4c\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xa0\x14\xe3\x32\x26\x00\x00\x00\x54\x56\x5c\x8d\x75\x00\x00\x01\xff"
      "\xff\xff\x7f\xff\xff\xff\x7f\xff\xff\xff\xff\xff\xff\xff\x7f\x00\x00\x00\x00\x00\x00\x00\x00\x00\x98\xf7\x3e\x5d"
      "\x01\x00\x00\xff\xff\xff\x7f\xff\xff\xff\xff\xff\x7f\xff\xff\xff\x7f\x7f\x7f\x00\xb1\x08\x19\x00\x00\x00\x00\x4e"
      "\x02\x4b\x09\x00\x02\x05\x00\x70\x9a\x0e\x0b\x8a\xd2\x15\x07\x00\x90\x0a\x74\x1d\x4f\x22\x17\x04\x00\x01\x47\x42"
      "\x58\x0a\x04\x00\x01\x07\x20\x04\x00\x05\x00\x00"
      // len 269, tid 54
      "\x0d\x01\xd8\x00\x36\x00\x01\x00\x09\x00\x00\xb8\xda\x00\x00\x41\x00\x32\x76\x41\x8f\xf5\x06\x17\x11\x82\x01\x4e"
      "\x10\x58\x4e\x59\x4d\x5a\x5a\x00\x00\x00\x00\x41\x48\x4d\x00\x00\x00\x41\x48\x4d\x4b\x33\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\xf2\x91\x02\x00\x46\x55\x54\x00\x00\x00\x46\x43\x4d\x58\x53\x58\xe7\x07\x05"
      "\xff\xff\x55\x53\x44\x00\x00\x00\x46\x01\x00\x00\x00\xe7\x03\x00\x00\x00\xf2\x05\x2a\x01\x00\x00\x00\x80\x96\x98"
      "\x00\x00\x00\x00\x00\xff\xff\xff\x4d\x57\x48\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x20\x5f\xa0\x12\x00\x00\x00\x00\x00\xb3\x20\x67\x06\x00\x00\x05\xff"
      "\xff\xff\x7f\xff\xff\xff\x7f\xff\xff\xff\xff\xff\xff\xff\x7f\x00\xf2\x05\x2a\x01\x00\x00\x00\x00\xf2\x05\x2a\x01"
      "\x00\x00\x00\xff\xff\xff\x7f\xff\xff\x50\x00\x00\x00\xdc\x00\x00\x00\x01\x7f\x00\x94\x35\x77\x00\x00\x00\x00\x4e"
      "\x02\x4b\x09\x00\x02\x05\x00\x10\x91\xf2\x7c\xe4\x04\x15\x07\x00\x28\xc7\x98\xc9\x4b\x5a\x17\x04\x00\x01\x47\x42"
      "\x58\x0a\x04\x00\x01\x07\x20\x05\x00\x05\x00\x01\x04\x20\xa1\x07\x00"
      // len 269, tid 54
      "\x0d\x01\xd8\x00\x36\x00\x01\x00\x09\x00\x00\xb8\xda\x00\x00\x41\x00\x32\x76\x41\x8f\xf5\x06\x17\x11\x82\x01\x4e"
      "\x10\x58\x4e\x59\x4d\x50\x41\x00\x00\x00\x00\x43\x32\x45\x00\x00\x00\x43\x32\x45\x58\x36\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\xe2\x26\x01\x00\x46\x55\x54\x00\x00\x00\x46\x43\x4d\x58\x53\x58\xea\x07\x0b"
      "\xff\xff\x55\x53\x44\x00\x00\x00\x4b\x01\x00\x00\x00\xe7\x03\x00\x00\x00\xca\x9a\x3b\x00\x00\x00\x00\x80\x96\x98"
      "\x00\x00\x00\x00\x00\xff\xff\xff\x4d\x57\x48\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xf2\x05\x2a\x01\x00\x00\x00\x00\x54\x02\xec\x53\x05\x00\x00\x01\xff"
      "\xff\xff\x7f\xff\xff\xff\x7f\xff\xff\xff\xff\xff\xff\xff\x7f\x00\xca\x9a\x3b\x00\x00\x00\x00\x00\xf2\x05\x2a\x01"
      "\x00\x00\x00\xff\xff\xff\x7f\xff\xff\x05\x00\x00\x00\x95\x06\x00\x00\x01\x7f\x00\x94\x35\x77\x00\x00\x00\x00\x4e"
      "\x02\x4b\x09\x00\x02\x05\x00\x10\xd6\x96\xb6\x69\x4c\x16\x07\x00\x20\x51\x58\x57\x6a\xe3\x18\x04\x00\x01\x47\x42"
      "\x58\x0a\x04\x00\x01\x07\x20\x05\x00\x05\x00\x01\x04\x20\xa1\x07\x00"
      // len 269, tid 54
      "\x0d\x01\xd8\x00\x36\x00\x01\x00\x09\x00\x00\xb8\xda\x00\x00\x41\x00\x32\x76\x41\x8f\xf5\x06\x17\x11\x82\x01\x4e"
      "\x10\x58\x4e\x59\x4d\x50\x57\x00\x00\x00\x00\x4e\x43\x44\x00\x00\x00\x4e\x43\x44\x51\x32\x32\x38\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\xaf\x4f\x02\x00\x46\x55\x54\x00\x00\x00\x46\x43\x4d\x58\x53\x58\xe6\x07\x08"
      "\x1c\xff\x55\x53\x44\x00\x00\x00\x46\x01\x00\x00\x00\xe7\x03\x00\x00\x00\xca\x9a\x3b\x00\x00\x00\x00\x80\x96\x98"
      "\x00\x00\x00\x00\x00\xff\xff\xff\x4d\x57\x48\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xf2\x05\x2a\x01\x00\x00\x00\x00\xa6\xb9\x2c\x04\x06\x00\x00\x01\xff"
      "\xff\xff\x7f\xff\xff\xff\x7f\xff\xff\xff\xff\xff\xff\xff\x7f\xff\xff\xff\xff\xff\xff\xff\x7f\x00\xd0\xed\x90\x2e"
      "\x00\x00\x00\xff\xff\xff\x7f\xff\xff\x05\x00\x00\x00\x01\x00\x00\x00\x01\x7f\x80\xf0\xfa\x02\x00\x00\x00\x00\x4e"
      "\x02\x4b\x09\x00\x02\x05\x00\x70\x2c\xe9\x4b\x83\xfd\x16\x07\x00\x20\x67\x2b\xbd\x00\x0f\x17\x04\x00\x01\x47\x42"
      "\x58\x0a\x04\x00\x01\x4f\x24\x07\x00\x05\x00\x01\x04\x50\xc3\x00\x00"sv;
  CHECK(std::size(message) == 1352);
  struct MyHandler final : public mdp::Parser::Handler {
    int counter = 0;
    void operator()(mdp::Frame const &) override {}
    void operator()(Trace<cme_mdp::ChannelReset4> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::AdminHeartbeat12> const &, mdp::Frame const &) override { FAIL(); }
    // - security status
    void operator()(Trace<cme_mdp::SecurityStatus30> const &, mdp::Frame const &) override { FAIL(); }
    // - instrument definition
    void operator()(Trace<cme_mdp::MDInstrumentDefinitionFuture54> const &event, mdp::Frame const &) override {
      ++counter;
      using value_type = std::remove_cvref<decltype(event)>::type::value_type;
      auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
      auto tmp = fmt::format("{}"sv, value);
    }
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
    void operator()(Trace<cme_mdp::MDIncrementalRefreshTradeSummary48> const &, mdp::Frame const &) override { FAIL(); }
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
  auto result = mdp::Parser::dispatch(handler, buffer, trace_info);
  CHECK(result == true);
  CHECK(handler.counter == 5);
}

TEST_CASE("md_instrument_definition_spread_56", "[mdp3]") {
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
    void operator()(Trace<cme_mdp::MDInstrumentDefinitionSpread56> const &, mdp::Frame const &) override { ++counter; }
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
    void operator()(Trace<cme_mdp::MDIncrementalRefreshTradeSummary48> const &, mdp::Frame const &) override { FAIL(); }
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
  auto result = mdp::Parser::dispatch(handler, buffer, trace_info);
  CHECK(result == true);
  CHECK(handler.counter == 4);
}

TEST_CASE("md_md_incremental_refresh_book_46_test_1", "[mdp3]") {
  auto message =
      "\xe0\x15\xcc\x17"
      "\x00\x64\xec\x6e\x82\x65\x08\x17"
      "\x58\x00"                          // message size (88)
      "\x0b\x00"                          // block length (11)
      "\x2e\x00"                          // template id (46 = MDIncrementalRefreshBook46)
      "\x01\x00"                          // schema id (1)
      "\x09\x00"                          // version (9)
      "\x0f\xf0\xea\x6e\x82\x65\x08\x17"  // transact time
      "\x04"                              // match event indicator
      "\x00\x00"                          // padding (block length)
      "\x20\x00"                          // block length (32)
      "\x01"                              // group size (1)
      "\x00\x25\x74\xaa\xf8\xff\xff\xff"  // md entry px
      "\x34\x03\x00\x00"                  // md entry size
      "\xde\x57\x00\x00"                  // security id
      "\xc2\xb5\x4d\x00"                  // rpt seq
      "\x11\x00\x00\x00"                  // number of orders
      "\x01"                              // md price level
      "\x01"                              // md update action
      "\x30"                              // md entry type book
      "\x00\x00\x00\x00"                  // tradeable size
      "\x00"                              // padding (block length)
      "\x18\x00"                          // block length (24)
      "\x00\x00\x00\x00\x00"              // padding
      "\x01"                              // group size (1)
      "\x24\xae\xc8\xc5\x77\x07\x00\x00"  // order id
      "\xa9\x88\x92\xf6\x0b\x00\x00\x00"  // md order priority
      "\x6e\x00\x00\x00"                  // md display qty
      "\x01"                              // reference id
      "\x00"                              // order update action
      "\x00\x00"                          // padding (block length)
      "\x60\x00"                          // message size (96)
      "\x0b\x00"                          // block length (11)
      "\x2e\x00"                          // template id (46 = MDIncrementalRefreshBook46)
      "\x01\x00"                          // schema id (1)
      "\x09\x00"                          // version (9)
      "\x0f\xf0\xea\x6e\x82\x65\x08\x17\x90\x00\x00\x20\x00\x02\x00\xe1\x14\x10\xfa\xff\xff\xff\xf2\x00\x00\x00\x5f\x54"
      "\x00\x00\x6b\xd9\x22\x00\xff\xff\xff\x7f\x02\x01\x46\x00\x00\x00\x00\x00\x00\xd3\x1a\x3a\xfb\xff\xff\xff\x35\x03"
      "\x00\x00\x34\x71\x00\x00\x41\xda\x2d\x00\xff\xff\xff\x7f\x01\x01\x45\x00\x00\x00\x00\x00\x18\x00\x00\x00\x00\x00"
      "\x00\x00"sv;
  CHECK(std::size(message) == 196);
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
    void operator()(Trace<cme_mdp::MDIncrementalRefreshBook46> const &event, mdp::Frame const &) override {
      ++counter;
      using value_type = std::remove_cvref<decltype(event)>::type::value_type;
      auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
      fmt::print("{}\n"sv, value);
    }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshOrderBook47> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshTradeSummary48> const &, mdp::Frame const &) override { FAIL(); }
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
  auto result = mdp::Parser::dispatch(handler, buffer, trace_info);
  CHECK(result == true);
  CHECK(handler.counter == 2);
}

TEST_CASE("md_md_incremental_refresh_book_46_test_2", "[mdp3]") {
  /*
  auto message =
      "\xe0\x15\xcc\x17"
      "\x00\x64\xec\x6e\x82\x65\x08\x17"
      "\x58\x00"                          // message size (88)
      "\x0b\x00"                          // block length (11)
      "\x2e\x00"                          // template id (46 = MDIncrementalRefreshBook46)
      "\x01\x00"                          // schema id (1)
      "\x09\x00"                          // version (9)
      "\x0f\xf0\xea\x6e\x82\x65\x08\x17"  // transact time
      "\x04"                              // match event indicator
      "\x00\x00"                          // padding (block length)
      "\x20\x00"                          // block length (32)
      "\x01"                              // group size (1)
      "\x00\x25\x74\xaa\xf8\xff\xff\xff"  // md entry px
      "\x34\x03\x00\x00"                  // md entry size
      "\xde\x57\x00\x00"                  // security id
      "\xc2\xb5\x4d\x00"                  // rpt seq
      "\x11\x00\x00\x00"                  // number of orders
                                          // */
  auto message =
      "\xaa\xdd\x0b\x18"
      "\xf5\xa3\x09\x8f\x63\x69\x08\x17"
      "\x58\x00"
      "\x0b\x00"
      "\x2e\x00"
      "\x01\x00"
      "\x09\x00"
      "\xd1\x15\x08\x8f\x63\x69\x08\x17\x04\x00\x00\x20\x00\x01\x00\xf9\xfe\x8e\xd9\x08\x00\x00\x2c\x02\x00\x00\xbc\x53\x00\x00\x30\xee\xb3\x03\x12\x00\x00\x00\x01\x01\x31\x00\x00\x00\x00\x00\x18\x00\x00\x00\x00\x00\x00\x01\x3a\xaa\xcc\xc5\x77\x07\x00\x00\x95\xf0\xcd\xf6\x0b\x00\x00\x00\xc4\x00\x00\x00\x01\x01\x00\x00\xe0\x02\x0b\x00\x2e\x00\x01\x00\x09\x00\xd1\x15\x08\x8f\x63\x69\x08\x17\x90\x00\x00\x20\x00\x16\x00\x09\x7c\xf8\xd3\x08\x00\x00\xaa\x10\x00\x00\xab\x01\x00\x00\x5f\x6c\xdd\x02\xff\xff\xff\x7f\x02\x01\x46\x00\x00\x00\x00\x00\x00\x31\xeb\xec\xfb\xff\xff\xff\x2c\x02\x00\x00\x21\x02\x00\x00\x99\xf5\x7a\x00\xff\xff\xff\x7f\x01\x01\x46\x00\x00\x00\x00\x00\x00\x15\xf3\x3a\xd7\x08\x00\x00\xd8\x03\x00\x00\xb8\x15\x00\x00\xda\x3c\xef\x02\xff\xff\xff\x7f\x01\x01\x46\x00\x00\x00\x00\x00\x00\x31\xeb\xec\xfb\xff\xff\xff\x2a\x01\x00\x00\xf0\x1b\x00\x00\x62\x7d\x8d\x00\xff\xff\xff\x7f\x02\x01\x46\x00\x00\x00\x00\x00\x00\x35\xed\x0d\xcc\x08\x00\x00\x0a\x09\x00\x00\xc6\x44\x00\x00\xdc\xa7\x8f\x01\xff\xff\xff\x7f\x01\x01\x46\x00\x00\x00\x00\x00\x00\x9a\xba\x2b\xcc\x08\x00\x00\x77\x08\x00\x00\xc6\x44\x00\x00\xdd\xa7\x8f\x01\xff\xff\xff\x7f\x02\x01\x46\x00\x00\x00\x00\x00\x00\x4e\xee\x7b\xe8\xff\xff\xff\x2c\x02\x00\x00\x7e\x4a\x00\x00\xf2\xb3\x97\x00\xff\xff\xff\x7f\x01\x01\x45\x00\x00\x00\x00\x00\x00\xe9\x20\x5e\xe8\xff\xff\xff\x1b\x00\x00\x00\x7e\x4a\x00\x00\xf3\xb3\x97\x00\xff\xff\xff\x7f\x02\x01\x45\x00\x00\x00\x00\x00\x00\xdc\x9a\x38\xf6\xff\xff\xff\xba\x00\x00\x00\xd0\x50\x00\x00\x91\x55\x51\x00\xff\xff\xff\x7f\x02\x01\x45\x00\x00\x00\x00\x00\x00\x0d\x86\x25\xf2\xff\xff\xff\xc0\x02\x00\x00\x8c\x54\x00\x00\x7d\x22\x5b\x00\xff\xff\xff\x7f\x02\x01\x45\x00\x00\x00\x00\x00\x00\x6a\xe2\x27\xe6\xff\xff\xff\x2c\x02\x00\x00\xcc\x76\x00\x00\x14\x73\x89\x00\xff\xff\xff\x7f\x01\x01\x45\x00\x00\x00\x00\x00\x00\x3d\x62\x2f\xff\xff\xff\xff\xee\x01\x00\x00\x9a\x81\x00\x00\x98\xb6\x41\x00\xff\xff\xff\x7f\x01\x01\x46\x00\x00\x00\x00\x00\x00\xa2\x2f\x4d\xff\xff\xff\xff\x3e\x00\x00\x00\x9a\x81\x00\x00\x99\xb6\x41\x00\xff\xff\xff\x7f\x02\x01\x46\x00\x00\x00\x00\x00\x00\x58\xe2\x2a\xf0\xff\xff\xff\x84\x01\x00\x00\x07\x84\x00\x00\x79\x50\xbf\x00\xff\xff\xff\x7f\x02\x01\x45\x00\x00\x00\x00\x00\x00\x05\x76\xd1\xdc\x08\x00\x00\xf0\x0c\x00\x00\x4d\xdc\x00\x00\x5a\xaa\x89\x04\xff\xff\xff\x7f\x02\x01\x46\x00\x00\x00\x00\x00\x00\xc8\x13\xa2\xdd\x08\x00\x00\x12\x0b\x00\x00\x9f\x14\x01\x00\xa9\x56\xb3\x04\xff\xff\xff\x7f\x01\x01\x46\x00\x00\x00\x00\x00\x00\xf7\x7b\xfb\xdd\x08\x00\x00\x7f\x0b\x00\x00\x76\xb3\x01\x00\xdd\x84\x9a\x04\xff\xff\xff\x7f\x02\x01\x46\x00\x00\x00\x00\x00\x00\x60\x53\x46\xfc\xff\xff\xff\x88\x00\x00\x00\x21\xc6\x01\x00\x00\x80\x85\x00\xff\xff\xff\x7f\x02\x01\x46\x00\x00\x00\x00\x00\x00\xe4\x07\x4e\xdb\x08\x00\x00\x98\x0c\x00\x00\x26\xb5\x02\x00\xa5\xfc\x12\x04\xff\xff\xff\x7f\x02\x01\x46\x00\x00\x00\x00\x00\x00\x04\x02\x21\xd0\x08\x00\x00\xa9\x0b\x00\x00\x49\xf2\x02\x00\xdb\x98\x29\x02\xff\xff\xff\x7f\x02\x01\x46\x00\x00\x00\x00\x00\x00\x7d\xb7\x9c\xdf\x08\x00\x00\x3e\x08\x00\x00\x2d\xe1\x03\x00\x63\xfb\x62\x03\xff\xff\xff\x7f\x01\x01\x46\x00\x00\x00\x00\x00\x00\xe2\x84\xba\xdf\x08\x00\x00\xc8\x09\x00\x00\x2d\xe1\x03\x00\x64\xfb\x62\x03\xff\xff\xff\x7f\x02\x01\x46\x00\x00\x00\x00\x00\x18\x00\x00\x00\x00\x00\x00\x00"sv;
  CHECK(std::size(message) == 836);
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
    void operator()(Trace<cme_mdp::MDIncrementalRefreshBook46> const &event, mdp::Frame const &) override {
      ++counter;
      auto &value = event.value;
      auto tmp = fmt::format("{}"sv, const_cast<cme_mdp::MDIncrementalRefreshBook46 &>(value));
    }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshOrderBook47> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshTradeSummary48> const &, mdp::Frame const &) override { FAIL(); }
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
  auto result = mdp::Parser::dispatch(handler, buffer, trace_info);
  CHECK(result == true);
  CHECK(handler.counter == 2);
}

TEST_CASE("admin_heartbeat_12", "[mdp3]") {
  auto message =
      "\x07\x02\x00\x00"
      "\xbf\xa0\x13\xed\xfa\x0d\x09\x17"
      "\x0a\x00"
      "\x00\x00"
      "\x0c\x00"  // template id (12 = AdminHeartbeat12)
      "\x01\x00"
      "\x09\x00"sv;
  CHECK(std::size(message) == 22);
  struct MyHandler final : public mdp::Parser::Handler {
    int counter = 0;
    void operator()(mdp::Frame const &) override {}
    void operator()(Trace<cme_mdp::ChannelReset4> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::AdminHeartbeat12> const &event, mdp::Frame const &) override {
      ++counter;
      auto &value = event.value;
      auto tmp = fmt::format("{}"sv, value);
    }
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
    void operator()(Trace<cme_mdp::MDIncrementalRefreshTradeSummary48> const &, mdp::Frame const &) override { FAIL(); }
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
  auto result = mdp::Parser::dispatch(handler, buffer, trace_info);
  CHECK(result == true);
  CHECK(handler.counter == 1);
}

TEST_CASE("snapshot_full_refresh_52", "[mdp3]") {
  auto message =
      "\x04\x00\x00\x00"
      "\x94\xcd\xf6\xed\x1e\x49\x09\x17"
      "\xdc\x02"
      "\x3b\x00"
      "\x34\x00"  // template id (52 = SnapshotFullRefresh52)
      "\x01\x00"
      "\x09\x00"
      "\xd0\xfc\x07\x00\x0b\x00\x00\x00\xf3\x25\x03\x00\x90\x8c\x01\x00\x1b\xbf\x29\xd3\x1e\x49\x09\x17\x1f\x0d\x96"
      "\xab"
      "\x9c\x1b\x09\x17\x0c\x4b\x11\x00\x12\x5d\xdf\x33\x95\x01\x00\x00\x72\xf7\xb1\x73\x60\x01\x00\x00\x70\xc9\xb2"
      "\x8b"
      "\x00\x00\x00\x16\x00\x1e\x00\x70\x03\x3f\xe5\x7a\x01\x00\x02\x00\x00\x00\x02\x00\x00\x00\x01\xff\xff\xff\x80"
      "\x30"
      "\x00\xb6\xe5\x6c\xdf\x7a\x01\x00\x02\x00\x00\x00\x02\x00\x00\x00\x02\xff\xff\xff\x80\x30\x00\xfc\xc7\x9a\xd9"
      "\x7a"
      "\x01\x00\x01\x00\x00\x00\x01\x00\x00\x00\x03\xff\xff\xff\x80\x30\x00\x42\xaa\xc8\xd3\x7a\x01\x00\x01\x00\x00"
      "\x00"
      "\x01\x00\x00\x00\x04\xff\xff\xff\x80\x30\x00\x88\x8c\xf6\xcd\x7a\x01\x00\x0a\x00\x00\x00\x02\x00\x00\x00\x05"
      "\xff"
      "\xff\xff\x80\x30\x00\xce\x6e\x24\xc8\x7a\x01\x00\x01\x00\x00\x00\x01\x00\x00\x00\x06\xff\xff\xff\x80\x30\x00"
      "\x14"
      "\x51\x52\xc2\x7a\x01\x00\x02\x00\x00\x00\x02\x00\x00\x00\x07\xff\xff\xff\x80\x30\x00\x5a\x33\x80\xbc\x7a\x01"
      "\x00"
      "\x02\x00\x00\x00\x02\x00\x00\x00\x08\xff\xff\xff\x80\x30\x00\xa0\x15\xae\xb6\x7a\x01\x00\x03\x00\x00\x00\x03"
      "\x00"
      "\x00\x00\x09\xff\xff\xff\x80\x30\x00\xe6\xf7\xdb\xb0\x7a\x01\x00\x07\x00\x00\x00\x03\x00\x00\x00\x0a\xff\xff"
      "\xff"
      "\x80\x30\x00\x58\x7a\x87\xfc\x7a\x01\x00\x01\x00\x00\x00\x01\x00\x00\x00\x01\xff\xff\xff\x80\x31\x00\x12\x98"
      "\x59"
      "\x02\x7b\x01\x00\x02\x00\x00\x00\x02\x00\x00\x00\x02\xff\xff\xff\x80\x31\x00\xcc\xb5\x2b\x08\x7b\x01\x00\x01"
      "\x00"
      "\x00\x00\x01\x00\x00\x00\x03\xff\xff\xff\x80\x31\x00\x86\xd3\xfd\x0d\x7b\x01\x00\x01\x00\x00\x00\x01\x00\x00"
      "\x00"
      "\x04\xff\xff\xff\x80\x31\x00\x40\xf1\xcf\x13\x7b\x01\x00\x02\x00\x00\x00\x02\x00\x00\x00\x05\xff\xff\xff\x80"
      "\x31"
      "\x00\xfa\x0e\xa2\x19\x7b\x01\x00\x0a\x00\x00\x00\x02\x00\x00\x00\x06\xff\xff\xff\x80\x31\x00\xb4\x2c\x74\x1f"
      "\x7b"
      "\x01\x00\x04\x00\x00\x00\x03\x00\x00\x00\x07\xff\xff\xff\x80\x31\x00\x6e\x4a\x46\x25\x7b\x01\x00\x02\x00\x00"
      "\x00"
      "\x01\x00\x00\x00\x08\xff\xff\xff\x80\x31\x00\x28\x68\x18\x2b\x7b\x01\x00\x08\x00\x00\x00\x03\x00\x00\x00\x09"
      "\xff"
      "\xff\xff\x80\x31\x00\xe2\x85\xea\x30\x7b\x01\x00\x0c\x00\x00\x00\x02\x00\x00\x00\x0a\xff\xff\xff\x80\x31\x00"
      "\xfe"
      "\x80\x93\x99\x7a\x01\x00\xff\xff\xff\x7f\xff\xff\xff\x7f\x7f\xff\xff\xff\x80\x37\x00\xf0\x94\xb7\x70\x79\x01"
      "\x00"
      "\xff\xff\xff\x7f\xff\xff\xff\x7f\x7f\xff\xff\xff\x80\x38\x00\x40\xf1\xcf\x13\x7b\x01\x00\xff\xff\xff\x7f\xff"
      "\xff"
      "\xff\x7f\x7f\xff\xff\xff\x80\x4e\x00\xf0\x94\xb7\x70\x79\x01\x00\xff\xff\xff\x7f\xff\xff\xff\x7f\x7f\xff\xff"
      "\xff"
      "\x80\x4f\x00\xfe\x80\x93\x99\x7a\x01\x00\x02\x00\x00\x00\xff\xff\xff\x7f\x7f\xff\xff\xff\x80\x32\xff\xff\xff"
      "\xff"
      "\xff\xff\xff\x7f\x79\x00\x00\x00\xff\xff\xff\x7f\x7f\xff\xff\xff\x80\x65\x00\x00\x3a\x8c\x59\x7a\x01\x00\xff"
      "\xff"
      "\xff\x7f\xff\xff\xff\x7f\x7f\xff\xff\x00\x80\x34\xff\xff\xff\xff\xff\xff\xff\x7f\x6d\x0c\x00\x00\xff\xff\xff"
      "\x7f"
      "\x7f\x09\x4b\xff\x80\x42\xff\xff\xff\xff\xff\xff\xff\x7f\x5c\x87\x00\x00\xff\xff\xff\x7f\x7f\x09\x4b\xff\x80"
      "\x43"
      "\x00\x42\xaa\xc8\xd3\x7a\x01\x00\xff\xff\xff\x7f\xff\xff\xff\x7f\x7f\x09\x4b\xff\x03\x36"
      "\x3a\x01"
      "\x3b\x00"
      "\x34\x00"  // template id (52 = SnapshotFullRefresh52)
      "\x01\x00"
      "\x09\x00"
      "\xd0\xfc\x07\x00\x0b\x00\x00\x00\x4c\x41\x05\x00\xaa\x01\x00\x00\x77\xcf\x41\xfd\xf6\x48\x09\x17\x1f\x0d\x96\xab\x9c\x1b\x09\x17\x0c\x4b\x11\xff\xff\xff\xff\xff\xff\xff\x7f\xff\xff\xff\xff\xff\xff\xff\x7f\x00\x5c\xb2\xec\x22\x00\x00\x00\x16\x00\x0b\x00\xb0\x72\xfc\x6f\x02\x00\x00\x02\x00\x00\x00\x01\x00\x00\x00\x01\xff\xff\xff\x80\x30\x00\xbe\x6c\xd2\x6e\x02\x00\x00\x05\x00\x00\x00\x01\x00\x00\x00\x02\xff\xff\xff\x80\x30\x00\x2e\x3d\x82\x65\x02\x00\x00\x02\x00\x00\x00\x01\x00\x00\x00\x03\xff\xff\xff\x80\x30\x00\x5a\xe4\xfc\x21\x02\x00\x00\x05\x00\x00\x00\x01\x00\x00\x00\x04\xff\xff\xff\x80\x30\x00\xc8\x02\xb1\xc3\x01\x00\x00\x05\x00\x00\x00\x01\x00\x00\x00\x05\xff\xff\xff\x80\x30\x00\xec\xc5\x48\x80\x02\x00\x00\x05\x00\x00\x00\x01\x00\x00\x00\x01\xff\xff\xff\x80\x31\x00\xe6\xa1\x5b\xab\x02\x00\x00\x02\x00\x00\x00\x01\x00\x00\x00\x02\xff\xff\xff\x80\x31\x00\xda\x59\x81\x01\x03\x00\x00\x02\x00\x00\x00\x01\x00\x00\x00\x03\xff\xff\xff\x80\x31\x00\xb0\x72\xfc\x6f\x02\x00\x00\xff\xff\xff\x7f\xff\xff\xff\x7f\x7f\xff\xff\xff\x80\x4e\x00\xa2\x78\x26\x71\x02\x00\x00\xff\xff\xff\x7f\xff\xff\xff\x7f\x7f\xff\xff\xff\x80\x4f\x00\xec\xc5\x48\x80\x02\x00\x00\xff\xff\xff\x7f\xff\xff\xff\x7f\x7f\x09\x4b\xff\x03\x36"sv;
  CHECK(std::size(message) == 1058);
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
    void operator()(Trace<cme_mdp::SnapshotFullRefresh52> const &event, mdp::Frame const &) override {
      ++counter;
      using value_type = std::remove_cvref<decltype(event)>::type::value_type;
      auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
      auto tmp = fmt::format("{}"sv, value);
    }
    void operator()(Trace<cme_mdp::SnapshotFullRefreshOrderBook53> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::SnapshotFullRefreshLongQty69> const &, mdp::Frame const &) override { FAIL(); }
    // - MDIncrementalRefresh
    void operator()(Trace<cme_mdp::MDIncrementalRefreshVolume37> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshBook46> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshOrderBook47> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshTradeSummary48> const &, mdp::Frame const &) override { FAIL(); }
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
  auto result = mdp::Parser::dispatch(handler, buffer, trace_info);
  CHECK(result == true);
  CHECK(handler.counter == 2);
}

TEST_CASE("quote_request_39", "[mdp3]") {
  auto message =
      "\xb2\xd6\x24\x04\xc1\x40\xf1\x58\xd4\x58\x40\x17\x50\x00\x23\x00\x27\x00\x01\x00\x09\x00\x99\x6e\xea\x58\xd4\x58\x40\x17\x43\x4d\x45\x30\x30\x36\x39\x39\x34\x38\x36\x37\x38\x34\x33\x32\x35\x35\x38\x00\x00\x00\x00\x80\x00\x00\x00\x20\x00\x01\x5a\x54\x4d\x33\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x5e\xac\x0a\x00\xff\xff\xff\x7f\x01\x7f\x00\x00"sv;
  CHECK(std::size(message) == 92);
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
    void operator()(Trace<cme_mdp::MDIncrementalRefreshTradeSummary48> const &, mdp::Frame const &) override { FAIL(); }
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
    void operator()(Trace<cme_mdp::QuoteRequest39> const &event, mdp::Frame const &) override {
      ++counter;
      using value_type = std::remove_cvref<decltype(event)>::type::value_type;
      auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
      auto tmp = fmt::format("{}"sv, value);
      // fmt::print("{}\n", tmp);
    }
  } handler;
  std::span buffer{reinterpret_cast<std::byte const *>(std::data(message)), std::size(message)};
  TraceInfo trace_info;
  auto result = mdp::Parser::dispatch(handler, buffer, trace_info);
  CHECK(result == true);
  CHECK(handler.counter == 1);
}
