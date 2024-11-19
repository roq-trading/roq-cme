/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/cme/dump/controller.hpp"

#include <fmt/chrono.h>

#include <cme_mdp/MessageHeader.h>

#include "roq/logging.hpp"

#include "roq/utils/debug/hex/message.hpp"

#include "roq/utils/pcap/reader.hpp"

#include "roq/cme/mdp/parser.hpp"

using namespace std::literals;

namespace roq {
namespace cme {
namespace dump {

// === HELPERS ===

namespace {
struct Bridge final : public utils::pcap::Reader::Handler, public mdp::Parser::Handler {
  explicit Bridge(Settings const &settings) : settings_{settings} {}

 protected:
  bool operator()(
      std::chrono::nanoseconds timestamp,
      [[maybe_unused]] std::string_view const &source_address,
      [[maybe_unused]] uint16_t source_port,
      std::string_view const &destination_address,
      uint16_t destination_port,
      std::span<std::byte const> const &payload) override {
    if (settings_.print_payload) {
      utils::debug::hex::Message message{payload};
      fmt::print("payload={}\n"sv, message);
    }
    fmt::print("message={{timestamp={}, address={}, port={}"sv, timestamp, destination_address, destination_port);
    TraceInfo trace_info;
    mdp::Parser::dispatch(*this, payload, trace_info);
    fmt::print("}}\n"sv);
    return false;
  }

  void operator()(mdp::Frame const &) override {}
  // admin
  void operator()(Trace<cme_mdp::AdminHeartbeat12> const &event, mdp::Frame const &frame) override { print(event, frame); }
  void operator()(Trace<cme_mdp::ChannelReset4> const &event, mdp::Frame const &frame) override { print(event, frame); }
  // instrument definitions
  void operator()(Trace<cme_mdp::MDInstrumentDefinitionFuture54> const &event, mdp::Frame const &frame) override { print(event, frame); }
  void operator()(Trace<cme_mdp::MDInstrumentDefinitionOption55> const &event, mdp::Frame const &frame) override { print(event, frame); }
  void operator()(Trace<cme_mdp::MDInstrumentDefinitionSpread56> const &event, mdp::Frame const &frame) override { print(event, frame); }
  void operator()(Trace<cme_mdp::MDInstrumentDefinitionFixedIncome57> const &event, mdp::Frame const &frame) override { print(event, frame); }
  void operator()(Trace<cme_mdp::MDInstrumentDefinitionRepo58> const &event, mdp::Frame const &frame) override { print(event, frame); }
  void operator()(Trace<cme_mdp::MDInstrumentDefinitionFX63> const &event, mdp::Frame const &frame) override { print(event, frame); }
  // status
  void operator()(Trace<cme_mdp::SecurityStatus30> const &event, mdp::Frame const &frame) override { print(event, frame); }
  // market by price
  void operator()(Trace<cme_mdp::SnapshotFullRefresh52> const &event, mdp::Frame const &frame) override { print(event, frame); }
  void operator()(Trace<cme_mdp::SnapshotFullRefreshLongQty69> const &event, mdp::Frame const &frame) override { print(event, frame); }
  void operator()(Trace<cme_mdp::MDIncrementalRefreshBook46> const &event, mdp::Frame const &frame) override { print(event, frame); }
  void operator()(Trace<cme_mdp::MDIncrementalRefreshBookLongQty64> const &event, mdp::Frame const &frame) override { print(event, frame); }
  // market by order
  void operator()(Trace<cme_mdp::SnapshotFullRefreshOrderBook53> const &event, mdp::Frame const &frame) override { print(event, frame); }
  void operator()(Trace<cme_mdp::MDIncrementalRefreshOrderBook47> const &event, mdp::Frame const &frame) override { print(event, frame); }
  // trade summary
  void operator()(Trace<cme_mdp::MDIncrementalRefreshTradeSummary48> const &event, mdp::Frame const &frame) override { print(event, frame); }
  void operator()(Trace<cme_mdp::MDIncrementalRefreshTradeSummaryLongQty65> const &event, mdp::Frame const &frame) override { print(event, frame); }
  // statistics
  void operator()(Trace<cme_mdp::MDIncrementalRefreshDailyStatistics49> const &event, mdp::Frame const &frame) override { print(event, frame); }
  void operator()(Trace<cme_mdp::MDIncrementalRefreshSessionStatistics51> const &event, mdp::Frame const &frame) override { print(event, frame); }
  void operator()(Trace<cme_mdp::MDIncrementalRefreshSessionStatisticsLongQty67> const &event, mdp::Frame const &frame) override { print(event, frame); }
  void operator()(Trace<cme_mdp::MDIncrementalRefreshVolume37> const &event, mdp::Frame const &frame) override { print(event, frame); }
  void operator()(Trace<cme_mdp::MDIncrementalRefreshVolumeLongQty66> const &event, mdp::Frame const &frame) override { print(event, frame); }
  // misc
  void operator()(Trace<cme_mdp::MDIncrementalRefreshLimitsBanding50> const &event, mdp::Frame const &frame) override { print(event, frame); }
  void operator()(Trace<cme_mdp::QuoteRequest39> const &event, mdp::Frame const &frame) override { print(event, frame); }

  void print(auto &event, auto &frame) {
    using value_type = std::remove_cvref<decltype(event)>::type::value_type;
    auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
    fmt::print(", sequence_number={}, sending_time={}, {}={}"sv, frame.sequence_number, frame.sending_time, get_name<value_type>(), value);
  }

 private:
  Settings const &settings_;
};
}  // namespace

// === IMPLEMENTATION ===

Controller::Controller(Settings const &settings, std::string_view const &pcap_path) : settings_{settings}, pcap_path_{pcap_path} {
}

void Controller::dispatch() {
  Bridge bridge{settings_};
  utils::pcap::Reader::dispatch(bridge, pcap_path_);
}

}  // namespace dump
}  // namespace cme
}  // namespace roq
