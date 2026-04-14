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

TEST_CASE("simple", "[snapshot_full_refresh]") {
  constexpr auto const message =
      "\x08\x00\x00\x00"
      "\x6b\x26\xab\xa0\x43\xcf\x48\x17"
      "\x84\x02"
      "\x3b\x00"
      "\x34\x00"  // snapshot_full_refresh_52
      "\x01\x00"
      "\x09\x00"
      "\xcd\xb3\xef\x03\x27\x01\x00\x00\xb8\x25\x00\x00\xed\x20\x43\x00\x8b\xfd\x53\x4d\x42\xcf\x48\x17\x91\x83\x59\x22"
      "\x70\xb3\x48\x17\xdb\x4b\x11\xff\xff\xff\xff\xff\xff\xff\x7f\xff\xff\xff\xff\xff\xff\xff\x7f\x00\x65\xcd\x1d\x00"
      "\x00\x00\x00\x16\x00\x1a\xd8\x94\x11\xff\xff\xff\xff\xff\x08\x00\x00\x00\x01\x00\x00\x00\x01\xff\xff\xff\x80\x30"
      "\x44\x5f\x9a\xfe\xff\xff\xff\xff\x10\x00\x00\x00\x02\x00\x00\x00\x02\xff\xff\xff\x80\x30\xb0\x29\x23\xfe\xff\xff"
      "\xff\xff\x14\x00\x00\x00\x04\x00\x00\x00\x03\xff\xff\xff\x80\x30\x1c\xf4\xab\xfd\xff\xff\xff\xff\x25\x00\x00\x00"
      "\x05\x00\x00\x00\x04\xff\xff\xff\x80\x30\x88\xbe\x34\xfd\xff\xff\xff\xff\x1f\x00\x00\x00\x04\x00\x00\x00\x05\xff"
      "\xff\xff\x80\x30\xf4\x88\xbd\xfc\xff\xff\xff\xff\x1f\x00\x00\x00\x04\x00\x00\x00\x06\xff\xff\xff\x80\x30\x60\x53"
      "\x46\xfc\xff\xff\xff\xff\x0d\x00\x00\x00\x02\x00\x00\x00\x07\xff\xff\xff\x80\x30\xcc\x1d\xcf\xfb\xff\xff\xff\xff"
      "\x0d\x00\x00\x00\x02\x00\x00\x00\x08\xff\xff\xff\x80\x30\x38\xe8\x57\xfb\xff\xff\xff\xff\x0d\x00\x00\x00\x02\x00"
      "\x00\x00\x09\xff\xff\xff\x80\x30\x00\x00\x00\x00\x00\x00\x00\x00\x08\x00\x00\x00\x01\x00\x00\x00\x01\xff\xff\xff"
      "\x80\x31\x94\x35\x77\x00\x00\x00\x00\x00\x12\x00\x00\x00\x03\x00\x00\x00\x02\xff\xff\xff\x80\x31\x28\x6b\xee\x00"
      "\x00\x00\x00\x00\x17\x00\x00\x00\x04\x00\x00\x00\x03\xff\xff\xff\x80\x31\xbc\xa0\x65\x01\x00\x00\x00\x00\x2a\x00"
      "\x00\x00\x06\x00\x00\x00\x04\xff\xff\xff\x80\x31\x50\xd6\xdc\x01\x00\x00\x00\x00\x1f\x00\x00\x00\x04\x00\x00\x00"
      "\x05\xff\xff\xff\x80\x31\xe4\x0b\x54\x02\x00\x00\x00\x00\x1f\x00\x00\x00\x04\x00\x00\x00\x06\xff\xff\xff\x80\x31"
      "\x78\x41\xcb\x02\x00\x00\x00\x00\x0d\x00\x00\x00\x02\x00\x00\x00\x07\xff\xff\xff\x80\x31\x0c\x77\x42\x03\x00\x00"
      "\x00\x00\x08\x00\x00\x00\x01\x00\x00\x00\x08\xff\xff\xff\x80\x31\xa0\xac\xb9\x03\x00\x00\x00\x00\x08\x00\x00\x00"
      "\x01\x00\x00\x00\x09\xff\xff\xff\x80\x31\x34\xe2\x30\x04\x00\x00\x00\x00\x08\x00\x00\x00\x01\x00\x00\x00\x0a\xff"
      "\xff\xff\x80\x31\x44\x5f\x9a\xfe\xff\xff\xff\xff\x11\x00\x00\x00\xff\xff\xff\x7f\x01\xff\xff\xff\x80\x45\xb0\x29"
      "\x23\xfe\xff\xff\xff\xff\x1b\x00\x00\x00\xff\xff\xff\x7f\x02\xff\xff\xff\x80\x45\x28\x6b\xee\x00\x00\x00\x00\x00"
      "\x0d\x00\x00\x00\xff\xff\xff\x7f\x01\xff\xff\xff\x80\x46\xbc\xa0\x65\x01\x00\x00\x00\x00\x1f\x00\x00\x00\xff\xff"
      "\xff\x7f\x02\xff\xff\xff\x80\x46\x94\x35\x77\x00\x00\x00\x00\x00\xff\xff\xff\x7f\xff\xff\xff\x7f\x7f\xff\xff\xff"
      "\x80\x4e\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\x7f\xff\xff\xff\x7f\x7f\xff\xff\xff\x80\x4f\x00\x00\x00\x00"
      "\x00\x00\x00\x00\xff\xff\xff\x7f\xff\xff\xff\x7f\x7f\xda\x4b\xff\x03\x36"
      "\xb6\x00"
      "\x3b\x00"
      "\x34\x00"  // snapshot_full_refresh_52
      "\x01\x00"
      "\x09\x00"
      "\xcd\xb3\xef\x03\x27\x01\x00\x00\xee\x25\x00\x00\x70\x5c\x0a\x00\x3b\x44\xa1\x2c\x42\xcf\x48\x17\x91\x83\x59\x22"
      "\x70\xb3\x48\x17\xdb\x4b\x11\xff\xff\xff\xff\xff\xff\xff\x7f\xff\xff\xff\xff\xff\xff\xff\x7f\x00\x65\xcd\x1d\x00"
      "\x00\x00\x00\x16\x00\x05\x88\xbe\x34\xfd\xff\xff\xff\xff\x01\x00\x00\x00\xff\xff\xff\x7f\x01\xff\xff\xff\x80\x45"
      "\x2a\xee\x81\xfc\xff\xff\xff\xff\x01\x00\x00\x00\xff\xff\xff\x7f\x02\xff\xff\xff\x80\x45\x5c\x4d\x1f\x05\x00\x00"
      "\x00\x00\x01\x00\x00\x00\xff\xff\xff\x7f\x01\xff\xff\xff\x80\x46\xe2\x88\xc0\x06\x00\x00\x00\x00\x01\x00\x00\x00"
      "\xff\xff\xff\x7f\x02\xff\xff\xff\x80\x46\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\x7f\xff\xff\xff\x7f\x7f\xda"
      "\x4b\xff\x03\x36"
      "\xb6\x00"
      "\x3b\x00"
      "\x34\x00"  // snapshot_full_refresh_52
      "\x01\x00"
      "\x09\x00"
      "\xcd\xb3\xef\x03\x27\x01\x00\x00\x6c\x26\x00\x00\x14\x7b\x07\x00\x51\x51\x7c\x0f\x41\xcf\x48\x17\x91\x83\x59\x22"
      "\x70\xb3\x48\x17\xdb\x4b\x11\xff\xff\xff\xff\xff\xff\xff\x7f\xff\xff\xff\xff\xff\xff\xff\x7f\x80\xb2\xe6\x0e\x00"
      "\x00\x00\x00\x16\x00\x05\x46\x7d\x60\xdc\xff\xff\xff\xff\x02\x00\x00\x00\xff\xff\xff\x7f\x01\xff\xff\xff\x80\x45"
      "\x7c\xe2\x24\xdc\xff\xff\xff\xff\x02\x00\x00\x00\xff\xff\xff\x7f\x02\xff\xff\xff\x80\x45\x02\x83\x93\xfb\xff\xff"
      "\xff\xff\x01\x00\x00\x00\xff\xff\xff\x7f\x01\xff\xff\xff\x80\x46\xcc\x1d\xcf\xfb\xff\xff\xff\xff\x01\x00\x00\x00"
      "\xff\xff\xff\x7f\x02\xff\xff\xff\x80\x46\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\x7f\xff\xff\xff\x7f\x7f\xda"
      "\x4b\xff\x03\x36"
      "\x5e\x00"
      "\x3b\x00"
      "\x34\x00"  // snapshot_full_refresh_52
      "\x01\x00"
      "\x09\x00"
      "\xcd\xb3\xef\x03\x27\x01\x00\x00\x99\x26\x00\x00\x1f\x17\x03\x00\xf7\xa9\x27\x0c\x5b\xbf\x48\x17\x91\x83\x59\x22"
      "\x70\xb3\x48\x17\xdb\x4b\x11\xff\xff\xff\xff\xff\xff\xff\x7f\xff\xff\xff\xff\xff\xff\xff\x7f\x00\xa8\x9c\x13\x46"
      "\x02\x00\x00\x16\x00\x01\x00\x1c\xc0\x2e\x9a\x19\x00\x00\xff\xff\xff\x7f\xff\xff\xff\x7f\x7f\xda\x4b\xff\x03\x36"
      "\xb6\x00"
      "\x3b\x00"
      "\x34\x00"  // snapshot_full_refresh_52
      "\x01\x00"
      "\x09\x00"
      "\xcd\xb3\xef\x03\x27\x01\x00\x00\xa6\x26\x00\x00\x7a\x6d\x08\x00\x51\x51\x7c\x0f\x41\xcf\x48\x17\x91\x83\x59\x22"
      "\x70\xb3\x48\x17\xdb\x4b\x11\xff\xff\xff\xff\xff\xff\xff\x7f\xff\xff\xff\xff\xff\xff\xff\x7f\x80\xb2\xe6\x0e\x00"
      "\x00\x00\x00\x16\x00\x05\x70\x06\x15\xbb\xff\xff\xff\xff\x06\x00\x00\x00\xff\xff\xff\x7f\x01\xff\xff\xff\x80\x45"
      "\x20\x30\x38\xb9\xff\xff\xff\xff\x01\x00\x00\x00\xff\xff\xff\x7f\x02\xff\xff\xff\x80\x45\x08\x0c\x4e\xee\xff\xff"
      "\xff\xff\x06\x00\x00\x00\xff\xff\xff\x7f\x01\xff\xff\xff\x80\x46\xe8\xdb\x15\x35\x00\x00\x00\x00\x01\x00\x00\x00"
      "\xff\xff\xff\x7f\x02\xff\xff\xff\x80\x46\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\x7f\xff\xff\xff\x7f\x7f\xda"
      "\x4b\xff\x03\x36"
      "\x5e\x00"
      "\x3b\x00"
      "\x34\x00"  // snapshot_full_refresh_52
      "\x01\x00"
      "\x09\x00"
      "\xcd\xb3\xef\x03\x27\x01\x00\x00\xb1\x26\x00\x00\x2f\xe1\x00\x00\x9b\x29\x0a\x0f\x5b\xbf\x48\x17\x91\x83\x59\x22"
      "\x70\xb3\x48\x17\xdb\x4b\x11\xff\xff\xff\xff\xff\xff\xff\x7f\xff\xff\xff\xff\xff\xff\xff\x7f\x40\x59\x73\x07\x00"
      "\x00\x00\x00\x16\x00\x01\xe4\x0b\x54\x02\x00\x00\x00\x00\xff\xff\xff\x7f\xff\xff\xff\x7f\x7f\xda\x4b\xff\x03"
      "\x36"sv;
  static_assert(std::size(message) == 1390);
  struct MyHandler final : public mdp::Parser::Handler {
    int counter = 0;
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
      // fmt::print("{}\n", value);
      switch (++counter) {
        case 1: {
          CHECK(value.lastMsgSeqNumProcessed() == 66040781);
          CHECK(value.totNumReports() == 295);
          CHECK(value.securityID() == 9656);
          CHECK(value.rptSeq() == 4399341);
          CHECK(value.transactTime() == 1677818744867650955);
          CHECK(value.lastUpdateTime() == 1677788155389510545);
          CHECK(value.tradeDate() == 19419);
          CHECK(value.mDSecurityTradingStatus() == ::cme::sbe::mdp::SecurityTradingStatus::ReadyToTrade);
          CHECK(std::isnan(map(value.highLimitPrice()).template get<double>()));
          CHECK(std::isnan(map(value.lowLimitPrice()).template get<double>()));
          CHECK(map(value.maxPriceVariation()).template get<double>() == 0.5_a);
          value.sbeRewind();  // wtf!
          auto no_md_entries_rows = 0;
          value.noMDEntries().forEach([&no_md_entries_rows](auto &item) {
            switch (++no_md_entries_rows) {
              case 1:
                CHECK(map(item.mDEntryPx()).template get<double>() == -0.015625_a);
                CHECK(mdp::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue()) == 8);
                CHECK(mdp::get_int(item.numberOfOrders(), item.numberOfOrdersNullValue()) == 1);
                CHECK(mdp::get_int(item.mDPriceLevel(), item.mDPriceLevelNullValue()) == 1);
                CHECK(mdp::get_int(item.tradingReferenceDate(), item.tradingReferenceDateNullValue()) == 0);  // null
                CHECK(item.openCloseSettlFlag() == ::cme::sbe::mdp::OpenCloseSettlFlag::NULL_VALUE);
                CHECK(item.settlPriceType().nullValue());
                CHECK(item.mDEntryType() == ::cme::sbe::mdp::MDEntryType::Value::Bid);
                break;
              case 2:
                CHECK(map(item.mDEntryPx()).template get<double>() == -0.0234375_a);
                CHECK(mdp::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue()) == 16);
                CHECK(mdp::get_int(item.numberOfOrders(), item.numberOfOrdersNullValue()) == 2);
                CHECK(mdp::get_int(item.mDPriceLevel(), item.mDPriceLevelNullValue()) == 2);
                CHECK(mdp::get_int(item.tradingReferenceDate(), item.tradingReferenceDateNullValue()) == 0);
                CHECK(item.openCloseSettlFlag() == ::cme::sbe::mdp::OpenCloseSettlFlag::NULL_VALUE);
                CHECK(item.settlPriceType().nullValue());
                CHECK(item.mDEntryType() == ::cme::sbe::mdp::MDEntryType::Value::Bid);
                break;
              case 3:
                CHECK(map(item.mDEntryPx()).template get<double>() == -0.03125_a);
                CHECK(mdp::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue()) == 20);
                CHECK(mdp::get_int(item.numberOfOrders(), item.numberOfOrdersNullValue()) == 4);
                CHECK(mdp::get_int(item.mDPriceLevel(), item.mDPriceLevelNullValue()) == 3);
                CHECK(mdp::get_int(item.tradingReferenceDate(), item.tradingReferenceDateNullValue()) == 0);
                CHECK(item.openCloseSettlFlag() == ::cme::sbe::mdp::OpenCloseSettlFlag::NULL_VALUE);
                CHECK(item.settlPriceType().nullValue());
                CHECK(item.mDEntryType() == ::cme::sbe::mdp::MDEntryType::Value::Bid);
                break;
              case 4:
                CHECK(map(item.mDEntryPx()).template get<double>() == -0.0390625_a);
                CHECK(mdp::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue()) == 37);
                CHECK(mdp::get_int(item.numberOfOrders(), item.numberOfOrdersNullValue()) == 5);
                CHECK(mdp::get_int(item.mDPriceLevel(), item.mDPriceLevelNullValue()) == 4);
                CHECK(mdp::get_int(item.tradingReferenceDate(), item.tradingReferenceDateNullValue()) == 0);
                CHECK(item.openCloseSettlFlag() == ::cme::sbe::mdp::OpenCloseSettlFlag::NULL_VALUE);
                CHECK(item.settlPriceType().nullValue());
                CHECK(item.mDEntryType() == ::cme::sbe::mdp::MDEntryType::Value::Bid);
                break;
              case 5:
                CHECK(map(item.mDEntryPx()).template get<double>() == -0.046875_a);
                CHECK(mdp::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue()) == 31);
                CHECK(mdp::get_int(item.numberOfOrders(), item.numberOfOrdersNullValue()) == 4);
                CHECK(mdp::get_int(item.mDPriceLevel(), item.mDPriceLevelNullValue()) == 5);
                CHECK(mdp::get_int(item.tradingReferenceDate(), item.tradingReferenceDateNullValue()) == 0);
                CHECK(item.openCloseSettlFlag() == ::cme::sbe::mdp::OpenCloseSettlFlag::NULL_VALUE);
                CHECK(item.settlPriceType().nullValue());
                CHECK(item.mDEntryType() == ::cme::sbe::mdp::MDEntryType::Value::Bid);
                break;
              case 6:
                CHECK(map(item.mDEntryPx()).template get<double>() == -0.0546875_a);
                CHECK(mdp::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue()) == 31);
                CHECK(mdp::get_int(item.numberOfOrders(), item.numberOfOrdersNullValue()) == 4);
                CHECK(mdp::get_int(item.mDPriceLevel(), item.mDPriceLevelNullValue()) == 6);
                CHECK(mdp::get_int(item.tradingReferenceDate(), item.tradingReferenceDateNullValue()) == 0);
                CHECK(item.openCloseSettlFlag() == ::cme::sbe::mdp::OpenCloseSettlFlag::NULL_VALUE);
                CHECK(item.settlPriceType().nullValue());
                CHECK(item.mDEntryType() == ::cme::sbe::mdp::MDEntryType::Value::Bid);
                break;
              case 7:
                CHECK(map(item.mDEntryPx()).template get<double>() == -0.0625_a);
                CHECK(mdp::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue()) == 13);
                CHECK(mdp::get_int(item.numberOfOrders(), item.numberOfOrdersNullValue()) == 2);
                CHECK(mdp::get_int(item.mDPriceLevel(), item.mDPriceLevelNullValue()) == 7);
                CHECK(mdp::get_int(item.tradingReferenceDate(), item.tradingReferenceDateNullValue()) == 0);
                CHECK(item.openCloseSettlFlag() == ::cme::sbe::mdp::OpenCloseSettlFlag::NULL_VALUE);
                CHECK(item.settlPriceType().nullValue());
                CHECK(item.mDEntryType() == ::cme::sbe::mdp::MDEntryType::Value::Bid);
                break;
              case 8:
                CHECK(map(item.mDEntryPx()).template get<double>() == -0.0703125_a);
                CHECK(mdp::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue()) == 13);
                CHECK(mdp::get_int(item.numberOfOrders(), item.numberOfOrdersNullValue()) == 2);
                CHECK(mdp::get_int(item.mDPriceLevel(), item.mDPriceLevelNullValue()) == 8);
                CHECK(mdp::get_int(item.tradingReferenceDate(), item.tradingReferenceDateNullValue()) == 0);
                CHECK(item.openCloseSettlFlag() == ::cme::sbe::mdp::OpenCloseSettlFlag::NULL_VALUE);
                CHECK(item.settlPriceType().nullValue());
                CHECK(item.mDEntryType() == ::cme::sbe::mdp::MDEntryType::Value::Bid);
                break;
              case 9:
                CHECK(map(item.mDEntryPx()).template get<double>() == -0.078125_a);
                CHECK(mdp::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue()) == 13);
                CHECK(mdp::get_int(item.numberOfOrders(), item.numberOfOrdersNullValue()) == 2);
                CHECK(mdp::get_int(item.mDPriceLevel(), item.mDPriceLevelNullValue()) == 9);
                CHECK(mdp::get_int(item.tradingReferenceDate(), item.tradingReferenceDateNullValue()) == 0);
                CHECK(item.openCloseSettlFlag() == ::cme::sbe::mdp::OpenCloseSettlFlag::NULL_VALUE);
                CHECK(item.settlPriceType().nullValue());
                CHECK(item.mDEntryType() == ::cme::sbe::mdp::MDEntryType::Value::Bid);
                break;
              case 10:
                CHECK(map(item.mDEntryPx()).template get<double>() == 0.0_a);
                CHECK(mdp::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue()) == 8);
                CHECK(mdp::get_int(item.numberOfOrders(), item.numberOfOrdersNullValue()) == 1);
                CHECK(mdp::get_int(item.mDPriceLevel(), item.mDPriceLevelNullValue()) == 1);
                CHECK(mdp::get_int(item.tradingReferenceDate(), item.tradingReferenceDateNullValue()) == 0);
                CHECK(item.openCloseSettlFlag() == ::cme::sbe::mdp::OpenCloseSettlFlag::NULL_VALUE);
                CHECK(item.settlPriceType().nullValue());
                CHECK(item.mDEntryType() == ::cme::sbe::mdp::MDEntryType::Value::Offer);
                break;
              case 11:
                CHECK(map(item.mDEntryPx()).template get<double>() == 0.0078125_a);
                CHECK(mdp::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue()) == 18);
                CHECK(mdp::get_int(item.numberOfOrders(), item.numberOfOrdersNullValue()) == 3);
                CHECK(mdp::get_int(item.mDPriceLevel(), item.mDPriceLevelNullValue()) == 2);
                CHECK(mdp::get_int(item.tradingReferenceDate(), item.tradingReferenceDateNullValue()) == 0);
                CHECK(item.openCloseSettlFlag() == ::cme::sbe::mdp::OpenCloseSettlFlag::NULL_VALUE);
                CHECK(item.settlPriceType().nullValue());
                CHECK(item.mDEntryType() == ::cme::sbe::mdp::MDEntryType::Value::Offer);
                break;
              case 12:
                CHECK(map(item.mDEntryPx()).template get<double>() == 0.015625_a);
                CHECK(mdp::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue()) == 23);
                CHECK(mdp::get_int(item.numberOfOrders(), item.numberOfOrdersNullValue()) == 4);
                CHECK(mdp::get_int(item.mDPriceLevel(), item.mDPriceLevelNullValue()) == 3);
                CHECK(mdp::get_int(item.tradingReferenceDate(), item.tradingReferenceDateNullValue()) == 0);
                CHECK(item.openCloseSettlFlag() == ::cme::sbe::mdp::OpenCloseSettlFlag::NULL_VALUE);
                CHECK(item.settlPriceType().nullValue());
                CHECK(item.mDEntryType() == ::cme::sbe::mdp::MDEntryType::Value::Offer);
                break;
              case 13:
                CHECK(map(item.mDEntryPx()).template get<double>() == 0.0234375_a);
                CHECK(mdp::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue()) == 42);
                CHECK(mdp::get_int(item.numberOfOrders(), item.numberOfOrdersNullValue()) == 6);
                CHECK(mdp::get_int(item.mDPriceLevel(), item.mDPriceLevelNullValue()) == 4);
                CHECK(mdp::get_int(item.tradingReferenceDate(), item.tradingReferenceDateNullValue()) == 0);
                CHECK(item.openCloseSettlFlag() == ::cme::sbe::mdp::OpenCloseSettlFlag::NULL_VALUE);
                CHECK(item.settlPriceType().nullValue());
                CHECK(item.mDEntryType() == ::cme::sbe::mdp::MDEntryType::Value::Offer);
                break;
              case 14:
                CHECK(map(item.mDEntryPx()).template get<double>() == 0.03125_a);
                CHECK(mdp::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue()) == 31);
                CHECK(mdp::get_int(item.numberOfOrders(), item.numberOfOrdersNullValue()) == 4);
                CHECK(mdp::get_int(item.mDPriceLevel(), item.mDPriceLevelNullValue()) == 5);
                CHECK(mdp::get_int(item.tradingReferenceDate(), item.tradingReferenceDateNullValue()) == 0);
                CHECK(item.openCloseSettlFlag() == ::cme::sbe::mdp::OpenCloseSettlFlag::NULL_VALUE);
                CHECK(item.settlPriceType().nullValue());
                CHECK(item.mDEntryType() == ::cme::sbe::mdp::MDEntryType::Value::Offer);
                break;
              case 15:
                CHECK(map(item.mDEntryPx()).template get<double>() == 0.0390625_a);
                CHECK(mdp::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue()) == 31);
                CHECK(mdp::get_int(item.numberOfOrders(), item.numberOfOrdersNullValue()) == 4);
                CHECK(mdp::get_int(item.mDPriceLevel(), item.mDPriceLevelNullValue()) == 6);
                CHECK(mdp::get_int(item.tradingReferenceDate(), item.tradingReferenceDateNullValue()) == 0);
                CHECK(item.openCloseSettlFlag() == ::cme::sbe::mdp::OpenCloseSettlFlag::NULL_VALUE);
                CHECK(item.settlPriceType().nullValue());
                CHECK(item.mDEntryType() == ::cme::sbe::mdp::MDEntryType::Value::Offer);
                break;
              case 16:
                CHECK(map(item.mDEntryPx()).template get<double>() == 0.046875_a);
                CHECK(mdp::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue()) == 13);
                CHECK(mdp::get_int(item.numberOfOrders(), item.numberOfOrdersNullValue()) == 2);
                CHECK(mdp::get_int(item.mDPriceLevel(), item.mDPriceLevelNullValue()) == 7);
                CHECK(mdp::get_int(item.tradingReferenceDate(), item.tradingReferenceDateNullValue()) == 0);
                CHECK(item.openCloseSettlFlag() == ::cme::sbe::mdp::OpenCloseSettlFlag::NULL_VALUE);
                CHECK(item.settlPriceType().nullValue());
                CHECK(item.mDEntryType() == ::cme::sbe::mdp::MDEntryType::Value::Offer);
                break;
              case 17:
                CHECK(map(item.mDEntryPx()).template get<double>() == 0.0546875_a);
                CHECK(mdp::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue()) == 8);
                CHECK(mdp::get_int(item.numberOfOrders(), item.numberOfOrdersNullValue()) == 1);
                CHECK(mdp::get_int(item.mDPriceLevel(), item.mDPriceLevelNullValue()) == 8);
                CHECK(mdp::get_int(item.tradingReferenceDate(), item.tradingReferenceDateNullValue()) == 0);
                CHECK(item.openCloseSettlFlag() == ::cme::sbe::mdp::OpenCloseSettlFlag::NULL_VALUE);
                CHECK(item.settlPriceType().nullValue());
                CHECK(item.mDEntryType() == ::cme::sbe::mdp::MDEntryType::Value::Offer);
                break;
              case 18:
                CHECK(map(item.mDEntryPx()).template get<double>() == 0.0625_a);
                CHECK(mdp::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue()) == 8);
                CHECK(mdp::get_int(item.numberOfOrders(), item.numberOfOrdersNullValue()) == 1);
                CHECK(mdp::get_int(item.mDPriceLevel(), item.mDPriceLevelNullValue()) == 9);
                CHECK(mdp::get_int(item.tradingReferenceDate(), item.tradingReferenceDateNullValue()) == 0);
                CHECK(item.openCloseSettlFlag() == ::cme::sbe::mdp::OpenCloseSettlFlag::NULL_VALUE);
                CHECK(item.settlPriceType().nullValue());
                CHECK(item.mDEntryType() == ::cme::sbe::mdp::MDEntryType::Value::Offer);
                break;
              case 19:
                CHECK(map(item.mDEntryPx()).template get<double>() == 0.0703125_a);
                CHECK(mdp::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue()) == 8);
                CHECK(mdp::get_int(item.numberOfOrders(), item.numberOfOrdersNullValue()) == 1);
                CHECK(mdp::get_int(item.mDPriceLevel(), item.mDPriceLevelNullValue()) == 10);
                CHECK(mdp::get_int(item.tradingReferenceDate(), item.tradingReferenceDateNullValue()) == 0);
                CHECK(item.openCloseSettlFlag() == ::cme::sbe::mdp::OpenCloseSettlFlag::NULL_VALUE);
                CHECK(item.settlPriceType().nullValue());
                CHECK(item.mDEntryType() == ::cme::sbe::mdp::MDEntryType::Value::Offer);
                break;
              case 20:
                CHECK(map(item.mDEntryPx()).template get<double>() == -0.0234375_a);
                CHECK(mdp::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue()) == 17);
                CHECK(mdp::get_int(item.numberOfOrders(), item.numberOfOrdersNullValue()) == 0);
                CHECK(mdp::get_int(item.mDPriceLevel(), item.mDPriceLevelNullValue()) == 1);
                CHECK(mdp::get_int(item.tradingReferenceDate(), item.tradingReferenceDateNullValue()) == 0);
                CHECK(item.openCloseSettlFlag() == ::cme::sbe::mdp::OpenCloseSettlFlag::NULL_VALUE);
                CHECK(item.settlPriceType().nullValue());
                CHECK(item.mDEntryType() == ::cme::sbe::mdp::MDEntryType::Value::ImpliedBid);
                break;
              case 21:
                CHECK(map(item.mDEntryPx()).template get<double>() == -0.03125_a);
                CHECK(mdp::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue()) == 27);
                CHECK(mdp::get_int(item.numberOfOrders(), item.numberOfOrdersNullValue()) == 0);
                CHECK(mdp::get_int(item.mDPriceLevel(), item.mDPriceLevelNullValue()) == 2);
                CHECK(mdp::get_int(item.tradingReferenceDate(), item.tradingReferenceDateNullValue()) == 0);
                CHECK(item.openCloseSettlFlag() == ::cme::sbe::mdp::OpenCloseSettlFlag::NULL_VALUE);
                CHECK(item.settlPriceType().nullValue());
                CHECK(item.mDEntryType() == ::cme::sbe::mdp::MDEntryType::Value::ImpliedBid);
                break;
              case 22:
                CHECK(map(item.mDEntryPx()).template get<double>() == 0.015625_a);
                CHECK(mdp::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue()) == 13);
                CHECK(mdp::get_int(item.numberOfOrders(), item.numberOfOrdersNullValue()) == 0);
                CHECK(mdp::get_int(item.mDPriceLevel(), item.mDPriceLevelNullValue()) == 1);
                CHECK(mdp::get_int(item.tradingReferenceDate(), item.tradingReferenceDateNullValue()) == 0);
                CHECK(item.openCloseSettlFlag() == ::cme::sbe::mdp::OpenCloseSettlFlag::NULL_VALUE);
                CHECK(item.settlPriceType().nullValue());
                CHECK(item.mDEntryType() == ::cme::sbe::mdp::MDEntryType::Value::ImpliedOffer);
                break;
              case 23:
                CHECK(map(item.mDEntryPx()).template get<double>() == 0.0234375_a);
                CHECK(mdp::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue()) == 31);
                CHECK(mdp::get_int(item.numberOfOrders(), item.numberOfOrdersNullValue()) == 0);
                CHECK(mdp::get_int(item.mDPriceLevel(), item.mDPriceLevelNullValue()) == 2);
                CHECK(mdp::get_int(item.tradingReferenceDate(), item.tradingReferenceDateNullValue()) == 0);
                CHECK(item.openCloseSettlFlag() == ::cme::sbe::mdp::OpenCloseSettlFlag::NULL_VALUE);
                CHECK(item.settlPriceType().nullValue());
                CHECK(item.mDEntryType() == ::cme::sbe::mdp::MDEntryType::Value::ImpliedOffer);
                break;
              case 24:
                CHECK(map(item.mDEntryPx()).template get<double>() == 0.0078125_a);
                CHECK(mdp::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue()) == 0);
                CHECK(mdp::get_int(item.numberOfOrders(), item.numberOfOrdersNullValue()) == 0);
                CHECK(mdp::get_int(item.mDPriceLevel(), item.mDPriceLevelNullValue()) == 0);
                CHECK(mdp::get_int(item.tradingReferenceDate(), item.tradingReferenceDateNullValue()) == 0);
                CHECK(item.openCloseSettlFlag() == ::cme::sbe::mdp::OpenCloseSettlFlag::NULL_VALUE);
                CHECK(item.settlPriceType().nullValue());
                CHECK(item.mDEntryType() == ::cme::sbe::mdp::MDEntryType::Value::SessionHighBid);
                break;
              case 25:
                CHECK(map(item.mDEntryPx()).template get<double>() == 0.0_a);
                CHECK(mdp::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue()) == 0);
                CHECK(mdp::get_int(item.numberOfOrders(), item.numberOfOrdersNullValue()) == 0);
                CHECK(mdp::get_int(item.mDPriceLevel(), item.mDPriceLevelNullValue()) == 0);
                CHECK(mdp::get_int(item.tradingReferenceDate(), item.tradingReferenceDateNullValue()) == 0);
                CHECK(item.openCloseSettlFlag() == ::cme::sbe::mdp::OpenCloseSettlFlag::NULL_VALUE);
                CHECK(item.settlPriceType().nullValue());
                CHECK(item.mDEntryType() == ::cme::sbe::mdp::MDEntryType::Value::SessionLowOffer);
                break;
              case 26:
                CHECK(map(item.mDEntryPx()).template get<double>() == 0.0_a);
                CHECK(mdp::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue()) == 0);
                CHECK(mdp::get_int(item.numberOfOrders(), item.numberOfOrdersNullValue()) == 0);
                CHECK(mdp::get_int(item.mDPriceLevel(), item.mDPriceLevelNullValue()) == 0);
                CHECK(mdp::get_int(item.tradingReferenceDate(), item.tradingReferenceDateNullValue()) == 19418);
                CHECK(item.openCloseSettlFlag() == ::cme::sbe::mdp::OpenCloseSettlFlag::NULL_VALUE);
                CHECK(item.settlPriceType().rawValue() == 3);  // FinalDaily | Actual
                CHECK(item.mDEntryType() == ::cme::sbe::mdp::MDEntryType::Value::SettlementPrice);
                break;
            }
          });
          CHECK(no_md_entries_rows == 26);
          break;
        }
        case 2: {
          CHECK(value.lastMsgSeqNumProcessed() == 66040781);
          CHECK(value.totNumReports() == 295);
          CHECK(value.securityID() == 9710);
          CHECK(value.rptSeq() == 679024);
          CHECK(value.transactTime() == 1677818744319067195);
          CHECK(value.lastUpdateTime() == 1677788155389510545);
          CHECK(value.tradeDate() == 19419);
          CHECK(value.mDSecurityTradingStatus() == ::cme::sbe::mdp::SecurityTradingStatus::ReadyToTrade);
          CHECK(std::isnan(map(value.highLimitPrice()).template get<double>()));
          CHECK(std::isnan(map(value.lowLimitPrice()).template get<double>()));
          CHECK(map(value.maxPriceVariation()).template get<double>() == 0.5_a);
          value.sbeRewind();  // wtf!
          auto no_md_entries_rows = 0;
          value.noMDEntries().forEach([&no_md_entries_rows](auto &item) {
            switch (++no_md_entries_rows) {
              case 1:
                CHECK(map(item.mDEntryPx()).template get<double>() == -0.046875_a);
                CHECK(mdp::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue()) == 1);
                CHECK(mdp::get_int(item.numberOfOrders(), item.numberOfOrdersNullValue()) == 0);
                CHECK(mdp::get_int(item.mDPriceLevel(), item.mDPriceLevelNullValue()) == 1);
                CHECK(mdp::get_int(item.tradingReferenceDate(), item.tradingReferenceDateNullValue()) == 0);  // null
                CHECK(item.openCloseSettlFlag() == ::cme::sbe::mdp::OpenCloseSettlFlag::NULL_VALUE);
                CHECK(item.settlPriceType().nullValue());
                CHECK(item.mDEntryType() == ::cme::sbe::mdp::MDEntryType::Value::ImpliedBid);
                break;
              case 2:
                CHECK(map(item.mDEntryPx()).template get<double>() == -0.05859375_a);
                CHECK(mdp::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue()) == 1);
                CHECK(mdp::get_int(item.numberOfOrders(), item.numberOfOrdersNullValue()) == 0);
                CHECK(mdp::get_int(item.mDPriceLevel(), item.mDPriceLevelNullValue()) == 2);
                CHECK(mdp::get_int(item.tradingReferenceDate(), item.tradingReferenceDateNullValue()) == 0);  // null
                CHECK(item.openCloseSettlFlag() == ::cme::sbe::mdp::OpenCloseSettlFlag::NULL_VALUE);
                CHECK(item.settlPriceType().nullValue());
                CHECK(item.mDEntryType() == ::cme::sbe::mdp::MDEntryType::Value::ImpliedBid);
                break;
              case 3:
                CHECK(map(item.mDEntryPx()).template get<double>() == 0.0859375_a);
                CHECK(mdp::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue()) == 1);
                CHECK(mdp::get_int(item.numberOfOrders(), item.numberOfOrdersNullValue()) == 0);
                CHECK(mdp::get_int(item.mDPriceLevel(), item.mDPriceLevelNullValue()) == 1);
                CHECK(mdp::get_int(item.tradingReferenceDate(), item.tradingReferenceDateNullValue()) == 0);  // null
                CHECK(item.openCloseSettlFlag() == ::cme::sbe::mdp::OpenCloseSettlFlag::NULL_VALUE);
                CHECK(item.settlPriceType().nullValue());
                CHECK(item.mDEntryType() == ::cme::sbe::mdp::MDEntryType::Value::ImpliedOffer);
                break;
              case 4:
                CHECK(map(item.mDEntryPx()).template get<double>() == 0.11328125_a);
                CHECK(mdp::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue()) == 1);
                CHECK(mdp::get_int(item.numberOfOrders(), item.numberOfOrdersNullValue()) == 0);
                CHECK(mdp::get_int(item.mDPriceLevel(), item.mDPriceLevelNullValue()) == 2);
                CHECK(mdp::get_int(item.tradingReferenceDate(), item.tradingReferenceDateNullValue()) == 0);  // null
                CHECK(item.openCloseSettlFlag() == ::cme::sbe::mdp::OpenCloseSettlFlag::NULL_VALUE);
                CHECK(item.settlPriceType().nullValue());
                CHECK(item.mDEntryType() == ::cme::sbe::mdp::MDEntryType::Value::ImpliedOffer);
                break;
              case 5:
                CHECK(map(item.mDEntryPx()).template get<double>() == 0.0_a);
                CHECK(mdp::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue()) == 0);
                CHECK(mdp::get_int(item.numberOfOrders(), item.numberOfOrdersNullValue()) == 0);
                CHECK(mdp::get_int(item.mDPriceLevel(), item.mDPriceLevelNullValue()) == 0);
                CHECK(mdp::get_int(item.tradingReferenceDate(), item.tradingReferenceDateNullValue()) == 19418);
                CHECK(item.openCloseSettlFlag() == ::cme::sbe::mdp::OpenCloseSettlFlag::NULL_VALUE);
                CHECK(item.settlPriceType().rawValue() == 3);  // FinalDaily | Actual
                CHECK(item.mDEntryType() == ::cme::sbe::mdp::MDEntryType::Value::SettlementPrice);
                break;
            }
          });
          CHECK(no_md_entries_rows == 5);
          break;
        }
        case 3: {
          CHECK(value.lastMsgSeqNumProcessed() == 66040781);
          CHECK(value.totNumReports() == 295);
          CHECK(value.securityID() == 9836);
          CHECK(value.rptSeq() == 490260);
          CHECK(value.transactTime() == 1677818739535139153);
          CHECK(value.lastUpdateTime() == 1677788155389510545);
          CHECK(value.tradeDate() == 19419);
          CHECK(value.mDSecurityTradingStatus() == ::cme::sbe::mdp::SecurityTradingStatus::ReadyToTrade);
          CHECK(std::isnan(map(value.highLimitPrice()).template get<double>()));
          CHECK(std::isnan(map(value.lowLimitPrice()).template get<double>()));
          CHECK(map(value.maxPriceVariation()).template get<double>() == 0.25_a);
          value.sbeRewind();  // wtf!
          auto no_md_entries_rows = 0;
          value.noMDEntries().forEach([&no_md_entries_rows](auto &item) {
            switch (++no_md_entries_rows) {
              case 1:
                CHECK(map(item.mDEntryPx()).template get<double>() == -0.59765625_a);
                CHECK(mdp::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue()) == 2);
                CHECK(mdp::get_int(item.numberOfOrders(), item.numberOfOrdersNullValue()) == 0);
                CHECK(mdp::get_int(item.mDPriceLevel(), item.mDPriceLevelNullValue()) == 1);
                CHECK(mdp::get_int(item.tradingReferenceDate(), item.tradingReferenceDateNullValue()) == 0);  // null
                CHECK(item.openCloseSettlFlag() == ::cme::sbe::mdp::OpenCloseSettlFlag::NULL_VALUE);
                CHECK(item.settlPriceType().nullValue());
                CHECK(item.mDEntryType() == ::cme::sbe::mdp::MDEntryType::Value::ImpliedBid);
                break;
              case 2:
                CHECK(map(item.mDEntryPx()).template get<double>() == -0.6015625_a);
                CHECK(mdp::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue()) == 2);
                CHECK(mdp::get_int(item.numberOfOrders(), item.numberOfOrdersNullValue()) == 0);
                CHECK(mdp::get_int(item.mDPriceLevel(), item.mDPriceLevelNullValue()) == 2);
                CHECK(mdp::get_int(item.tradingReferenceDate(), item.tradingReferenceDateNullValue()) == 0);  // null
                CHECK(item.openCloseSettlFlag() == ::cme::sbe::mdp::OpenCloseSettlFlag::NULL_VALUE);
                CHECK(item.settlPriceType().nullValue());
                CHECK(item.mDEntryType() == ::cme::sbe::mdp::MDEntryType::Value::ImpliedBid);
                break;
              case 3:
                CHECK(map(item.mDEntryPx()).template get<double>() == -0.07421875_a);
                CHECK(mdp::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue()) == 1);
                CHECK(mdp::get_int(item.numberOfOrders(), item.numberOfOrdersNullValue()) == 0);
                CHECK(mdp::get_int(item.mDPriceLevel(), item.mDPriceLevelNullValue()) == 1);
                CHECK(mdp::get_int(item.tradingReferenceDate(), item.tradingReferenceDateNullValue()) == 0);  // null
                CHECK(item.openCloseSettlFlag() == ::cme::sbe::mdp::OpenCloseSettlFlag::NULL_VALUE);
                CHECK(item.settlPriceType().nullValue());
                CHECK(item.mDEntryType() == ::cme::sbe::mdp::MDEntryType::Value::ImpliedOffer);
                break;
              case 4:
                CHECK(map(item.mDEntryPx()).template get<double>() == -0.0703125_a);
                CHECK(mdp::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue()) == 1);
                CHECK(mdp::get_int(item.numberOfOrders(), item.numberOfOrdersNullValue()) == 0);
                CHECK(mdp::get_int(item.mDPriceLevel(), item.mDPriceLevelNullValue()) == 2);
                CHECK(mdp::get_int(item.tradingReferenceDate(), item.tradingReferenceDateNullValue()) == 0);  // null
                CHECK(item.openCloseSettlFlag() == ::cme::sbe::mdp::OpenCloseSettlFlag::NULL_VALUE);
                CHECK(item.settlPriceType().nullValue());
                CHECK(item.mDEntryType() == ::cme::sbe::mdp::MDEntryType::Value::ImpliedOffer);
                break;
              case 5:
                CHECK(map(item.mDEntryPx()).template get<double>() == 0.0_a);
                CHECK(mdp::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue()) == 0);
                CHECK(mdp::get_int(item.numberOfOrders(), item.numberOfOrdersNullValue()) == 0);
                CHECK(mdp::get_int(item.mDPriceLevel(), item.mDPriceLevelNullValue()) == 0);
                CHECK(mdp::get_int(item.tradingReferenceDate(), item.tradingReferenceDateNullValue()) == 19418);
                CHECK(item.openCloseSettlFlag() == ::cme::sbe::mdp::OpenCloseSettlFlag::NULL_VALUE);
                CHECK(item.settlPriceType().rawValue() == 3);  // FinalDaily | Actual
                CHECK(item.mDEntryType() == ::cme::sbe::mdp::MDEntryType::Value::SettlementPrice);
                break;
            }
          });
          CHECK(no_md_entries_rows == 5);
          break;
        }
        case 4: {
          CHECK(value.lastMsgSeqNumProcessed() == 66040781);
          CHECK(value.totNumReports() == 295);
          CHECK(value.securityID() == 9881);
          CHECK(value.rptSeq() == 202527);
          CHECK(value.transactTime() == 1677801258962364919);
          CHECK(value.lastUpdateTime() == 1677788155389510545);
          CHECK(value.tradeDate() == 19419);
          CHECK(value.mDSecurityTradingStatus() == ::cme::sbe::mdp::SecurityTradingStatus::ReadyToTrade);
          CHECK(std::isnan(map(value.highLimitPrice()).template get<double>()));
          CHECK(std::isnan(map(value.lowLimitPrice()).template get<double>()));
          CHECK(map(value.maxPriceVariation()).template get<double>() == 2500.0_a);
          value.sbeRewind();  // wtf!
          auto no_md_entries_rows = 0;
          value.noMDEntries().forEach([&no_md_entries_rows](auto &item) {
            switch (++no_md_entries_rows) {
              case 1:
                CHECK(map(item.mDEntryPx()).template get<double>() == 28150.0_a);
                CHECK(mdp::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue()) == 0);
                CHECK(mdp::get_int(item.numberOfOrders(), item.numberOfOrdersNullValue()) == 0);
                CHECK(mdp::get_int(item.mDPriceLevel(), item.mDPriceLevelNullValue()) == 0);
                CHECK(mdp::get_int(item.tradingReferenceDate(), item.tradingReferenceDateNullValue()) == 19418);
                CHECK(item.openCloseSettlFlag() == ::cme::sbe::mdp::OpenCloseSettlFlag::NULL_VALUE);
                CHECK(item.settlPriceType().rawValue() == 3);  // FinalDaily | Actual
                CHECK(item.mDEntryType() == ::cme::sbe::mdp::MDEntryType::Value::SettlementPrice);
                break;
            }
          });
          CHECK(no_md_entries_rows == 1);
          break;
        }
        case 5: {
          CHECK(value.lastMsgSeqNumProcessed() == 66040781);
          CHECK(value.totNumReports() == 295);
          CHECK(value.securityID() == 9894);
          CHECK(value.rptSeq() == 552314);
          CHECK(value.transactTime() == 1677818739535139153);
          CHECK(value.lastUpdateTime() == 1677788155389510545);
          CHECK(value.tradeDate() == 19419);
          CHECK(value.mDSecurityTradingStatus() == ::cme::sbe::mdp::SecurityTradingStatus::ReadyToTrade);
          CHECK(std::isnan(map(value.highLimitPrice()).template get<double>()));
          CHECK(std::isnan(map(value.lowLimitPrice()).template get<double>()));
          CHECK(map(value.maxPriceVariation()).template get<double>() == 0.25_a);
          value.sbeRewind();  // wtf!
          auto no_md_entries_rows = 0;
          value.noMDEntries().forEach([&no_md_entries_rows](auto &item) {
            switch (++no_md_entries_rows) {
              case 1:
                CHECK(map(item.mDEntryPx()).template get<double>() == -1.15625_a);
                CHECK(mdp::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue()) == 6);
                CHECK(mdp::get_int(item.numberOfOrders(), item.numberOfOrdersNullValue()) == 0);
                CHECK(mdp::get_int(item.mDPriceLevel(), item.mDPriceLevelNullValue()) == 1);
                CHECK(mdp::get_int(item.tradingReferenceDate(), item.tradingReferenceDateNullValue()) == 0);  // null
                CHECK(item.openCloseSettlFlag() == ::cme::sbe::mdp::OpenCloseSettlFlag::NULL_VALUE);
                CHECK(item.settlPriceType().nullValue());
                CHECK(item.mDEntryType() == ::cme::sbe::mdp::MDEntryType::Value::ImpliedBid);
                break;
              case 2:
                CHECK(map(item.mDEntryPx()).template get<double>() == -1.1875);
                CHECK(mdp::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue()) == 1);
                CHECK(mdp::get_int(item.numberOfOrders(), item.numberOfOrdersNullValue()) == 0);
                CHECK(mdp::get_int(item.mDPriceLevel(), item.mDPriceLevelNullValue()) == 2);
                CHECK(mdp::get_int(item.tradingReferenceDate(), item.tradingReferenceDateNullValue()) == 0);  // null
                CHECK(item.openCloseSettlFlag() == ::cme::sbe::mdp::OpenCloseSettlFlag::NULL_VALUE);
                CHECK(item.settlPriceType().nullValue());
                CHECK(item.mDEntryType() == ::cme::sbe::mdp::MDEntryType::Value::ImpliedBid);
                break;
              case 3:
                CHECK(map(item.mDEntryPx()).template get<double>() == -0.296875_a);
                CHECK(mdp::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue()) == 6);
                CHECK(mdp::get_int(item.numberOfOrders(), item.numberOfOrdersNullValue()) == 0);
                CHECK(mdp::get_int(item.mDPriceLevel(), item.mDPriceLevelNullValue()) == 1);
                CHECK(mdp::get_int(item.tradingReferenceDate(), item.tradingReferenceDateNullValue()) == 0);  // null
                CHECK(item.openCloseSettlFlag() == ::cme::sbe::mdp::OpenCloseSettlFlag::NULL_VALUE);
                CHECK(item.settlPriceType().nullValue());
                CHECK(item.mDEntryType() == ::cme::sbe::mdp::MDEntryType::Value::ImpliedOffer);
                break;
              case 4:
                CHECK(map(item.mDEntryPx()).template get<double>() == 0.890625_a);
                CHECK(mdp::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue()) == 1);
                CHECK(mdp::get_int(item.numberOfOrders(), item.numberOfOrdersNullValue()) == 0);
                CHECK(mdp::get_int(item.mDPriceLevel(), item.mDPriceLevelNullValue()) == 2);
                CHECK(mdp::get_int(item.tradingReferenceDate(), item.tradingReferenceDateNullValue()) == 0);  // null
                CHECK(item.openCloseSettlFlag() == ::cme::sbe::mdp::OpenCloseSettlFlag::NULL_VALUE);
                CHECK(item.settlPriceType().nullValue());
                CHECK(item.mDEntryType() == ::cme::sbe::mdp::MDEntryType::Value::ImpliedOffer);
                break;
              case 5:
                CHECK(map(item.mDEntryPx()).template get<double>() == 0.0_a);
                CHECK(mdp::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue()) == 0);
                CHECK(mdp::get_int(item.numberOfOrders(), item.numberOfOrdersNullValue()) == 0);
                CHECK(mdp::get_int(item.mDPriceLevel(), item.mDPriceLevelNullValue()) == 0);
                CHECK(mdp::get_int(item.tradingReferenceDate(), item.tradingReferenceDateNullValue()) == 19418);
                CHECK(item.openCloseSettlFlag() == ::cme::sbe::mdp::OpenCloseSettlFlag::NULL_VALUE);
                CHECK(item.settlPriceType().rawValue() == 3);  // FinalDaily | Actual
                CHECK(item.mDEntryType() == ::cme::sbe::mdp::MDEntryType::Value::SettlementPrice);
                break;
            }
          });
          CHECK(no_md_entries_rows == 5);
          break;
        }
        case 6: {
          CHECK(value.lastMsgSeqNumProcessed() == 66040781);
          CHECK(value.totNumReports() == 295);
          CHECK(value.securityID() == 9905);
          CHECK(value.rptSeq() == 57647);
          CHECK(value.transactTime() == 1677801259010763163);
          CHECK(value.lastUpdateTime() == 1677788155389510545);
          CHECK(value.tradeDate() == 19419);
          CHECK(value.mDSecurityTradingStatus() == ::cme::sbe::mdp::SecurityTradingStatus::ReadyToTrade);
          CHECK(std::isnan(map(value.highLimitPrice()).template get<double>()));
          CHECK(std::isnan(map(value.lowLimitPrice()).template get<double>()));
          CHECK(map(value.maxPriceVariation()).template get<double>() == 0.125_a);
          value.sbeRewind();  // wtf!
          auto no_md_entries_rows = 0;
          value.noMDEntries().forEach([&no_md_entries_rows](auto &item) {
            switch (++no_md_entries_rows) {
              case 1:
                CHECK(map(item.mDEntryPx()).template get<double>() == 0.0390625_a);
                CHECK(mdp::get_int(item.mDEntrySize(), item.mDEntrySizeNullValue()) == 0);
                CHECK(mdp::get_int(item.numberOfOrders(), item.numberOfOrdersNullValue()) == 0);
                CHECK(mdp::get_int(item.mDPriceLevel(), item.mDPriceLevelNullValue()) == 0);
                CHECK(mdp::get_int(item.tradingReferenceDate(), item.tradingReferenceDateNullValue()) == 19418);
                CHECK(item.openCloseSettlFlag() == ::cme::sbe::mdp::OpenCloseSettlFlag::NULL_VALUE);
                CHECK(item.settlPriceType().rawValue() == 3);  // FinalDaily | Actual
                CHECK(item.mDEntryType() == ::cme::sbe::mdp::MDEntryType::Value::SettlementPrice);
                break;
            }
          });
          CHECK(no_md_entries_rows == 1);
          break;
        }
      }
    }
    void operator()(Trace<::cme::sbe::mdp::SnapshotFullRefreshOrderBook53> const &, mdp::Frame const &) override { FAIL(); }
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
  CHECK(handler.counter == 6);
}
