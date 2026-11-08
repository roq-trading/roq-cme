/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/cme/protocol/mdp/parser.hpp"

namespace roq {
namespace cme {

template <typename T>
struct MDPParserTester final : public protocol::mdp::Parser::Handler {
  using value_type = std::remove_cvref_t<T>;
  using callback_type = std::function<void(value_type const &)>;

  static void dispatch(callback_type const &callback, std::string_view const &message) {
    std::span raw{reinterpret_cast<std::byte const *>(std::data(message)), std::size(message)};
    // XXX FIXME TODO catch2 block ???
    MDPParserTester handler{callback};
    TraceInfo trace_info;
    auto res = protocol::mdp::Parser::dispatch(handler, raw, trace_info);
    CHECK(res == true);
    CHECK(handler.found_ == true);
  }

 protected:
  explicit MDPParserTester(callback_type const &callback) : callback_{callback} {}

  void operator()(protocol::mdp::Frame const &) override {}  // note!
  void operator()(Trace<::cme::sbe::mdp::ChannelReset4> const &event, protocol::mdp::Frame const &) override { dispatch_helper(event); }
  void operator()(Trace<::cme::sbe::mdp::AdminHeartbeat12> const &event, protocol::mdp::Frame const &) override { dispatch_helper(event); }
  // - security status
  void operator()(Trace<::cme::sbe::mdp::SecurityStatus30> const &event, protocol::mdp::Frame const &) override { dispatch_helper(event); }
  // - instrument definition
  void operator()(Trace<::cme::sbe::mdp::MDInstrumentDefinitionFuture54> const &event, protocol::mdp::Frame const &) override { dispatch_helper(event); }
  void operator()(Trace<::cme::sbe::mdp::MDInstrumentDefinitionOption55> const &event, protocol::mdp::Frame const &) override { dispatch_helper(event); }
  void operator()(Trace<::cme::sbe::mdp::MDInstrumentDefinitionSpread56> const &event, protocol::mdp::Frame const &) override { dispatch_helper(event); }
  void operator()(Trace<::cme::sbe::mdp::MDInstrumentDefinitionFixedIncome57> const &event, protocol::mdp::Frame const &) override { dispatch_helper(event); }
  void operator()(Trace<::cme::sbe::mdp::MDInstrumentDefinitionRepo58> const &event, protocol::mdp::Frame const &) override { dispatch_helper(event); }
  void operator()(Trace<::cme::sbe::mdp::MDInstrumentDefinitionFX63> const &event, protocol::mdp::Frame const &) override { dispatch_helper(event); }
  // - SnapshotFullRefresh
  void operator()(Trace<::cme::sbe::mdp::SnapshotFullRefresh52> const &event, protocol::mdp::Frame const &) override { dispatch_helper(event); }
  void operator()(Trace<::cme::sbe::mdp::SnapshotFullRefreshOrderBook53> const &event, protocol::mdp::Frame const &) override { dispatch_helper(event); }
  void operator()(Trace<::cme::sbe::mdp::SnapshotFullRefreshLongQty69> const &event, protocol::mdp::Frame const &) override { dispatch_helper(event); }
  // - MDIncrementalRefresh
  void operator()(Trace<::cme::sbe::mdp::MDIncrementalRefreshVolume37> const &event, protocol::mdp::Frame const &) override { dispatch_helper(event); }
  void operator()(Trace<::cme::sbe::mdp::MDIncrementalRefreshBook46> const &event, protocol::mdp::Frame const &) override { dispatch_helper(event); }
  void operator()(Trace<::cme::sbe::mdp::MDIncrementalRefreshOrderBook47> const &event, protocol::mdp::Frame const &) override { dispatch_helper(event); }
  void operator()(Trace<::cme::sbe::mdp::MDIncrementalRefreshTradeSummary48> const &event, protocol::mdp::Frame const &) override { dispatch_helper(event); }
  void operator()(Trace<::cme::sbe::mdp::MDIncrementalRefreshDailyStatistics49> const &event, protocol::mdp::Frame const &) override { dispatch_helper(event); }
  void operator()(Trace<::cme::sbe::mdp::MDIncrementalRefreshLimitsBanding50> const &event, protocol::mdp::Frame const &) override { dispatch_helper(event); }
  void operator()(Trace<::cme::sbe::mdp::MDIncrementalRefreshSessionStatistics51> const &event, protocol::mdp::Frame const &) override {
    dispatch_helper(event);
  }
  void operator()(Trace<::cme::sbe::mdp::MDIncrementalRefreshBookLongQty64> const &event, protocol::mdp::Frame const &) override { dispatch_helper(event); }
  void operator()(Trace<::cme::sbe::mdp::MDIncrementalRefreshTradeSummaryLongQty65> const &event, protocol::mdp::Frame const &) override {
    dispatch_helper(event);
  }
  void operator()(Trace<::cme::sbe::mdp::MDIncrementalRefreshVolumeLongQty66> const &event, protocol::mdp::Frame const &) override { dispatch_helper(event); }
  void operator()(Trace<::cme::sbe::mdp::MDIncrementalRefreshSessionStatisticsLongQty67> const &event, protocol::mdp::Frame const &) override {
    dispatch_helper(event);
  }
  void operator()(Trace<::cme::sbe::mdp::QuoteRequest39> const &event, protocol::mdp::Frame const &) override { dispatch_helper(event); }

  template <typename U>
  void dispatch_helper(Trace<U> const &event) {
    if constexpr (std::is_invocable_v<callback_type, U>) {
      found_ = true;
      callback_(event);
    } else {
      FAIL();
    }
  }

 private:
  callback_type const callback_;
  bool found_ = false;
};

}  // namespace cme
}  // namespace roq
