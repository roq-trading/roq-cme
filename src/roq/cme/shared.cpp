/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/cme/shared.hpp"

#include "roq/logging.hpp"

#include "roq/utils/safe_cast.hpp"

using namespace std::literals;

namespace roq {
namespace cme {

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
    struct Handler final : public ilink::ConfigReader::Handler {
      explicit Handler(R &result) : result_{result} {}

     protected:
      void operator()(uint8_t market_segment_id, ilink::ConfigReader::MarketSegment const &market_segment) override {
        result_.try_emplace(market_segment_id, market_segment);
      }

     private:
      R &result_;
    } handler{result};
    ilink::ConfigReader::read(handler, filename);
  }
  return result;
}

}  // namespace

// === IMPLEMENTATION ===

Shared::Shared(
    server::Dispatcher &dispatcher, Settings const &settings, market_data::SecurityDefinitions &security_definitions)
    : dispatcher_{dispatcher}, settings{settings}, security_definitions{security_definitions}, buffer(BUFFER_SIZE),
      mdp_config{settings.multicast.config_file, true},
      ilink_config_{read_ilink_config<decltype(ilink_config_)>(settings.ilink.config_file)} {
}

std::pair<std::string, uint16_t> Shared::get_multicast_config(
    uint16_t channel_id, mdp::ConnectionType type, Priority priority) const {
  std::pair<std::string, uint16_t> result;
  if (mdp_config.find(channel_id, type, priority, [&](auto &connection) {
        result = {connection.multicast_address, connection.port};
      })) {
  } else {
    throw RuntimeError{
        R"(Unable to find multicast configuration using channel_id="{}", type={}, priority={})"sv,
        channel_id,
        magic_enum::enum_name(type),
        priority};
  }
  return result;
}

}  // namespace cme
}  // namespace roq
