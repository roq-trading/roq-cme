/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include <fmt/format.h>

#include <cmath>
#include <span>

#include "roq/cme/sbe/parser.hpp"

using namespace std::literals;

using namespace Catch::literals;

using namespace roq;
using namespace roq::cme;

TEST_CASE("batched", "[incremental]") {
  auto message =
      "\x0e\x00\x00\x00"                  // sequence number
      "\x7e\x8d\x4e\x9b\x55\x31\x41\x17"  // sending time
      "\x08\x03"                          // message size
      "\x3b\x00"                          // block length
      "\x34\x00"                          // template id (52)
      "\x01\x00"                          // schema id (1)
      "\x09\x00"                          // version (9)
      "\xe3\xdc\x2c\x00\x64\x00\x00\x00\x9a\x2f\x00\x00\x3e\x38\x0c\x00\x6f\xa6\x7c\x96\x55\x31\x41\x17\xc0\xd1\xd4\xbc"
      "\x7d\xfc\x40\x17\xc2\x4b\x11\xff\xff\xff\xff\xff\xff\xff\x7f\xff\xff\xff\xff\xff\xff\xff\x7f\x00\x65\xcd\x1d\x00"
      "\x00\x00\x00\x16\x00\x20\xd0\x23\xf6\xf2\xff\xff\xff\xff\x02\x00\x00\x00\x01\x00\x00\x00\x01\xff\xff\xff\x80\x30"
      "\xa8\xb8\x07\xf2\xff\xff\xff\xff\x04\x00\x00\x00\x02\x00\x00\x00\x02\xff\xff\xff\x80\x30\x80\x4d\x19\xf1\xff\xff"
      "\xff\xff\x02\x00\x00\x00\x01\x00\x00\x00\x03\xff\xff\xff\x80\x30\x58\xe2\x2a\xf0\xff\xff\xff\xff\x02\x00\x00\x00"
      "\x01\x00\x00\x00\x04\xff\xff\xff\x80\x30\x30\x77\x3c\xef\xff\xff\xff\xff\x02\x00\x00\x00\x01\x00\x00\x00\x05\xff"
      "\xff\xff\x80\x30\x08\x0c\x4e\xee\xff\xff\xff\xff\x02\x00\x00\x00\x01\x00\x00\x00\x06\xff\xff\xff\x80\x30\xe0\xa0"
      "\x5f\xed\xff\xff\xff\xff\x02\x00\x00\x00\x01\x00\x00\x00\x07\xff\xff\xff\x80\x30\xb8\x35\x71\xec\xff\xff\xff\xff"
      "\x02\x00\x00\x00\x01\x00\x00\x00\x08\xff\xff\xff\x80\x30\x90\xca\x82\xeb\xff\xff\xff\xff\x02\x00\x00\x00\x01\x00"
      "\x00\x00\x09\xff\xff\xff\x80\x30\x68\x5f\x94\xea\xff\xff\xff\xff\x02\x00\x00\x00\x01\x00\x00\x00\x0a\xff\xff\xff"
      "\x80\x30\x20\xfa\xd2\xf4\xff\xff\xff\xff\x04\x00\x00\x00\x02\x00\x00\x00\x01\xff\xff\xff\x80\x31\x48\x65\xc1\xf5"
      "\xff\xff\xff\xff\x02\x00\x00\x00\x01\x00\x00\x00\x02\xff\xff\xff\x80\x31\x70\xd0\xaf\xf6\xff\xff\xff\xff\x02\x00"
      "\x00\x00\x01\x00\x00\x00\x03\xff\xff\xff\x80\x31\x98\x3b\x9e\xf7\xff\xff\xff\xff\x02\x00\x00\x00\x01\x00\x00\x00"
      "\x04\xff\xff\xff\x80\x31\xc0\xa6\x8c\xf8\xff\xff\xff\xff\x02\x00\x00\x00\x01\x00\x00\x00\x05\xff\xff\xff\x80\x31"
      "\xe8\x11\x7b\xf9\xff\xff\xff\xff\x02\x00\x00\x00\x01\x00\x00\x00\x06\xff\xff\xff\x80\x31\x10\x7d\x69\xfa\xff\xff"
      "\xff\xff\x02\x00\x00\x00\x01\x00\x00\x00\x07\xff\xff\xff\x80\x31\x38\xe8\x57\xfb\xff\xff\xff\xff\x02\x00\x00\x00"
      "\x01\x00\x00\x00\x08\xff\xff\xff\x80\x31\x60\x53\x46\xfc\xff\xff\xff\xff\x02\x00\x00\x00\x01\x00\x00\x00\x09\xff"
      "\xff\xff\x80\x31\x88\xbe\x34\xfd\xff\xff\xff\xff\x02\x00\x00\x00\x01\x00\x00\x00\x0a\xff\xff\xff\x80\x31\xa8\xb8"
      "\x07\xf2\xff\xff\xff\xff\xaf\x00\x00\x00\xff\xff\xff\x7f\x01\xff\xff\xff\x80\x45\x80\x4d\x19\xf1\xff\xff\xff\xff"
      "\x19\x01\x00\x00\xff\xff\xff\x7f\x02\xff\xff\xff\x80\x45\xf8\x8e\xe4\xf3\xff\xff\xff\xff\x18\x01\x00\x00\xff\xff"
      "\xff\x7f\x01\xff\xff\xff\x80\x46\x20\xfa\xd2\xf4\xff\xff\xff\xff\x0b\x00\x00\x00\xff\xff\xff\x7f\x02\xff\xff\xff"
      "\x80\x46\x10\x7d\x69\xfa\xff\xff\xff\xff\xff\xff\xff\x7f\xff\xff\xff\x7f\x7f\xff\xff\xff\x80\x37\xc0\xa6\x8c\xf8"
      "\xff\xff\xff\xff\xff\xff\xff\x7f\xff\xff\xff\x7f\x7f\xff\xff\xff\x80\x38\xd8\x94\x11\xff\xff\xff\xff\xff\xff\xff"
      "\xff\x7f\xff\xff\xff\x7f\x7f\xff\xff\xff\x80\x4e\x20\xfa\xd2\xf4\xff\xff\xff\xff\xff\xff\xff\x7f\xff\xff\xff\x7f"
      "\x7f\xff\xff\xff\x80\x4f\xc0\xa6\x8c\xf8\xff\xff\xff\xff\x02\x00\x00\x00\xff\xff\xff\x7f\x7f\xff\xff\xff\x80\x32"
      "\xff\xff\xff\xff\xff\xff\xff\x7f\x06\x00\x00\x00\xff\xff\xff\x7f\x7f\xff\xff\xff\x80\x65\x10\x7d\x69\xfa\xff\xff"
      "\xff\xff\xff\xff\xff\x7f\xff\xff\xff\x7f\x7f\xff\xff\x00\x80\x34\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\x7f"
      "\xff\xff\xff\x7f\x7f\xbf\x4b\xff\x03\x36"
      "\xb6\x00"  // message size
      "\x3b\x00"  // block length
      "\x34\x00"  // template id (52)
      "\x01\x00"  // schema id (1)
      "\x09\x00"  // version (9)
      "\x02\xdd\x2c\x00\x64\x00\x00\x00"
      "\x62\x30\x00\x00\x57\x62\x01\x00\x5f\x8a\xfb\x95\x55\x31\x41\x17\xc0\xd1\xd4\xbc\x7d\xfc\x40\x17\xc2\x4b\x11\xff"
      "\xff\xff\xff\xff\xff\xff\x7f\xff\xff\xff\xff\xff\xff\xff\x7f\x00\x65\xcd\x1d\x00\x00\x00\x00\x16\x00\x05\x98\x3b"
      "\x9e\xf7\xff\xff\xff\xff\x02\x00\x00\x00\xff\xff\xff\x7f\x01\xff\xff\xff\x80\x45\xce\xa0\x62\xf7\xff\xff\xff\xff"
      "\x02\x00\x00\x00\xff\xff\xff\x7f\x02\xff\xff\xff\x80\x45\xc0\xa6\x8c\xf8\xff\xff\xff\xff\x01\x00\x00\x00\xff\xff"
      "\xff\x7f\x01\xff\xff\xff\x80\x46\x8a\x41\xc8\xf8\xff\xff\xff\xff\x03\x00\x00\x00\xff\xff\xff\x7f\x02\xff\xff\xff"
      "\x80\x46\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\x7f\xff\xff\xff\x7f\x7f\xbf\x4b\xff\x03\x36"sv;
  REQUIRE(std::size(message) == 970);
  struct MyHandler final : public sbe::Parser::Handler {
    int counter = 0;
    void operator()(sbe::Frame const &) override {}
    void operator()(Trace<cme_mdp::ChannelReset4> const &, sbe::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::AdminHeartbeat12> const &, sbe::Frame const &) override { FAIL(); }
    // - security status
    void operator()(Trace<cme_mdp::SecurityStatus30> const &, sbe::Frame const &) override { FAIL(); }
    // - instrument definition
    void operator()(Trace<cme_mdp::MDInstrumentDefinitionFuture54> const &, sbe::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDInstrumentDefinitionOption55> const &, sbe::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDInstrumentDefinitionSpread56> const &, sbe::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDInstrumentDefinitionFixedIncome57> const &, sbe::Frame const &) override {
      FAIL();
    }
    void operator()(Trace<cme_mdp::MDInstrumentDefinitionRepo58> const &, sbe::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDInstrumentDefinitionFX63> const &, sbe::Frame const &) override { FAIL(); }
    // - SnapshotFullRefresh
    void operator()(Trace<cme_mdp::SnapshotFullRefresh52> const &event, sbe::Frame const &) override {
      using value_type = std::remove_cvref<decltype(event)>::type::value_type;
      auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
      switch (++counter) {
        case 1: {
          CHECK(value.lastMsgSeqNumProcessed() == 2940131);
          CHECK(value.totNumReports() == 100);
          CHECK(value.securityID() == 12186);
          CHECK(value.rptSeq() == 800830);
          CHECK(value.transactTime() == 1675674780025267823);
          CHECK(value.lastUpdateTime() == 1675616678351000000);
          CHECK(value.tradeDate() == 19394);
          CHECK(value.mDSecurityTradingStatus() == cme_mdp::SecurityTradingStatus::ReadyToTrade);
          CHECK(std::isnan(sbe::get_double(value.highLimitPrice())));
          CHECK(std::isnan(sbe::get_double(value.lowLimitPrice())));
          CHECK(sbe::get_double(value.maxPriceVariation()) == 0.5_a);
          value.sbeRewind();  // wtf!
          auto no_md_entries_rows = 0;
          value.noMDEntries().forEach([&no_md_entries_rows](auto &item) {
            switch (++no_md_entries_rows) {
              case 1:
                CHECK(sbe::get_double(item.mDEntryPx()) == -0.21875_a);
                CHECK(sbe::get_int<int32_t>(item.mDEntrySize(), item.mDEntrySizeNullValue()) == 2);
                CHECK(sbe::get_int<int32_t>(item.numberOfOrders(), item.numberOfOrdersNullValue()) == 1);
                CHECK(sbe::get_int<int8_t>(item.mDPriceLevel(), item.mDPriceLevelNullValue()) == 1);
                CHECK(sbe::get_int<uint16_t>(item.tradingReferenceDate(), item.tradingReferenceDateNullValue()) == 0);
                CHECK(item.openCloseSettlFlag() == cme_mdp::OpenCloseSettlFlag::NULL_VALUE);
                CHECK(item.settlPriceType().nullValue());
                CHECK(item.mDEntryType() == cme_mdp::MDEntryType::Value::Bid);
                break;
              case 2:
                CHECK(sbe::get_double(item.mDEntryPx()) == -0.234375_a);
                CHECK(sbe::get_int<int32_t>(item.mDEntrySize(), item.mDEntrySizeNullValue()) == 4);
                CHECK(sbe::get_int<int32_t>(item.numberOfOrders(), item.numberOfOrdersNullValue()) == 2);
                CHECK(sbe::get_int<int8_t>(item.mDPriceLevel(), item.mDPriceLevelNullValue()) == 2);
                CHECK(sbe::get_int<uint16_t>(item.tradingReferenceDate(), item.tradingReferenceDateNullValue()) == 0);
                CHECK(item.openCloseSettlFlag() == cme_mdp::OpenCloseSettlFlag::NULL_VALUE);
                CHECK(item.settlPriceType().nullValue());
                CHECK(item.mDEntryType() == cme_mdp::MDEntryType::Value::Bid);
                break;
              // ...
              case 31:
                CHECK(sbe::get_double(item.mDEntryPx()) == -0.09375_a);
                CHECK(sbe::get_int<int32_t>(item.mDEntrySize(), item.mDEntrySizeNullValue()) == 0);
                CHECK(sbe::get_int<int32_t>(item.numberOfOrders(), item.numberOfOrdersNullValue()) == 0);
                CHECK(sbe::get_int<int8_t>(item.mDPriceLevel(), item.mDPriceLevelNullValue()) == 0);
                CHECK(sbe::get_int<uint16_t>(item.tradingReferenceDate(), item.tradingReferenceDateNullValue()) == 0);
                CHECK(item.openCloseSettlFlag() == cme_mdp::OpenCloseSettlFlag::DailyOpenPrice);
                CHECK(item.settlPriceType().nullValue());
                CHECK(item.mDEntryType() == cme_mdp::MDEntryType::Value::OpenPrice);
                break;
              case 32:
                CHECK(sbe::get_double(item.mDEntryPx()) == 0.0_a);
                CHECK(sbe::get_int<int32_t>(item.mDEntrySize(), item.mDEntrySizeNullValue()) == 0);
                CHECK(sbe::get_int<int32_t>(item.numberOfOrders(), item.numberOfOrdersNullValue()) == 0);
                CHECK(sbe::get_int<int8_t>(item.mDPriceLevel(), item.mDPriceLevelNullValue()) == 0);
                CHECK(
                    sbe::get_int<uint16_t>(item.tradingReferenceDate(), item.tradingReferenceDateNullValue()) == 19391);
                CHECK(item.openCloseSettlFlag() == cme_mdp::OpenCloseSettlFlag::NULL_VALUE);
                CHECK(item.settlPriceType().rawValue() == 3);  // note! 3 == intraday
                CHECK(item.mDEntryType() == cme_mdp::MDEntryType::Value::SettlementPrice);
                break;
            }
          });
          CHECK(no_md_entries_rows == 32);
          break;
        }
        case 2: {
          CHECK(value.lastMsgSeqNumProcessed() == 2940162);
          CHECK(value.totNumReports() == 100);
          CHECK(value.securityID() == 12386);
          CHECK(value.rptSeq() == 90711);
          CHECK(value.transactTime() == 1675674780016806495);
          CHECK(value.lastUpdateTime() == 1675616678351000000);
          CHECK(value.tradeDate() == 19394);
          CHECK(value.mDSecurityTradingStatus() == cme_mdp::SecurityTradingStatus::ReadyToTrade);
          CHECK(std::isnan(sbe::get_double(value.highLimitPrice())));
          CHECK(std::isnan(sbe::get_double(value.lowLimitPrice())));
          CHECK(sbe::get_double(value.maxPriceVariation()) == 0.5_a);
          value.sbeRewind();  // wtf!
          auto no_md_entries_rows = 0;
          value.noMDEntries().forEach([&no_md_entries_rows](auto &item) {
            switch (++no_md_entries_rows) {
              case 1:
                CHECK(sbe::get_double(item.mDEntryPx()) == -0.140625_a);
                CHECK(sbe::get_int<int32_t>(item.mDEntrySize(), item.mDEntrySizeNullValue()) == 2);
                CHECK(sbe::get_int<int32_t>(item.numberOfOrders(), item.numberOfOrdersNullValue()) == 0);
                CHECK(sbe::get_int<int8_t>(item.mDPriceLevel(), item.mDPriceLevelNullValue()) == 1);
                CHECK(sbe::get_int<uint16_t>(item.tradingReferenceDate(), item.tradingReferenceDateNullValue()) == 0);
                CHECK(item.openCloseSettlFlag() == cme_mdp::OpenCloseSettlFlag::NULL_VALUE);
                CHECK(item.settlPriceType().nullValue());
                CHECK(item.mDEntryType() == cme_mdp::MDEntryType::Value::ImpliedBid);
                break;
              case 2:
                CHECK(sbe::get_double(item.mDEntryPx()) == -0.14453125_a);
                CHECK(sbe::get_int<int32_t>(item.mDEntrySize(), item.mDEntrySizeNullValue()) == 2);
                CHECK(sbe::get_int<int32_t>(item.numberOfOrders(), item.numberOfOrdersNullValue()) == 0);
                CHECK(sbe::get_int<int8_t>(item.mDPriceLevel(), item.mDPriceLevelNullValue()) == 2);
                CHECK(sbe::get_int<uint16_t>(item.tradingReferenceDate(), item.tradingReferenceDateNullValue()) == 0);
                CHECK(item.openCloseSettlFlag() == cme_mdp::OpenCloseSettlFlag::NULL_VALUE);
                CHECK(item.settlPriceType().nullValue());
                CHECK(item.mDEntryType() == cme_mdp::MDEntryType::Value::ImpliedBid);
                break;
              // ...
              case 4:
                CHECK(sbe::get_double(item.mDEntryPx()) == -0.12109375_a);
                CHECK(sbe::get_int<int32_t>(item.mDEntrySize(), item.mDEntrySizeNullValue()) == 3);
                CHECK(sbe::get_int<int32_t>(item.numberOfOrders(), item.numberOfOrdersNullValue()) == 0);
                CHECK(sbe::get_int<int8_t>(item.mDPriceLevel(), item.mDPriceLevelNullValue()) == 2);
                CHECK(sbe::get_int<uint16_t>(item.tradingReferenceDate(), item.tradingReferenceDateNullValue()) == 0);
                CHECK(item.openCloseSettlFlag() == cme_mdp::OpenCloseSettlFlag::NULL_VALUE);
                CHECK(item.settlPriceType().nullValue());
                CHECK(item.mDEntryType() == cme_mdp::MDEntryType::Value::ImpliedOffer);
                break;
              case 5:
                CHECK(sbe::get_double(item.mDEntryPx()) == 0.0_a);
                CHECK(sbe::get_int<int32_t>(item.mDEntrySize(), item.mDEntrySizeNullValue()) == 0);
                CHECK(sbe::get_int<int32_t>(item.numberOfOrders(), item.numberOfOrdersNullValue()) == 0);
                CHECK(sbe::get_int<int8_t>(item.mDPriceLevel(), item.mDPriceLevelNullValue()) == 0);
                CHECK(
                    sbe::get_int<uint16_t>(item.tradingReferenceDate(), item.tradingReferenceDateNullValue()) == 19391);
                CHECK(item.openCloseSettlFlag() == cme_mdp::OpenCloseSettlFlag::NULL_VALUE);
                CHECK(item.settlPriceType().rawValue() == 3);  // note! 3 == intraday
                CHECK(item.mDEntryType() == cme_mdp::MDEntryType::Value::SettlementPrice);
                break;
            }
          });
          CHECK(no_md_entries_rows == 5);
          break;
        }
      }
    }
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
    void operator()(Trace<cme_mdp::QuoteRequest39> const &, sbe::Frame const &) override { FAIL(); }
  } handler;
  std::span buffer{reinterpret_cast<std::byte const *>(std::data(message)), std::size(message)};
  TraceInfo trace_info;
  auto res = sbe::Parser::dispatch(handler, buffer, trace_info);
  CHECK(res);
  CHECK(handler.counter == 2);
}

TEST_CASE("simple", "[incremental]") {
  auto message =
      "\x03\xdd\x2c\x00"                  // sequence number
      "\xe8\xf2\x7e\x9b\x55\x31\x41\x17"  // sending time
      "\x60\x00"                          // message size
      "\x0b\x00"                          // block length
      "\x30\x00"                          // template id (48)
      "\x01\x00"                          // schema id (1)
      "\x09\x00"                          // version (9)
      "\xcd\xd3\x7b\x9b\x55\x31\x41\x17\x01\x00\x00\x20\x00\x01\x14\x85\x78\x52\x19\x00\x00\x00\x25\x00\x00\x00\x6e\x84"
      "\x07\x00\x20\x1f\x09\x00\x02\x00\x00\x00\x02\x00\xe2\xf0\x0b\x00\x00\x00\x10\x00\x00\x00\x00\x00\x00\x02\x20\x29"
      "\x4e\x80\xa6\x07\x00\x00\x25\x00\x00\x00\x00\x00\x00\x00\xe9\x1f\x9d\x7f\xa6\x07\x00\x00\x25\x00\x00\x00\x00\x00"
      "\x00\x00"sv;
  REQUIRE(std::size(message) == 108);
  struct MyHandler final : public sbe::Parser::Handler {
    int counter = 0;
    void operator()(sbe::Frame const &) override {}
    void operator()(Trace<cme_mdp::ChannelReset4> const &, sbe::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::AdminHeartbeat12> const &, sbe::Frame const &) override { FAIL(); }
    // - security status
    void operator()(Trace<cme_mdp::SecurityStatus30> const &, sbe::Frame const &) override { FAIL(); }
    // - instrument definition
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
    void operator()(Trace<cme_mdp::MDIncrementalRefreshBook46> const &, sbe::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshOrderBook47> const &, sbe::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshTradeSummary48> const &event, sbe::Frame const &) override {
      using value_type = std::remove_cvref<decltype(event)>::type::value_type;
      auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
      switch (++counter) {
        case 1: {
          CHECK(value.transactTime() == 1675674780109099981);
          // CHECK(value.matchEventIndicator() ==
          value.sbeRewind();  // wtf!
          auto no_md_entries_rows = 0;
          value.noMDEntries().forEach([&no_md_entries_rows](auto &item) {
            switch (++no_md_entries_rows) {
              case 1:
                CHECK(sbe::get_double(item.mDEntryPx()) == 108.7578125_a);
                CHECK(item.mDEntrySize() == 37);
                CHECK(item.securityID() == 492654);
                CHECK(item.rptSeq() == 597792);
                CHECK(item.numberOfOrders() == 2);
                CHECK(item.aggressorSide() == cme_mdp::AggressorSide::Value::Sell);
                CHECK(item.mDUpdateAction() == cme_mdp::MDUpdateAction::Value::New);
                // CHECK(item.mDEntryType().rawValue() == 2); // note! constant
                CHECK(sbe::get_int<uint32_t>(item.mDTradeEntryID(), item.mDTradeEntryIDNullValue()) == 782562);
                break;
            }
          });
          CHECK(no_md_entries_rows == 1);
          break;
        }
      }
    }
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
    void operator()(Trace<cme_mdp::QuoteRequest39> const &, sbe::Frame const &) override { FAIL(); }
  } handler;
  std::span buffer{reinterpret_cast<std::byte const *>(std::data(message)), std::size(message)};
  TraceInfo trace_info;
  auto res = sbe::Parser::dispatch(handler, buffer, trace_info);
  CHECK(res);
  CHECK(handler.counter == 1);
}

TEST_CASE("complex", "[incremental]") {
  auto message =
      "\x04\xdd\x2c\x00"                  // sequence number
      "\x4a\xed\x7f\x9b\x55\x31\x41\x17"  // sending time
      "\x28\x00"                          // message size
      "\x0b\x00"                          // block length
      "\x25\x00"                          // template id (37)
      "\x01\x00"                          // schema id (1)
      "\x09\x00"                          // version (9)
      "\xcd\xd3\x7b\x9b\x55\x31\x41\x17\x02\x00\x00\x10\x00\x01\xbc\xbc\x03\x00\x6e\x84\x07\x00\x21\x1f\x09\x00\x00\x00"
      "\x00\x00"
      "\x58\x00"  // message size
      "\x0b\x00"  // block length
      "\x2e\x00"  // template id (46)
      "\x01\x00"  // schema id (1)
      "\x09\x00"  // version (9)
      "\xcd\xd3\x7b\x9b\x55\x31\x41\x17\x04\x00\x00\x20\x00\x01\x14\x85\x78\x52\x19\x00\x00\x00\x2f\x01\x00\x00\x6e\x84"
      "\x07\x00\x22\x1f\x09\x00\x27\x00\x00\x00\x01\x01\x30\x00\x00\x00\x00\x00\x18\x00\x00\x00\x00\x00\x00\x01\xe9\x1f"
      "\x9d\x7f\xa6\x07\x00\x00\x88\xfc\x76\xc5\x04\x00\x00\x00\x2e\x00\x00\x00\x01\x01\x00\x00"
      "\xa0\x01"  // mssage size
      "\x0b\x00"  // block length
      "\x2e\x00"  // template id (46)
      "\x01\x00"  // schema id (1)
      "\x09\x00"  // version (9)
      "\xcd\xd3\x7b\x9b\x55\x31\x41\x17\x90\x00\x00\x20\x00\x0c\xc8\x17\xa8\x04\x00\x00\x00\x00\x2f\x01\x00\x00\x7b\x22"
      "\x00\x00\x3d\xa4\x0d\x00\xff\xff\xff\x7f\x01\x01\x45\x00\x00\x00\x00\x00\x34\xe2\x30\x04\x00\x00\x00\x00\x42\x02"
      "\x00\x00\x7b\x22\x00\x00\x3e\xa4\x0d\x00\xff\xff\xff\x7f\x02\x01\x45\x00\x00\x00\x00\x00\xca\x9a\x3b\x00\x00\x00"
      "\x00\x00\x0d\x00\x00\x00\xb2\x23\x00\x00\x24\xee\x07\x00\xff\xff\xff\x7f\x02\x01\x46\x00\x00\x00\x00\x00\x60\x53"
      "\x46\xfc\xff\xff\xff\xff\x65\x00\x00\x00\xb8\x25\x00\x00\xf1\x49\x0b\x00\xff\xff\xff\x7f\x01\x01\x45\x00\x00\x00"
      "\x00\x00\xc0\xa6\x8c\xf8\xff\xff\xff\xff\x97\x00\x00\x00\x55\x29\x00\x00\xee\x4a\x0b\x00\xff\xff\xff\x7f\x01\x01"
      "\x45\x00\x00\x00\x00\x00\x44\x5f\x9a\xfe\xff\xff\xff\xff\x39\x00\x00\x00\xa0\x29\x00\x00\x0e\xca\x08\x00\xff\xff"
      "\xff\x7f\x02\x01\x46\x00\x00\x00\x00\x00\x98\x3b\x9e\xf7\xff\xff\xff\xff\x65\x00\x00\x00\xe4\x30\x00\x00\xa2\xed"
      "\x0b\x00\xff\xff\xff\x7f\x01\x01\x45\x00\x00\x00\x00\x00\x04\x06\x27\xf7\xff\xff\xff\xff\x4a\x00\x00\x00\xe4\x30"
      "\x00\x00\xa3\xed\x0b\x00\xff\xff\xff\x7f\x02\x01\x45\x00\x00\x00\x00\x00\x3c\xee\x7e\xf2\xff\xff\xff\xff\x15\x00"
      "\x00\x00\xf9\x34\x00\x00\x9c\x02\x08\x00\xff\xff\xff\x7f\x02\x01\x45\x00\x00\x00\x00\x00\xe8\x11\x7b\xf9\xff\xff"
      "\xff\xff\x11\x00\x00\x00\xdc\x37\x00\x00\x5d\x1c\x0e\x00\xff\xff\xff\x7f\x02\x01\x45\x00\x00\x00\x00\x00\x50\xd6"
      "\xdc\x01\x00\x00\x00\x00\x14\x00\x00\x00\xa8\x3a\x00\x00\x9f\xe5\x0a\x00\xff\xff\xff\x7f\x02\x01\x46\x00\x00\x00"
      "\x00\x00\xb0\x1d\x11\x89\x1a\x00\x00\x00\x35\x00\x00\x00\xa2\x11\x06\x00\xb9\x3b\x13\x00\xff\xff\xff\x7f\x02\x01"
      "\x45\x00\x00\x00\x00\x00\x18\x00\x00\x00\x00\x00\x00\x00"
      "\x70\x00"  // message size
      "\x0b\x00"  // block length
      "\x30\x00"  // template id (48)
      "\x01\x00"  // schema id (1)
      "\x09\x00"  // version (9)
      "\x3b\x08\x7c\x9b\x55\x31\x41\x17\x01\x00\x00\x20\x00\x01\x00\xf4\xed\x8a\x1a\x00\x00\x00\x08\x00\x00\x00\xa2\x11"
      "\x06\x00\xba\x3b\x13\x00\x03\x00\x00\x00\x02\x00\xe3\xf0\x0b\x00\x00\x00\x10\x00\x00\x00\x00\x00\x00\x03\x21\x29"
      "\x4e\x80\xa6\x07\x00\x00\x08\x00\x00\x00\x00\x00\x00\x00\x61\x3e\x08\x7f\xa6\x07\x00\x00\x03\x00\x00\x00\x00\x00"
      "\x00\x00\xe6\x2d\x09\x7f\xa6\x07\x00\x00\x05\x00\x00\x00\x00\x00\x00\x00"
      "\x28\x00"  // msesage size
      "\x0b\x00"  // block length
      "\x25\x00"  // template id (37)
      "\x01\x00"  // schema id (1)
      "\x09\x00"  // version (9)
      "\x3b\x08\x7c\x9b\x55\x31\x41\x17\x02\x00\x00\x10\x00\x01\x06\x0e\x05\x00\xa2\x11\x06\x00\xbb\x3b\x13\x00\x00\x00"
      "\x00\x00"
      "\x68\x00"  // message size
      "\x0b\x00"  // block length
      "\x2f\x00"  // template id (47)
      "\x01\x00"  // schema id (1)
      "\x09\x00"  // version (9)
      "\x3b\x08\x7c\x9b\x55\x31\x41\x17\x04\x00\x00\x28\x00\x02\x61\x3e\x08\x7f\xa6\x07\x00\x00\x7d\x09\x8d\xc4\x04\x00"
      "\x00\x00\x00\xf4\xed\x8a\x1a\x00\x00\x00\x03\x00\x00\x00\xa2\x11\x06\x00\x02\x30\x00\x00\x00\x00\x00\x00\xe6\x2d"
      "\x09\x7f\xa6\x07\x00\x00\x9d\x58\x8e\xc4\x04\x00\x00\x00\x00\xf4\xed\x8a\x1a\x00\x00\x00\x1b\x01\x00\x00\xa2\x11"
      "\x06\x00\x01\x30\x00\x00\x00\x00\x00\x00"
      "\x40\x00"  // message size
      "\x0b\x00"  // block length
      "\x2e\x00"  // template id (46)
      "\x01\x00"  // schema id (1)
      "\x09\x00"  // version (9)
      "\x3b\x08\x7c\x9b\x55\x31\x41\x17\x04\x00\x00\x20\x00\x01\x00\xf4\xed\x8a\x1a\x00\x00\x00\x5b\x09\x00\x00\xa2\x11"
      "\x06\x00\xbc\x3b\x13\x00\x8a\x00\x00\x00\x01\x01\x30\x00\x00\x00\x00\x00\x18\x00\x00\x00\x00\x00\x00\x00"sv;
  REQUIRE(std::size(message) == 876);
  struct MyHandler final : public sbe::Parser::Handler {
    int count_37 = 0;
    int count_46 = 0;
    int count_47 = 0;
    int count_48 = 0;
    void operator()(sbe::Frame const &) override {}
    void operator()(Trace<cme_mdp::ChannelReset4> const &, sbe::Frame const &) override { FAIL(); }
    void operator()(Trace<cme_mdp::AdminHeartbeat12> const &, sbe::Frame const &) override { FAIL(); }
    // - security status
    void operator()(Trace<cme_mdp::SecurityStatus30> const &, sbe::Frame const &) override { FAIL(); }
    // - instrument definition
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
    void operator()(Trace<cme_mdp::MDIncrementalRefreshVolume37> const &event, sbe::Frame const &) override {
      using value_type = std::remove_cvref<decltype(event)>::type::value_type;
      auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
      switch (++count_37) {
        case 1: {
          CHECK(value.transactTime() == 1675674780109099981);
          // CHECK(value.matchEventIndicator() ==
          value.sbeRewind();  // wtf!
          auto no_md_entries_rows = 0;
          value.noMDEntries().forEach([&no_md_entries_rows](auto &item) {
            switch (++no_md_entries_rows) {
              case 1:
                CHECK(item.mDEntrySize() == 244924);
                CHECK(item.securityID() == 492654);
                CHECK(item.rptSeq() == 597793);
                CHECK(item.mDUpdateAction() == cme_mdp::MDUpdateAction::Value::New);
                // CHECK(item.mDEntryType().rawValue() == 'e'); // note! constant MDEntryTypeVol
                break;
            }
          });
          CHECK(no_md_entries_rows == 1);
          break;
        }
      }
    }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshBook46> const &event, sbe::Frame const &) override {
      using value_type = std::remove_cvref<decltype(event)>::type::value_type;
      auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
      switch (++count_46) {
        case 1: {
          CHECK(value.transactTime() == 1675674780109099981);
          value.sbeRewind();  // wtf!
          auto no_md_entries_rows = 0;
          value.noMDEntries().forEach([&no_md_entries_rows](auto &item) {
            switch (++no_md_entries_rows) {
              case 1:
                CHECK(sbe::get_double(item.mDEntryPx()) == 108.7578125_a);
                CHECK(sbe::get_int<int32_t>(item.mDEntrySize(), item.mDEntrySizeNullValue()) == 303);
                CHECK(item.securityID() == 492654);
                CHECK(item.rptSeq() == 597794);
                CHECK(sbe::get_int<int32_t>(item.numberOfOrders(), item.numberOfOrdersNullValue()) == 39);
                CHECK(item.mDPriceLevel() == 1);
                CHECK(item.mDUpdateAction() == cme_mdp::MDUpdateAction::Value::Change);
                CHECK(item.mDEntryType() == cme_mdp::MDEntryTypeBook::Value::Bid);
                CHECK(sbe::get_int<int32_t>(item.tradeableSize(), item.tradeableSizeNullValue()) == 0);
                break;
            }
          });
          CHECK(no_md_entries_rows == 1);
          auto no_order_id_entries = 0;
          value.noOrderIDEntries().forEach([&no_order_id_entries](auto &item) {
            switch (++no_order_id_entries) {
              case 1:
                CHECK(item.orderID() == 8411686969321);
                CHECK(sbe::get_int<uint64_t>(item.mDOrderPriority(), item.mDOrderPriorityNullValue()) == 20492778632);
                CHECK(sbe::get_int<int32_t>(item.mDDisplayQty(), item.mDDisplayQtyNullValue()) == 46);
                CHECK(sbe::get_int<uint8_t>(item.referenceID(), item.referenceIDNullValue()) == 1);
                CHECK(item.orderUpdateAction() == cme_mdp::OrderUpdateAction::Value::Update);
                break;
            }
          });
          CHECK(no_order_id_entries == 1);
          break;
        }
        case 2: {
          CHECK(value.transactTime() == 1675674780109099981);
          value.sbeRewind();  // wtf!
          auto no_md_entries_rows = 0;
          value.noMDEntries().forEach([&no_md_entries_rows](auto &item) {
            switch (++no_md_entries_rows) {
              case 1:
                CHECK(sbe::get_double(item.mDEntryPx()) == 0.078125_a);
                CHECK(sbe::get_int<int32_t>(item.mDEntrySize(), item.mDEntrySizeNullValue()) == 303);
                CHECK(item.securityID() == 8827);
                CHECK(item.rptSeq() == 894013);
                CHECK(sbe::get_int<int32_t>(item.numberOfOrders(), item.numberOfOrdersNullValue()) == 0);
                CHECK(item.mDPriceLevel() == 1);
                CHECK(item.mDUpdateAction() == cme_mdp::MDUpdateAction::Value::Change);
                CHECK(item.mDEntryType() == cme_mdp::MDEntryTypeBook::Value::ImpliedBid);
                CHECK(sbe::get_int<int32_t>(item.tradeableSize(), item.tradeableSizeNullValue()) == 0);
                break;
              // ...
              case 12:
                CHECK(sbe::get_double(item.mDEntryPx()) == 113.96875_a);
                CHECK(sbe::get_int<int32_t>(item.mDEntrySize(), item.mDEntrySizeNullValue()) == 53);
                CHECK(item.securityID() == 397730);
                CHECK(item.rptSeq() == 1260473);
                CHECK(sbe::get_int<int32_t>(item.numberOfOrders(), item.numberOfOrdersNullValue()) == 0);
                CHECK(item.mDPriceLevel() == 2);
                CHECK(item.mDUpdateAction() == cme_mdp::MDUpdateAction::Value::Change);
                CHECK(item.mDEntryType() == cme_mdp::MDEntryTypeBook::Value::ImpliedBid);
                CHECK(sbe::get_int<int32_t>(item.tradeableSize(), item.tradeableSizeNullValue()) == 0);
                break;
            }
          });
          CHECK(no_md_entries_rows == 12);
          auto no_order_id_entries = 0;
          value.noOrderIDEntries().forEach(
              [&no_order_id_entries]([[maybe_unused]] auto &item) { ++no_order_id_entries; });
          CHECK(no_order_id_entries == 0);
          break;
        }
        case 3: {
          CHECK(value.transactTime() == 1675674780109113403);
          value.sbeRewind();  // wtf!
          auto no_md_entries_rows = 0;
          value.noMDEntries().forEach([&no_md_entries_rows](auto &item) {
            switch (++no_md_entries_rows) {
              case 1:
                CHECK(sbe::get_double(item.mDEntryPx()) == 114.0_a);
                CHECK(sbe::get_int<int32_t>(item.mDEntrySize(), item.mDEntrySizeNullValue()) == 2395);
                CHECK(item.securityID() == 397730);
                CHECK(item.rptSeq() == 1260476);
                CHECK(sbe::get_int<int32_t>(item.numberOfOrders(), item.numberOfOrdersNullValue()) == 138);
                CHECK(item.mDPriceLevel() == 1);
                CHECK(item.mDUpdateAction() == cme_mdp::MDUpdateAction::Value::Change);
                CHECK(item.mDEntryType() == cme_mdp::MDEntryTypeBook::Value::Bid);
                CHECK(sbe::get_int<int32_t>(item.tradeableSize(), item.tradeableSizeNullValue()) == 0);
                break;
            }
          });
          CHECK(no_md_entries_rows == 1);
          auto no_order_id_entries = 0;
          value.noOrderIDEntries().forEach(
              [&no_order_id_entries]([[maybe_unused]] auto &item) { ++no_order_id_entries; });
          CHECK(no_order_id_entries == 0);
          break;
        }
      }
    }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshOrderBook47> const &event, sbe::Frame const &) override {
      using value_type = std::remove_cvref<decltype(event)>::type::value_type;
      auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
      switch (++count_47) {
        case 1: {
          CHECK(value.transactTime() == 1675674780109113403);
          value.sbeRewind();  // wtf!
          auto no_md_entries_rows = 0;
          value.noMDEntries().forEach([&no_md_entries_rows](auto &item) {
            switch (++no_md_entries_rows) {
              case 1:
                CHECK(sbe::get_int<uint64_t>(item.orderID(), item.orderIDNullValue()) == 8411677212257);
                CHECK(sbe::get_int<uint64_t>(item.mDOrderPriority(), item.mDOrderPriorityNullValue()) == 20477446525);
                CHECK(sbe::get_double(item.mDEntryPx()) == 114.0_a);
                CHECK(sbe::get_int<int32_t>(item.mDDisplayQty(), item.mDDisplayQtyNullValue()) == 3);
                CHECK(item.securityID() == 397730);
                CHECK(item.mDUpdateAction() == cme_mdp::MDUpdateAction::Value::Delete);
                CHECK(item.mDEntryType() == cme_mdp::MDEntryTypeBook::Value::Bid);
                break;
              case 2:
                CHECK(sbe::get_int<uint64_t>(item.orderID(), item.orderIDNullValue()) == 8411677273574);
                CHECK(sbe::get_int<uint64_t>(item.mDOrderPriority(), item.mDOrderPriorityNullValue()) == 20477532317);
                CHECK(sbe::get_double(item.mDEntryPx()) == 114.0_a);
                CHECK(sbe::get_int<int32_t>(item.mDDisplayQty(), item.mDDisplayQtyNullValue()) == 283);
                CHECK(item.securityID() == 397730);
                CHECK(item.mDUpdateAction() == cme_mdp::MDUpdateAction::Value::Change);
                CHECK(item.mDEntryType() == cme_mdp::MDEntryTypeBook::Value::Bid);
                break;
            }
          });
          CHECK(no_md_entries_rows == 2);
          break;
        }
      }
    }
    void operator()(Trace<cme_mdp::MDIncrementalRefreshTradeSummary48> const &event, sbe::Frame const &) override {
      using value_type = std::remove_cvref<decltype(event)>::type::value_type;
      auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
      switch (++count_48) {
        case 1: {
          CHECK(value.transactTime() == 1675674780109113403);
          value.sbeRewind();  // wtf!
          auto no_md_entries_rows = 0;
          value.noMDEntries().forEach([&no_md_entries_rows](auto &item) {
            switch (++no_md_entries_rows) {
              case 1:
                CHECK(sbe::get_double(item.mDEntryPx()) == 114.0_a);
                CHECK(item.mDEntrySize() == 8);
                CHECK(item.securityID() == 397730);
                CHECK(item.rptSeq() == 1260474);
                CHECK(item.numberOfOrders() == 3);
                CHECK(item.aggressorSide() == cme_mdp::AggressorSide::Value::Sell);
                CHECK(item.mDUpdateAction() == cme_mdp::MDUpdateAction::Value::New);
                // CHECK(item.mDEntryType() == cme_mdp::MDEntryTypeBook::Value::Trade); // constant '2'
                CHECK(sbe::get_int<uint32_t>(item.mDTradeEntryID(), item.mDTradeEntryIDNullValue()) == 782563);
                break;
            }
          });
          CHECK(no_md_entries_rows == 1);
          auto no_order_id_entries = 0;
          value.noOrderIDEntries().forEach([&no_order_id_entries](auto &item) {
            switch (++no_order_id_entries) {
              case 1:
                CHECK(item.orderID() == 8411698571553);
                CHECK(item.lastQty() == 8);
                break;
              case 2:
                CHECK(item.orderID() == 8411677212257);
                CHECK(item.lastQty() == 3);
                break;
              case 3:
                CHECK(item.orderID() == 8411677273574);
                CHECK(item.lastQty() == 5);
                break;
            }
          });
          CHECK(no_order_id_entries == 3);
          break;
        }
      }
    }
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
    void operator()(Trace<cme_mdp::QuoteRequest39> const &, sbe::Frame const &) override { FAIL(); }
  } handler;
  std::span buffer{reinterpret_cast<std::byte const *>(std::data(message)), std::size(message)};
  TraceInfo trace_info;
  auto res = sbe::Parser::dispatch(handler, buffer, trace_info);
  CHECK(res);
  CHECK(handler.count_37 == 2);
  CHECK(handler.count_46 == 3);
  CHECK(handler.count_47 == 1);
  CHECK(handler.count_48 == 1);
}
