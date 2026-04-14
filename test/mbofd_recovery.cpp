/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include <fmt/format.h>

#include <cmath>
#include <span>

#include "roq/cme/mdp/parser.hpp"

using namespace std::literals;

using namespace Catch::literals;

using namespace roq;
using namespace roq::cme;

TEST_CASE("simple", "[mbofd_recovery]") {
  auto message =
      "\xf2\x01\x00\x00"                  // sequence number
      "\x6e\xfc\x55\x7a\xb9\x29\x41\x17"  // sending time
      "\xe1\x02"                          // message size (737)
      "\x1c\x00"                          // block length (28)
      "\x35\x00"                          // template id (53)
      "\x01\x00"                          // schema id (1)
      "\x09\x00"                          // version (9)
      "\x62\x37\x18\x00\x64\x00\x00\x00\xd6\x0d\x01\x00\xf2\x00\x00\x00\xf2\x00\x00\x00\x57\xcc\x16\xd3\xb6\x29\x41"
      "\x17\x1d\x00\x18\xf7\x3c\x5c\x7f\xa6\x07\x00\x00\x81\x5d\x0e\xc5\x04\x00\x00\x00\x70\xa8\xa9\xd5\x1c\x00\x00"
      "\x00\x14\x00\x00\x00\x31\x35\xd7\xe4\x7f\xa6\x07\x00\x00\xc7\x2c\xe9\xc5\x04\x00\x00\x00\x20\x32\x20\x1a\x19"
      "\x00\x00\x00\x44\x00\x00\x00\x30\x47\x3d\x5c\x7f\xa6\x07\x00\x00\x02\x5e\x0e\xc5\x04\x00\x00\x00\x98\x13\x98"
      "\xd6\x1c\x00\x00\x00\x14\x00\x00\x00\x31\x36\xd7\xe4\x7f\xa6\x07\x00\x00\xc8\x2c\xe9\xc5\x04\x00\x00\x00\xf8"
      "\xc6\x31\x19\x19\x00\x00\x00\x42\x00\x00\x00\x30\x78\x3d\x5c\x7f\xa6\x07\x00\x00\x4b\x5e\x0e\xc5\x04\x00\x00"
      "\x00\xc0\x7e\x86\xd7\x1c\x00\x00\x00\x14\x00\x00\x00\x31\x37\xd7\xe4\x7f\xa6\x07\x00\x00\xc9\x2c\xe9\xc5\x04"
      "\x00\x00\x00\xd0\x5b\x43\x18\x19\x00\x00\x00\x42\x00\x00\x00\x30\xd7\x4e\x5c\x7f\xa6\x07\x00\x00\x07\x7a\x0e"
      "\xc5\x04\x00\x00\x00\xe8\xe9\x74\xd8\x1c\x00\x00\x00\x14\x00\x00\x00\x31\x5c\x76\x43\x80\xa6\x07\x00\x00\xc9"
      "\x4a\x7a\xc6\x04\x00\x00\x00\xa8\xf0\x54\x17\x19\x00\x00\x00\x44\x00\x00\x00\x30\xf3\x4e\x5c\x7f\xa6\x07\x00"
      "\x00\x56\x7a\x0e\xc5\x04\x00\x00\x00\x10\x55\x63\xd9\x1c\x00\x00\x00\x14\x00\x00\x00\x31\x5d\x76\x43\x80\xa6"
      "\x07\x00\x00\xca\x4a\x7a\xc6\x04\x00\x00\x00\x80\x85\x66\x16\x19\x00\x00\x00\x33\x00\x00\x00\x30\x5e\x76\x43"
      "\x80\xa6\x07\x00\x00\xcb\x4a\x7a\xc6\x04\x00\x00\x00\x58\x1a\x78\x15\x19\x00\x00\x00\x39\x00\x00\x00\x30\x5f"
      "\x76\x43\x80\xa6\x07\x00\x00\xcc\x4a\x7a\xc6\x04\x00\x00\x00\x30\xaf\x89\x14\x19\x00\x00\x00\x41\x00\x00\x00"
      "\x30\x60\x76\x43\x80\xa6\x07\x00\x00\xcd\x4a\x7a\xc6\x04\x00\x00\x00\x08\x44\x9b\x13\x19\x00\x00\x00\x36\x00"
      "\x00\x00\x30\x61\x76\x43\x80\xa6\x07\x00\x00\xce\x4a\x7a\xc6\x04\x00\x00\x00\xe0\xd8\xac\x12\x19\x00\x00\x00"
      "\x42\x00\x00\x00\x30\x62\x76\x43\x80\xa6\x07\x00\x00\xcf\x4a\x7a\xc6\x04\x00\x00\x00\xb8\x6d\xbe\x11\x19\x00"
      "\x00\x00\x3b\x00\x00\x00\x30\x63\x76\x43\x80\xa6\x07\x00\x00\xd0\x4a\x7a\xc6\x04\x00\x00\x00\x90\x02\xd0\x10"
      "\x19\x00\x00\x00\x36\x00\x00\x00\x30\x64\x76\x43\x80\xa6\x07\x00\x00\xd1\x4a\x7a\xc6\x04\x00\x00\x00\x68\x97"
      "\xe1\x0f\x19\x00\x00\x00\x33\x00\x00\x00\x30\x65\x76\x43\x80\xa6\x07\x00\x00\xd2\x4a\x7a\xc6\x04\x00\x00\x00"
      "\x40\x2c\xf3\x0e\x19\x00\x00\x00\x32\x00\x00\x00\x30\x66\x76\x43\x80\xa6\x07\x00\x00\xd3\x4a\x7a\xc6\x04\x00"
      "\x00\x00\x18\xc1\x04\x0e\x19\x00\x00\x00\x33\x00\x00\x00\x30\x67\x76\x43\x80\xa6\x07\x00\x00\xd4\x4a\x7a\xc6"
      "\x04\x00\x00\x00\xf0\x55\x16\x0d\x19\x00\x00\x00\x36\x00\x00\x00\x30\x68\x76\x43\x80\xa6\x07\x00\x00\xd5\x4a"
      "\x7a\xc6\x04\x00\x00\x00\xc8\xea\x27\x0c\x19\x00\x00\x00\x3b\x00\x00\x00\x30\x69\x76\x43\x80\xa6\x07\x00\x00"
      "\xd6\x4a\x7a\xc6\x04\x00\x00\x00\xa0\x7f\x39\x0b\x19\x00\x00\x00\x42\x00\x00\x00\x30\x6a\x76\x43\x80\xa6\x07"
      "\x00\x00\xd7\x4a\x7a\xc6\x04\x00\x00\x00\x78\x14\x4b\x0a\x19\x00\x00\x00\x36\x00\x00\x00\x30\x02\x51\xec\x78"
      "\xa6\x07\x00\x00\x36\x73\x18\xbb\x04\x00\x00\x00\x00\x6e\xb2\xe9\x18\x00\x00\x00\x16\x00\x00\x00\x30"
      "\xe2\x00"  // message size (226)
      "\x3b\x00"  // block length (59)
      "\x34\x00"  // template id (52)
      "\x01\x00"  // schema id (1)
      "\x09\x00"  // version (9)
      "\x8c\x37\x18\x00\x64\x00\x00\x00\xdc\x19\x01\x00\x42\x00\x00\x00\xb7\xa9\xbf\x96\x15\x29\x41\x17\xc0\xd1\xd4\xbc"
      "\x7d\xfc\x40\x17\xc2\x4b\x11\xff\xff\xff\xff\xff\xff\xff\x7f\xca\x9a\x3b\x00\x00\x00\x00\x00\x40\x59\x73\x07\x00"
      "\x00\x00\x00\x16\x00\x07\xdc\x7b\xb2\xd3\x17\x00\x00\x00\x01\x00\x00\x00\x01\x00\x00\x00\x01\xff\xff\xff\x80\x30"
      "\x1a\x81\xfa\x38\x18\x00\x00\x00\x01\x00\x00\x00\x01\x00\x00\x00\x01\xff\xff\xff\x80\x31\x6e\x2e\x96\xd8\x17\x00"
      "\x00\x00\xff\xff\xff\x7f\xff\xff\xff\x7f\x7f\xff\xff\xff\x80\x4e\x1a\x81\xfa\x38\x18\x00\x00\x00\xff\xff\xff\x7f"
      "\xff\xff\xff\x7f\x7f\xff\xff\xff\x80\x4f\x72\xfe\x57\x0b\x18\x00\x00\x00\xff\xff\xff\x7f\xff\xff\xff\x7f\x7f\xbf"
      "\x4b\xff\x03\x36\xff\xff\xff\xff\xff\xff\xff\x7f\x00\x00\x00\x00\xff\xff\xff\x7f\x7f\xbf\x4b\xff\x80\x42\xff\xff"
      "\xff\xff\xff\xff\xff\x7f\x06\x00\x00\x00\xff\xff\xff\x7f\x7f\xbf\x4b\xff\x80\x43"
      "\x63\x00"  // message size (99)
      "\x1c\x00"  // block length (28)
      "\x35\x00"  // template id (53)
      "\x01\x00"  // schema id (1)
      "\x09\x00"  // version (9)
      "\x8c\x37\x18\x00\x64\x00\x00\x00\xdc\x19\x01\x00\x01\x00\x00\x00\x01\x00\x00\x00\xb7\xa9\xbf\x96\x15\x29\x41\x17"
      "\x1d\x00\x02\xb0\x18\x44\x80\xa6\x07\x00\x00\xb6\x64\x7b\xc6\x04\x00\x00\x00\xdc\x7b\xb2\xd3\x17\x00\x00\x00\x01"
      "\x00\x00\x00\x30\xb3\x18\x44\x80\xa6\x07\x00\x00\xb9\x64\x7b\xc6\x04\x00\x00\x00\x1a\x81\xfa\x38\x18\x00\x00\x00"
      "\x01\x00\x00\x00\x31"
      "\xe2\x00"  // message size (226)
      "\x3b\x00"  // block length (59)
      "\x34\x00"  // template id (52)
      "\x01\x00"  // schema id (1)
      "\x09\x00"  // version (9)
      "\x8c\x37\x18\x00\x64\x00\x00\x00\x18\xf3\x01\x00\x17\x16\x00\x00\x83\x99\x8f\xe8\xb1\x29\x41\x17\xc0\xd1\xd4\xbc"
      "\x7d\xfc\x40\x17\xc2\x4b\x11\xff\xff\xff\xff\xff\xff\xff\x7f\x50\xd6\xdc\x01\x00\x00\x00\x00\x00\x65\xcd\x1d\x00"
      "\x00\x00\x00\x16\x00\x07\xe0\xf7\x97\x02\x1f\x00\x00\x00\x0f\x00\x00\x00\x01\x00\x00\x00\x01\xff\xff\xff\x80\x30"
      "\x70\x27\xe8\x0b\x1f\x00\x00\x00\x0f\x00\x00\x00\x01\x00\x00\x00\x01\xff\xff\xff\x80\x31\x70\x27\xe8\x0b\x1f\x00"
      "\x00\x00\xff\xff\xff\x7f\xff\xff\xff\x7f\x7f\xff\xff\xff\x80\x4e\x80\x3f\x84\xe8\x1e\x00\x00\x00\xff\xff\xff\x7f"
      "\xff\xff\xff\x7f\x7f\xff\xff\xff\x80\x4f\x00\x57\x38\x15\x1f\x00\x00\x00\xff\xff\xff\x7f\xff\xff\xff\x7f\x7f\xbf"
      "\x4b\xff\x03\x36\xff\xff\xff\xff\xff\xff\xff\x7f\x1b\x00\x00\x00\xff\xff\xff\x7f\x7f\xbf\x4b\xff\x80\x42\xff\xff"
      "\xff\xff\xff\xff\xff\x7f\x52\x01\x00\x00\xff\xff\xff\x7f\x7f\xbf\x4b\xff\x80\x43"sv;
  CHECK(std::size(message) == 1300);
  struct MyHandler final : public mdp::Parser::Handler {
    int count_52 = 0;
    int count_53 = 0;
    void operator()(mdp::Frame const &) override {}
    void operator()(Trace<::cme::sbe::mdp::ChannelReset4> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<::cme::sbe::mdp::AdminHeartbeat12> const &, mdp::Frame const &) override { FAIL(); }
    // - security status
    void operator()(Trace<::cme::sbe::mdp::SecurityStatus30> const &, mdp::Frame const &) override { FAIL(); }
    // - instrument definition
    void operator()(Trace<::cme::sbe::mdp::MDInstrumentDefinitionFuture54> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<::cme::sbe::mdp::MDInstrumentDefinitionOption55> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<::cme::sbe::mdp::MDInstrumentDefinitionSpread56> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<::cme::sbe::mdp::MDInstrumentDefinitionFixedIncome57> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<::cme::sbe::mdp::MDInstrumentDefinitionRepo58> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<::cme::sbe::mdp::MDInstrumentDefinitionFX63> const &, mdp::Frame const &) override { FAIL(); }
    // - SnapshotFullRefresh
    void operator()(Trace<::cme::sbe::mdp::SnapshotFullRefresh52> const &event, mdp::Frame const &) override {
      using value_type = std::remove_cvref_t<decltype(event)>::value_type;
      auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
      switch (++count_52) {
        case 1: {
          CHECK(value.lastMsgSeqNumProcessed() == 1587084);
          CHECK(value.totNumReports() == 100);
          CHECK(value.securityID() == 72156);
          CHECK(value.rptSeq() == 66);
          CHECK(value.transactTime() == 1675665709058730423);
          CHECK(value.lastUpdateTime() == 1675616678351000000);
          CHECK(value.tradeDate() == 19394);
          CHECK(value.mDSecurityTradingStatus() == ::cme::sbe::mdp::SecurityTradingStatus::ReadyToTrade);
          CHECK(std::isnan(map(value.highLimitPrice()).template get<double>()));
          CHECK(map(value.lowLimitPrice()).template get<double>() == 0.00390625_a);
          CHECK(map(value.maxPriceVariation()).template get<double>() == 0.125_a);
          value.sbeRewind();  // wtf!
          auto no_md_entries_rows = 0;
          value.noMDEntries().forEach([&no_md_entries_rows](auto &item) {
            switch (++no_md_entries_rows) {
              case 1:
                CHECK(map(item.mDEntryPx()).template get<double>() == 102.3359375_a);
                CHECK(mdp::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue()) == 1);
                CHECK(mdp::get_int(item.numberOfOrders(), item.numberOfOrdersNullValue()) == 1);
                CHECK(mdp::get_int(item.mDPriceLevel(), item.mDPriceLevelNullValue()) == 1);
                CHECK(mdp::get_int(item.tradingReferenceDate(), item.tradingReferenceDateNullValue()) == 0);
                CHECK(item.openCloseSettlFlag() == ::cme::sbe::mdp::OpenCloseSettlFlag::NULL_VALUE);
                CHECK(item.settlPriceType().nullValue());
                CHECK(item.mDEntryType() == ::cme::sbe::mdp::MDEntryType::Value::Bid);
                break;
              case 2:
                CHECK(map(item.mDEntryPx()).template get<double>() == 104.03515625_a);
                CHECK(mdp::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue()) == 1);
                CHECK(mdp::get_int(item.numberOfOrders(), item.numberOfOrdersNullValue()) == 1);
                CHECK(mdp::get_int(item.mDPriceLevel(), item.mDPriceLevelNullValue()) == 1);
                CHECK(mdp::get_int(item.tradingReferenceDate(), item.tradingReferenceDateNullValue()) == 0);
                CHECK(item.openCloseSettlFlag() == ::cme::sbe::mdp::OpenCloseSettlFlag::NULL_VALUE);
                CHECK(item.settlPriceType().nullValue());
                CHECK(item.mDEntryType() == ::cme::sbe::mdp::MDEntryType::Value::Offer);
                break;
              // ...
              case 6:
                CHECK(std::isnan(map(item.mDEntryPx()).template get<double>()));
                CHECK(mdp::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue()) == 0);
                CHECK(mdp::get_int(item.numberOfOrders(), item.numberOfOrdersNullValue()) == 0);
                CHECK(mdp::get_int(item.mDPriceLevel(), item.mDPriceLevelNullValue()) == 0);
                CHECK(mdp::get_int(item.tradingReferenceDate(), item.tradingReferenceDateNullValue()) == 19391);
                CHECK(item.openCloseSettlFlag() == ::cme::sbe::mdp::OpenCloseSettlFlag::NULL_VALUE);
                CHECK(item.settlPriceType().nullValue());
                CHECK(item.mDEntryType() == ::cme::sbe::mdp::MDEntryType::Value::ClearedVolume);
                break;
              case 7:
                CHECK(std::isnan(map(item.mDEntryPx()).template get<double>()));
                CHECK(mdp::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue()) == 6);
                CHECK(mdp::get_int(item.numberOfOrders(), item.numberOfOrdersNullValue()) == 0);
                CHECK(mdp::get_int(item.mDPriceLevel(), item.mDPriceLevelNullValue()) == 0);
                CHECK(mdp::get_int(item.tradingReferenceDate(), item.tradingReferenceDateNullValue()) == 19391);
                CHECK(item.openCloseSettlFlag() == ::cme::sbe::mdp::OpenCloseSettlFlag::NULL_VALUE);
                CHECK(item.settlPriceType().nullValue());
                CHECK(item.mDEntryType() == ::cme::sbe::mdp::MDEntryType::Value::OpenInterest);
                break;
            }
          });
          CHECK(no_md_entries_rows == 7);
          break;
        }
        case 2: {
          CHECK(value.lastMsgSeqNumProcessed() == 1587084);
          CHECK(value.totNumReports() == 100);
          CHECK(value.securityID() == 127768);
          CHECK(value.rptSeq() == 5655);
          CHECK(value.transactTime() == 1675666380446210435);
          CHECK(value.lastUpdateTime() == 1675616678351000000);
          CHECK(value.tradeDate() == 19394);
          CHECK(value.mDSecurityTradingStatus() == ::cme::sbe::mdp::SecurityTradingStatus::ReadyToTrade);
          CHECK(std::isnan(map(value.highLimitPrice()).template get<double>()));
          CHECK(map(value.lowLimitPrice()).template get<double>() == 0.03125_a);
          CHECK(map(value.maxPriceVariation()).template get<double>() == 0.5_a);
          value.sbeRewind();  // wtf!
          auto no_md_entries_rows = 0;
          value.noMDEntries().forEach([&no_md_entries_rows](auto &item) {
            switch (++no_md_entries_rows) {
              case 1:
                CHECK(map(item.mDEntryPx()).template get<double>() == 133.1875_a);
                CHECK(mdp::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue()) == 15);
                CHECK(mdp::get_int(item.numberOfOrders(), item.numberOfOrdersNullValue()) == 1);
                CHECK(mdp::get_int(item.mDPriceLevel(), item.mDPriceLevelNullValue()) == 1);
                CHECK(mdp::get_int(item.tradingReferenceDate(), item.tradingReferenceDateNullValue()) == 0);
                CHECK(item.openCloseSettlFlag() == ::cme::sbe::mdp::OpenCloseSettlFlag::NULL_VALUE);
                CHECK(item.settlPriceType().nullValue());
                CHECK(item.mDEntryType() == ::cme::sbe::mdp::MDEntryType::Value::Bid);
                break;
              case 2:
                CHECK(map(item.mDEntryPx()).template get<double>() == 133.34375_a);
                CHECK(mdp::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue()) == 15);
                CHECK(mdp::get_int(item.numberOfOrders(), item.numberOfOrdersNullValue()) == 1);
                CHECK(mdp::get_int(item.mDPriceLevel(), item.mDPriceLevelNullValue()) == 1);
                CHECK(mdp::get_int(item.tradingReferenceDate(), item.tradingReferenceDateNullValue()) == 0);
                CHECK(item.openCloseSettlFlag() == ::cme::sbe::mdp::OpenCloseSettlFlag::NULL_VALUE);
                CHECK(item.settlPriceType().nullValue());
                CHECK(item.mDEntryType() == ::cme::sbe::mdp::MDEntryType::Value::Offer);
                break;
              // ...
              case 6:
                CHECK(std::isnan(map(item.mDEntryPx()).template get<double>()));
                CHECK(mdp::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue()) == 27);
                CHECK(mdp::get_int(item.numberOfOrders(), item.numberOfOrdersNullValue()) == 0);
                CHECK(mdp::get_int(item.mDPriceLevel(), item.mDPriceLevelNullValue()) == 0);
                CHECK(mdp::get_int(item.tradingReferenceDate(), item.tradingReferenceDateNullValue()) == 19391);
                CHECK(item.openCloseSettlFlag() == ::cme::sbe::mdp::OpenCloseSettlFlag::NULL_VALUE);
                CHECK(item.settlPriceType().nullValue());
                CHECK(item.mDEntryType() == ::cme::sbe::mdp::MDEntryType::Value::ClearedVolume);
                break;
              case 7:
                CHECK(std::isnan(map(item.mDEntryPx()).template get<double>()));
                CHECK(mdp::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue()) == 338);
                CHECK(mdp::get_int(item.numberOfOrders(), item.numberOfOrdersNullValue()) == 0);
                CHECK(mdp::get_int(item.mDPriceLevel(), item.mDPriceLevelNullValue()) == 0);
                CHECK(mdp::get_int(item.tradingReferenceDate(), item.tradingReferenceDateNullValue()) == 19391);
                CHECK(item.openCloseSettlFlag() == ::cme::sbe::mdp::OpenCloseSettlFlag::NULL_VALUE);
                CHECK(item.settlPriceType().nullValue());
                CHECK(item.mDEntryType() == ::cme::sbe::mdp::MDEntryType::Value::OpenInterest);
                break;
            }
          });
          CHECK(no_md_entries_rows == 7);
          break;
        }
      }
    }
    void operator()(Trace<::cme::sbe::mdp::SnapshotFullRefreshOrderBook53> const &event, mdp::Frame const &) override {
      using value_type = std::remove_cvref_t<decltype(event)>::value_type;
      auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
      switch (++count_53) {
        case 1: {
          CHECK(value.lastMsgSeqNumProcessed() == 1587042);
          CHECK(value.totNumReports() == 100);
          CHECK(value.securityID() == 69078);
          CHECK(value.noChunks() == 242);
          CHECK(value.currentChunk() == 242);
          CHECK(value.transactTime() == 1675666401560808535);
          value.sbeRewind();  // wtf!
          auto no_md_entries_rows = 0;
          value.noMDEntries().forEach([&no_md_entries_rows](auto &item) {
            switch (++no_md_entries_rows) {
              case 1:
                CHECK(item.orderID() == 8411682716919);
                CHECK(mdp::get_int(item.mDOrderPriority(), item.mDOrderPriorityNullValue()) == 20485922177);
                CHECK(map(item.mDEntryPx()).template get<double>() == 123.84375_a);
                CHECK(item.mDDisplayQty() == 20);
                CHECK(item.mDEntryType() == ::cme::sbe::mdp::MDEntryTypeBook::Value::Offer);
                break;
              case 2:
                CHECK(item.orderID() == 8411691669301);
                CHECK(mdp::get_int(item.mDOrderPriority(), item.mDOrderPriorityNullValue()) == 20500262087);
                CHECK(map(item.mDEntryPx()).template get<double>() == 107.8125_a);
                CHECK(item.mDDisplayQty() == 68);
                CHECK(item.mDEntryType() == ::cme::sbe::mdp::MDEntryTypeBook::Value::Bid);
                break;
              // ...
              case 23:
                CHECK(item.orderID() == 8411697870442);
                CHECK(mdp::get_int(item.mDOrderPriority(), item.mDOrderPriorityNullValue()) == 20509772503);
                CHECK(map(item.mDEntryPx()).template get<double>() == 107.546875_a);
                CHECK(item.mDDisplayQty() == 54);
                CHECK(item.mDEntryType() == ::cme::sbe::mdp::MDEntryTypeBook::Value::Bid);
                break;
              case 24:
                CHECK(item.orderID() == 8411574718722);
                CHECK(mdp::get_int(item.mDOrderPriority(), item.mDOrderPriorityNullValue()) == 20318810934);
                CHECK(map(item.mDEntryPx()).template get<double>() == 107.0_a);
                CHECK(item.mDDisplayQty() == 22);
                CHECK(item.mDEntryType() == ::cme::sbe::mdp::MDEntryTypeBook::Value::Bid);
                break;
            }
          });
          CHECK(no_md_entries_rows == 24);
          break;
        }
        case 2: {
          CHECK(value.lastMsgSeqNumProcessed() == 1587084);
          CHECK(value.totNumReports() == 100);
          CHECK(value.securityID() == 72156);
          CHECK(value.noChunks() == 1);
          CHECK(value.currentChunk() == 1);
          CHECK(value.transactTime() == 1675665709058730423);
          value.sbeRewind();  // wtf!
          auto no_md_entries_rows = 0;
          value.noMDEntries().forEach([&no_md_entries_rows](auto &item) {
            switch (++no_md_entries_rows) {
              case 1:
                CHECK(item.orderID() == 8411697911984);
                CHECK(mdp::get_int(item.mDOrderPriority(), item.mDOrderPriorityNullValue()) == 20509844662);
                CHECK(map(item.mDEntryPx()).template get<double>() == 102.3359375_a);
                CHECK(item.mDDisplayQty() == 1);
                CHECK(item.mDEntryType() == ::cme::sbe::mdp::MDEntryTypeBook::Value::Bid);
                break;
              case 2:
                CHECK(item.orderID() == 8411697911987);
                CHECK(mdp::get_int(item.mDOrderPriority(), item.mDOrderPriorityNullValue()) == 20509844665);
                CHECK(map(item.mDEntryPx()).template get<double>() == 104.03515625_a);
                CHECK(item.mDDisplayQty() == 1);
                CHECK(item.mDEntryType() == ::cme::sbe::mdp::MDEntryTypeBook::Value::Offer);
                break;
            }
          });
          CHECK(no_md_entries_rows == 2);
          break;
        }
      }
    }
    void operator()(Trace<::cme::sbe::mdp::SnapshotFullRefreshLongQty69> const &, mdp::Frame const &) override { FAIL(); }
    // - MDIncrementalRefresh
    void operator()(Trace<::cme::sbe::mdp::MDIncrementalRefreshVolume37> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<::cme::sbe::mdp::MDIncrementalRefreshBook46> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<::cme::sbe::mdp::MDIncrementalRefreshOrderBook47> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<::cme::sbe::mdp::MDIncrementalRefreshTradeSummary48> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<::cme::sbe::mdp::MDIncrementalRefreshDailyStatistics49> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<::cme::sbe::mdp::MDIncrementalRefreshLimitsBanding50> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<::cme::sbe::mdp::MDIncrementalRefreshSessionStatistics51> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<::cme::sbe::mdp::MDIncrementalRefreshBookLongQty64> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<::cme::sbe::mdp::MDIncrementalRefreshTradeSummaryLongQty65> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<::cme::sbe::mdp::MDIncrementalRefreshVolumeLongQty66> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<::cme::sbe::mdp::MDIncrementalRefreshSessionStatisticsLongQty67> const &, mdp::Frame const &) override { FAIL(); }
    void operator()(Trace<::cme::sbe::mdp::QuoteRequest39> const &, mdp::Frame const &) override { FAIL(); }
  } handler;
  std::span buffer{reinterpret_cast<std::byte const *>(std::data(message)), std::size(message)};
  TraceInfo trace_info;
  auto res = mdp::Parser::dispatch(handler, buffer, trace_info);
  CHECK(res);
  CHECK(handler.count_52 == 2);
  CHECK(handler.count_53 == 2);
}
