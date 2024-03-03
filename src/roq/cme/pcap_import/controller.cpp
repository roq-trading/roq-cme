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
auto create_market_data(auto &handler, auto &settings) {
  auto config = market_data::Manager::Config{
      .cache_all_reference_data = settings.cache_all_reference_data,
  };
  return market_data::Manager{handler, config};
}

template <typename R>
R create_symbols_regex(auto &symbols) {
  using result_type = std::remove_cvref<R>::type;
  result_type result;
  for (auto &item : symbols)
    result.emplace_back(item);
  return result;
}

auto convert(timeval ts) {
  return std::chrono::nanoseconds{std::chrono::seconds{ts.tv_sec} + std::chrono::microseconds{ts.tv_usec}};
}
}  // namespace

// === IMPLEMENTATION ===

Controller::Controller(Settings const &settings)
    : settings_{settings}, config_{settings.config_file, false}, market_data_{create_market_data(*this, settings)},
      symbols_regex_{create_symbols_regex<decltype(symbols_regex_)>(settings.symbols)} {
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

void Controller::operator()(Trace<ReferenceData> const &event, [[maybe_unused]] bool is_last) {
  log::info("event={}"sv, event);
}

void Controller::operator()(Trace<MarketStatus> const &event, [[maybe_unused]] bool is_last) {
  log::info("event={}"sv, event);
}

void Controller::operator()(Trace<TopOfBook> const &event, [[maybe_unused]] bool is_last) {
  log::info("event={}"sv, event);
}

void Controller::operator()(Trace<MarketByPriceUpdate> const &event, [[maybe_unused]] bool is_last) {
  log::info("event={}"sv, event);
}

void Controller::operator()(Trace<MarketByOrderUpdate> const &event, [[maybe_unused]] bool is_last) {
  log::info("event={}"sv, event);
}

void Controller::operator()(Trace<TradeSummary> const &event, [[maybe_unused]] bool is_last) {
  log::info("event={}"sv, event);
}

void Controller::operator()(Trace<StatisticsUpdate> const &event, [[maybe_unused]] bool is_last) {
  log::info("event={}"sv, event);
}

bool Controller::discard_symbol(std::string_view const &symbol) {
  auto iter = discard_symbol_.find(symbol);
  if (iter != std::end(discard_symbol_))
    return (*iter).second;
  bool discard = !std::empty(symbols_regex_);
  for (auto &regex : symbols_regex_) {  // note! O(n)
    if (regex.match(symbol)) {
      discard = false;
      break;
    }
  }
  if (discard)
    log::info<1>(R"(Discard symbol="{}" (reason: no regex match))"sv, symbol);
  discard_symbol_.emplace(symbol, discard);
  return discard;
}

}  // namespace pcap_import
}  // namespace cme
}  // namespace roq
