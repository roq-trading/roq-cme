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

using namespace std::literals;

namespace roq {
namespace cme {
namespace pcap_import {

// === HELPERS ===

namespace {
using callback_t = std::function<void(struct pcap_pkthdr const *header, u_char const *packet)>;

void helper(u_char *user_data, struct pcap_pkthdr const *header, u_char const *packet) {
  auto callback = reinterpret_cast<callback_t const *>(user_data);
  (*callback)(header, packet);
}

struct PCAP final {
  explicit PCAP(std::string const &path) : handle_{pcap_open_offline(path.c_str(), error_buffer_), deleter} {
    if (handle_ == nullptr)
      throw RuntimeError{"pcap_open_offline: {}"sv, error_buffer_};
  }

  explicit PCAP(std::string_view const &path) : PCAP{std::string{path}} {}

  void dispatch(callback_t const &callback) {
    auto res =
        pcap_dispatch(handle_.get(), -1, helper, reinterpret_cast<u_char *>(&const_cast<callback_t &>(callback)));
    if (res < 0)
      throw RuntimeError{"pcap_dispatch: {}"sv, pcap_geterr(handle_.get())};
  }

 private:
  using value_type = pcap_t;

  char error_buffer_[PCAP_ERRBUF_SIZE] = {};
  std::unique_ptr<value_type, void (*)(value_type *)> handle_;

  static void deleter(value_type *handle) {
    if (handle)
      pcap_close(handle);
  }
};

auto convert(timeval ts) {
  return std::chrono::nanoseconds{std::chrono::seconds{ts.tv_sec} + std::chrono::microseconds{ts.tv_usec}};
}
}  // namespace

// === IMPLEMENTATION ===

Controller::Controller(Settings const &settings) : settings_{settings} {
}

void Controller::dispatch(std::string_view const &path) {
  PCAP pcap{path};
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
        auto src_port = ntohs((*udp_header).source);
        auto dst_port = ntohs((*udp_header).dest);
        auto offset = sizeof(struct ether_header) + sizeof(struct ip) + sizeof(struct udphdr);
        std::span payload{reinterpret_cast<std::byte const *>(packet + offset), (*header).len - offset};
        log::info("{} {} {}"sv, timestamp, dst, dst_port);
        log::info<5>("{}"sv, utils::debug::hex::Message{payload});
        TraceInfo trace_info;
        mdp::Parser::dispatch(*this, payload, trace_info);
      }
    }
  };
  pcap.dispatch(callback);
}

// mdp::Parser::Handler

void Controller::operator()(mdp::Frame const &) {
  // log::info("frame={}"sv, frame);
}

// - admin

void Controller::operator()(Trace<cme_mdp::AdminHeartbeat12> const &event, mdp::Frame const &frame) {
  dispatch(event, frame);
}

void Controller::operator()(Trace<cme_mdp::ChannelReset4> const &event, mdp::Frame const &frame) {
  dispatch(event, frame);
}

// - instrument definitions

void Controller::operator()(Trace<cme_mdp::MDInstrumentDefinitionFuture54> const &event, mdp::Frame const &frame) {
  dispatch(event, frame);
}

void Controller::operator()(Trace<cme_mdp::MDInstrumentDefinitionOption55> const &event, mdp::Frame const &frame) {
  dispatch(event, frame);
}

void Controller::operator()(Trace<cme_mdp::MDInstrumentDefinitionSpread56> const &event, mdp::Frame const &frame) {
  dispatch(event, frame);
}

void Controller::operator()(Trace<cme_mdp::MDInstrumentDefinitionFixedIncome57> const &event, mdp::Frame const &frame) {
  dispatch(event, frame);
}

void Controller::operator()(Trace<cme_mdp::MDInstrumentDefinitionRepo58> const &event, mdp::Frame const &frame) {
  dispatch(event, frame);
}

void Controller::operator()(Trace<cme_mdp::MDInstrumentDefinitionFX63> const &event, mdp::Frame const &frame) {
  dispatch(event, frame);
}

// - status

void Controller::operator()(Trace<cme_mdp::SecurityStatus30> const &event, mdp::Frame const &frame) {
  dispatch(event, frame);
}

// - market by price

void Controller::operator()(Trace<cme_mdp::SnapshotFullRefresh52> const &event, mdp::Frame const &frame) {
  dispatch(event, frame);
}

void Controller::operator()(Trace<cme_mdp::SnapshotFullRefreshLongQty69> const &event, mdp::Frame const &frame) {
  dispatch(event, frame);
}

void Controller::operator()(Trace<cme_mdp::MDIncrementalRefreshBook46> const &event, mdp::Frame const &frame) {
  dispatch(event, frame);
}

void Controller::operator()(Trace<cme_mdp::MDIncrementalRefreshBookLongQty64> const &event, mdp::Frame const &frame) {
  dispatch(event, frame);
}

// - market by order

void Controller::operator()(Trace<cme_mdp::SnapshotFullRefreshOrderBook53> const &event, mdp::Frame const &frame) {
  dispatch(event, frame);
}

void Controller::operator()(Trace<cme_mdp::MDIncrementalRefreshOrderBook47> const &event, mdp::Frame const &frame) {
  dispatch(event, frame);
}

// - trade summary

void Controller::operator()(Trace<cme_mdp::MDIncrementalRefreshTradeSummary48> const &event, mdp::Frame const &frame) {
  dispatch(event, frame);
}

void Controller::operator()(
    Trace<cme_mdp::MDIncrementalRefreshTradeSummaryLongQty65> const &event, mdp::Frame const &frame) {
  dispatch(event, frame);
}

// - statistics

void Controller::operator()(
    Trace<cme_mdp::MDIncrementalRefreshDailyStatistics49> const &event, mdp::Frame const &frame) {
  dispatch(event, frame);
}

void Controller::operator()(
    Trace<cme_mdp::MDIncrementalRefreshSessionStatistics51> const &event, mdp::Frame const &frame) {
  dispatch(event, frame);
}

void Controller::operator()(
    Trace<cme_mdp::MDIncrementalRefreshSessionStatisticsLongQty67> const &event, mdp::Frame const &frame) {
  dispatch(event, frame);
}

void Controller::operator()(Trace<cme_mdp::MDIncrementalRefreshVolume37> const &event, mdp::Frame const &frame) {
  dispatch(event, frame);
}

void Controller::operator()(Trace<cme_mdp::MDIncrementalRefreshVolumeLongQty66> const &event, mdp::Frame const &frame) {
  dispatch(event, frame);
}

// - misc

void Controller::operator()(Trace<cme_mdp::MDIncrementalRefreshLimitsBanding50> const &event, mdp::Frame const &frame) {
  dispatch(event, frame);
}

void Controller::operator()(Trace<cme_mdp::QuoteRequest39> const &event, mdp::Frame const &frame) {
  dispatch(event, frame);
}

// helpers

template <typename T>
void Controller::dispatch(Trace<T> const &event, mdp::Frame const &) {
  auto &value = const_cast<T &>(event.value);  // note! not const-safe
  log::info("value={}"sv, value);
}

}  // namespace pcap_import
}  // namespace cme
}  // namespace roq
