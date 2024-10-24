/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/cme/dump/controller.hpp"

#include <arpa/inet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <netinet/if_ether.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>

#include <fmt/chrono.h>

#include <cme_mdp/MessageHeader.h>

#include "roq/logging.hpp"

#include "roq/cme/mdp/parser.hpp"

#include "roq/cme/dump/pcap.hpp"

using namespace std::literals;

namespace roq {
namespace cme {
namespace dump {

// === HELPERS ===

namespace {
auto convert(timeval ts) {
  return std::chrono::nanoseconds{std::chrono::seconds{ts.tv_sec} + std::chrono::microseconds{ts.tv_usec}};
}

struct Bridge final : public mdp::Parser::Handler {
  Bridge(struct pcap_pkthdr const *header, u_char const *packet) : header_{header}, packet_{packet} {}

 protected:
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
    auto timestamp = convert((*header_).ts);
    size_t offset = 0;
    auto ether_header = reinterpret_cast<struct ether_header const *>(packet_ + offset);
    auto ether_type = ntohs((*ether_header).ether_type);
    // XXX FIXME there is also VLAG double-tagging... how to identify?
    if (ether_type == ETHERTYPE_VLAN) {
      offset += 4;  // XXX FIXME find somee struct or length in system header files... (VLAN tag)
      ether_header = reinterpret_cast<struct ether_header const *>(packet_ + offset);
      ether_type = ntohs((*ether_header).ether_type);
    }
    if (ether_type == ETHERTYPE_IP) {
      offset += sizeof(struct ether_header);
      auto ip_header = reinterpret_cast<struct ip const *>(packet_ + offset);
      char src[INET_ADDRSTRLEN];
      inet_ntop(AF_INET, &((*ip_header).ip_src), src, INET_ADDRSTRLEN);
      char dst[INET_ADDRSTRLEN];
      inet_ntop(AF_INET, &((*ip_header).ip_dst), dst, INET_ADDRSTRLEN);
      if ((*ip_header).ip_p == IPPROTO_UDP) {
        offset += sizeof(struct ip);
        auto udp_header = reinterpret_cast<struct udphdr const *>(packet_ + offset);
#if __APPLE__
        // auto src_port = ntohs((*udp_header).uh_sport);
        auto dst_port = ntohs((*udp_header).uh_dport);
#else
        // auto src_port = ntohs((*udp_header).source);
        auto dst_port = ntohs((*udp_header).dest);
#endif
        using value_type = std::remove_cvref<decltype(event)>::type::value_type;
        auto &value = const_cast<value_type &>(event.value);  // note! not const-safe
        log::info(
            "timestamp={}, address={}, port={}, sequence_number={}, sending_time={}, {}={}"sv,
            timestamp,
            dst,
            dst_port,
            frame.sequence_number,
            frame.sending_time,
            get_name<value_type>(),
            value);
      }
    }
  }

 private:
  struct pcap_pkthdr const *header_;
  u_char const *packet_;
};
}  // namespace

// === IMPLEMENTATION ===

Controller::Controller(Settings const &settings, std::string_view const &pcap_path) : settings_{settings}, pcap_path_{pcap_path} {
}

void Controller::dispatch() {
  auto callback = [&](struct pcap_pkthdr const *header, u_char const *packet) -> bool {
    auto timestamp = convert((*header).ts);
    size_t offset = 0;
    auto ether_header = reinterpret_cast<struct ether_header const *>(packet + offset);
    auto ether_type = ntohs((*ether_header).ether_type);
    // XXX FIXME there is also VLAG double-tagging... how to identify?
    if (ether_type == ETHERTYPE_VLAN) {
      offset += 4;  // XXX FIXME find somee struct or length in system header files... (VLAN tag)
      ether_header = reinterpret_cast<struct ether_header const *>(packet + offset);
      ether_type = ntohs((*ether_header).ether_type);
    }
    if (ether_type == ETHERTYPE_IP) {
      offset += sizeof(struct ether_header);
      auto ip_header = reinterpret_cast<struct ip const *>(packet + offset);
      if ((*ip_header).ip_p == IPPROTO_UDP) {
        offset += sizeof(struct ip) + sizeof(struct udphdr);
        std::span payload{reinterpret_cast<std::byte const *>(packet + offset), (*header).len - offset};
        Bridge bridge{header, packet};
        TraceInfo trace_info;
        mdp::Parser::dispatch(bridge, payload, trace_info);
      }
    } else {
      log::fatal("Unexpected: ether_type=0x{:x}"sv, ether_type);
    }
    return false;
  };
  // market_data_.start();
  PCAP{pcap_path_}.dispatch(callback);
}

}  // namespace dump
}  // namespace cme
}  // namespace roq
