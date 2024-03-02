/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/cme/pcap_import/controller.hpp"

#include <arpa/inet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <netinet/if_ether.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>

#include <pcap/pcap.h>

#include "roq/exceptions.hpp"
#include "roq/logging.hpp"

#include "roq/utils/debug/hex/message.hpp"

#include "roq/cme/pcap_import/pcap.hpp"

using namespace std::literals;

namespace roq {
namespace cme {
namespace pcap_import {

// === HELPERS ===

namespace {
auto convert(timeval ts) {
  return std::chrono::nanoseconds{std::chrono::seconds{ts.tv_sec} + std::chrono::microseconds{ts.tv_usec}};
}
}  // namespace

// === IMPLEMENTATION ===

Controller::Controller(Settings const &settings)
    : settings_{settings}, config_{settings.config_file, false}, market_data_{*this} {
}

void Controller::dispatch(std::string_view const &path) {
  auto callback = [this](struct pcap_pkthdr const *header, u_char const *packet) {
    auto timestamp = convert((*header).ts);
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
        auto udp_header =
            reinterpret_cast<struct udphdr const *>(packet + sizeof(struct ether_header) + sizeof(struct ip));
#if __APPLE__
        auto src_port = ntohs((*udp_header).uh_sport);
        auto dst_port = ntohs((*udp_header).uh_dport);
#else
        auto src_port = ntohs((*udp_header).source);
        auto dst_port = ntohs((*udp_header).dest);
#endif
        auto offset = sizeof(struct ether_header) + sizeof(struct ip) + sizeof(struct udphdr);
        std::span payload{reinterpret_cast<std::byte const *>(packet + offset), (*header).len - offset};
        log::info<5>(
            "timestamp={}, address={}, port={}, payload={}"sv,
            timestamp,
            dst,
            dst_port,
            utils::debug::hex::Message{payload});
        if (config_.find(dst, dst_port, [&](auto connection_type) {
              TraceInfo trace_info{timestamp, timestamp, timestamp};
              market_data_.dispatch(connection_type, payload, trace_info, 0);
            })) {
        } else {
          log::warn(R"(Unexpected: address="{}", port={})"sv, dst, dst_port);
        }
      }
    }
  };
  PCAP{path}.dispatch(callback);
}

// market_data::Manager::Handler

void Controller::operator()(Trace<StreamStatus> const &event) {
  log::info("event={}"sv, event);
}

void Controller::operator()(Trace<ExternalLatency> const &event) {
  log::info("event={}"sv, event);
}

void Controller::operator()(Trace<ReferenceData> const &event, bool is_last) {
  log::info("event={}"sv, event);
}

void Controller::operator()(Trace<MarketStatus> const &event, bool is_last) {
  log::info("event={}"sv, event);
}

void Controller::operator()(Trace<TopOfBook> const &event, bool is_last) {
  log::info("event={}"sv, event);
}

void Controller::operator()(Trace<MarketByPriceUpdate> const &event, bool is_last) {
  log::info("event={}"sv, event);
}

void Controller::operator()(Trace<MarketByOrderUpdate> const &event, bool is_last) {
  log::info("event={}"sv, event);
}

void Controller::operator()(Trace<TradeSummary> const &event, bool is_last) {
  log::info("event={}"sv, event);
}

void Controller::operator()(Trace<StatisticsUpdate> const &event, bool is_last) {
  log::info("event={}"sv, event);
}

}  // namespace pcap_import
}  // namespace cme
}  // namespace roq
