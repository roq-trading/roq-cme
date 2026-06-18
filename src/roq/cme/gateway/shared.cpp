/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/cme/gateway/shared.hpp"

#include <fmt/format.h>

#include <magic_enum/magic_enum_format.hpp>

#include "roq/logging.hpp"

#include "roq/utils/safe_cast.hpp"

using namespace std::literals;

namespace roq {
namespace cme {
namespace gateway {

// === CONSTANTS ===

namespace {
auto const BUFFER_SIZE = 4096uz;
}

// === HELPERS ===

namespace {
template <typename R>
R read_ilink_config(auto const &filename) {
  R result;
  if (!std::empty(filename)) {
    struct Handler final : public protocol::ilink::ConfigReader::Handler {
      explicit Handler(R &result) : result_{result} {}

     protected:
      void operator()(uint8_t market_segment_id, protocol::ilink::ConfigReader::MarketSegment const &market_segment) override {
        result_.try_emplace(market_segment_id, market_segment);
      }

     private:
      R &result_;
    } handler{result};
    protocol::ilink::ConfigReader::read(handler, filename);
  }
  return result;
}

}  // namespace

// === IMPLEMENTATION ===

Shared::Shared(server::Dispatcher &dispatcher, Settings const &settings, market_data::SecurityDefinitions &security_definitions)
    : dispatcher{dispatcher}, settings{settings}, security_definitions{security_definitions}, buffer(BUFFER_SIZE),
      mdp_config{settings.multicast.config_file, true}, ilink_config_{read_ilink_config<decltype(ilink_config_)>(settings.ilink.config_file)} {
}

std::pair<std::string, uint16_t> Shared::get_multicast_config(uint16_t channel_id, protocol::mdp::ConnectionType type, Priority priority) const {
  std::pair<std::string, uint16_t> result;
  if (mdp_config.find(channel_id, type, priority, [&](auto &connection) { result = {connection.multicast_address, connection.port}; })) {
  } else {
    throw RuntimeError{R"(Unable to find multicast configuration using channel_id="{}", type={}, priority={})"sv, channel_id, type, priority};
  }
  return result;
}

}  // namespace gateway
}  // namespace cme
}  // namespace roq
