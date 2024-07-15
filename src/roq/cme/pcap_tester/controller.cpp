/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/cme/pcap_tester/controller.hpp"

#include <arpa/inet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <netinet/if_ether.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>

#include <fmt/chrono.h>

// #include "roq/exceptions.hpp"
#include "roq/logging.hpp"

// #include "roq/utils/compare.hpp"
// #include "roq/utils/datetime.hpp"

#include "roq/utils/debug/hex/message.hpp"

#include "roq/cme/pcap_tester/pcap.hpp"

using namespace std::literals;

namespace roq {
namespace cme {
namespace pcap_tester {

// === HELPERS ===

namespace {
auto convert(timeval ts) {
  return std::chrono::nanoseconds{std::chrono::seconds{ts.tv_sec} + std::chrono::microseconds{ts.tv_usec}};
}
}  // namespace

// === IMPLEMENTATION ===

Controller::Controller(Settings const &settings, std::string_view const &pcap_path) : settings_{settings}, pcap_path_{pcap_path} {
}

void Controller::dispatch() {
  auto callback = [&](struct pcap_pkthdr const *header, u_char const *packet) -> bool {
    auto timestamp = convert((*header).ts);
    if (timestamp == 1707048276964937000ns)
      log::warn("XXX"sv);
    // note! assuming ether
    auto ether_header = reinterpret_cast<struct ether_header const *>(packet);
    auto ether_type = ntohs((*ether_header).ether_type);
    if (ether_type == ETHERTYPE_IP) {
      auto ip_header = reinterpret_cast<struct ip const *>(packet + sizeof(struct ether_header));
      char src[INET_ADDRSTRLEN];
      inet_ntop(AF_INET, &((*ip_header).ip_src), src, INET_ADDRSTRLEN);
      char dst[INET_ADDRSTRLEN];
      inet_ntop(AF_INET, &((*ip_header).ip_dst), dst, INET_ADDRSTRLEN);
      if ((*ip_header).ip_p == IPPROTO_UDP) {
        auto udp_header = reinterpret_cast<struct udphdr const *>(packet + sizeof(struct ether_header) + sizeof(struct ip));
#if __APPLE__
        // auto src_port = ntohs((*udp_header).uh_sport);
        auto dst_port = ntohs((*udp_header).uh_dport);
#else
        // auto src_port = ntohs((*udp_header).source);
        auto dst_port = ntohs((*udp_header).dest);
#endif
        auto offset = sizeof(struct ether_header) + sizeof(struct ip) + sizeof(struct udphdr);
        std::span payload{reinterpret_cast<std::byte const *>(packet + offset), (*header).len - offset};
        log::info<0>("timestamp={}, address={}, port={}, payload={}"sv, timestamp, dst, dst_port, utils::debug::hex::Message{payload});
        TraceInfo trace_info;
        mdp::Parser::dispatch(*this, payload, trace_info);
      }
    }
    return false;
  };
  // market_data_.start();
  PCAP{pcap_path_}.dispatch(callback);
}

void Controller::operator()(mdp::Frame const &frame) {
  log::info<0>("frame={}"sv, frame);
}

void Controller::operator()(Trace<cme_mdp::AdminHeartbeat12> const &, mdp::Frame const &) {
}

void Controller::operator()(Trace<cme_mdp::ChannelReset4> const &, mdp::Frame const &) {
}

void Controller::operator()(Trace<cme_mdp::MDInstrumentDefinitionFuture54> const &event, mdp::Frame const &frame) {
  // log::info<0>("frame={}, md_instrument_definition_future_54={}"sv, frame, event.value);
  log::info<0>("HERE"sv);
}

void Controller::operator()(Trace<cme_mdp::MDInstrumentDefinitionOption55> const &, mdp::Frame const &) {
}

void Controller::operator()(Trace<cme_mdp::MDInstrumentDefinitionSpread56> const &event, mdp::Frame const &frame) {
  // log::info<0>("frame={}, md_instrument_definition_spread_56={}"sv, frame, event.value);
  log::info<0>("HERE"sv);
}

void Controller::operator()(Trace<cme_mdp::MDInstrumentDefinitionFixedIncome57> const &, mdp::Frame const &) {
}

void Controller::operator()(Trace<cme_mdp::MDInstrumentDefinitionRepo58> const &, mdp::Frame const &) {
}

void Controller::operator()(Trace<cme_mdp::MDInstrumentDefinitionFX63> const &, mdp::Frame const &) {
}

void Controller::operator()(Trace<cme_mdp::SecurityStatus30> const &, mdp::Frame const &) {
}

void Controller::operator()(Trace<cme_mdp::SnapshotFullRefresh52> const &, mdp::Frame const &) {
}

void Controller::operator()(Trace<cme_mdp::SnapshotFullRefreshLongQty69> const &, mdp::Frame const &) {
}

void Controller::operator()(Trace<cme_mdp::MDIncrementalRefreshBook46> const &, mdp::Frame const &) {
}

void Controller::operator()(Trace<cme_mdp::MDIncrementalRefreshBookLongQty64> const &, mdp::Frame const &) {
}

void Controller::operator()(Trace<cme_mdp::SnapshotFullRefreshOrderBook53> const &, mdp::Frame const &) {
}

void Controller::operator()(Trace<cme_mdp::MDIncrementalRefreshOrderBook47> const &, mdp::Frame const &) {
}

void Controller::operator()(Trace<cme_mdp::MDIncrementalRefreshTradeSummary48> const &, mdp::Frame const &) {
}

void Controller::operator()(Trace<cme_mdp::MDIncrementalRefreshTradeSummaryLongQty65> const &, mdp::Frame const &) {
}

void Controller::operator()(Trace<cme_mdp::MDIncrementalRefreshDailyStatistics49> const &, mdp::Frame const &) {
}

void Controller::operator()(Trace<cme_mdp::MDIncrementalRefreshSessionStatistics51> const &, mdp::Frame const &) {
}

void Controller::operator()(Trace<cme_mdp::MDIncrementalRefreshSessionStatisticsLongQty67> const &, mdp::Frame const &) {
}

void Controller::operator()(Trace<cme_mdp::MDIncrementalRefreshVolume37> const &, mdp::Frame const &) {
}

void Controller::operator()(Trace<cme_mdp::MDIncrementalRefreshVolumeLongQty66> const &, mdp::Frame const &) {
}

void Controller::operator()(Trace<cme_mdp::MDIncrementalRefreshLimitsBanding50> const &, mdp::Frame const &) {
}

void Controller::operator()(Trace<cme_mdp::QuoteRequest39> const &, mdp::Frame const &) {
}

}  // namespace pcap_tester
}  // namespace cme
}  // namespace roq
