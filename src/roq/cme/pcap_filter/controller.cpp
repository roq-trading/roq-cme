/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/cme/pcap_filter/controller.hpp"

#include <vector>

#include "roq/logging.hpp"

#include "roq/cme/mdp/config.hpp"

using namespace std::literals;

namespace roq {
namespace cme {
namespace pcap_filter {

// === HELPERS ===

namespace {
auto find(auto &result, auto &config, auto &channel_id, auto type, auto priority) {
  if (config.find(channel_id, type, priority, [&](auto &connection) {
        result.emplace_back(connection.multicast_address, connection.port);
      })) {
  } else {
    log::fatal(
        R"(Unable to find multicast configuration using channel_id="{}", type={}, priority={})"sv,
        channel_id,
        magic_enum::enum_name(type),
        priority);
  }
}
}  // namespace

// === IMPLEMENTATION ===

Controller::Controller(Settings const &settings) : settings_{settings} {
  if (settings_.type != "tcpdump"sv)
    log::fatal(R"(Unexpected: type="{}")"sv, settings_.type);
}

void Controller::dispatch() {
  mdp::Config config{settings_.config_file, false};
  std::vector<std::pair<std::string, uint16_t>> filter;
  for (auto &channel_id : settings_.channel_ids) {
    find(filter, config, channel_id, mdp::ConnectionType::INSTRUMENT_REPLAY, Priority::PRIMARY);
    find(filter, config, channel_id, mdp::ConnectionType::INSTRUMENT_REPLAY, Priority::SECONDARY);
    find(filter, config, channel_id, mdp::ConnectionType::INCREMENTAL, Priority::PRIMARY);
    find(filter, config, channel_id, mdp::ConnectionType::INCREMENTAL, Priority::SECONDARY);
    find(filter, config, channel_id, mdp::ConnectionType::SNAPSHOT, Priority::PRIMARY);
    find(filter, config, channel_id, mdp::ConnectionType::SNAPSHOT, Priority::SECONDARY);
    find(filter, config, channel_id, mdp::ConnectionType::SNAPSHOT_MBO, Priority::PRIMARY);
    find(filter, config, channel_id, mdp::ConnectionType::SNAPSHOT_MBO, Priority::SECONDARY);
  }
  auto n = std::size(filter);
  for (size_t i = 0; i < n; ++i) {
    auto &[host, port] = filter[i];
    fmt::print(stdout, "(host {} and port {})"sv, host, port);
    if (i != (n - 1))
      fmt::print(" or "sv);
  }
}

}  // namespace pcap_filter
}  // namespace cme
}  // namespace roq
