/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/cme/mdp/config.hpp"

#include "roq/logging.hpp"

#include "roq/core/charconv.hpp"

#include "roq/cme/mdp/config_reader.hpp"

using namespace std::literals;

namespace roq {
namespace cme {
namespace mdp {

namespace {
auto get_type(auto &value) {
  using enum ConnectionType;
  if (value.compare("H"sv) == 0)
    return HISTORICAL_REPLAY;
  if (value.compare("N"sv) == 0)
    return INSTRUMENT_REPLAY;
  if (value.compare("S"sv) == 0)
    return SNAPSHOT;
  if (value.compare("SMBO"sv) == 0)
    return SNAPSHOT_MBO;
  if (value.compare("I"sv) == 0)
    return INCREMENTAL;
  throw RuntimeError{R"(Unexpected: feed-type="{}")"sv, value};
}

auto get_priority(auto &value) {
  using enum Priority;
  if (value.compare("A"sv) == 0)
    return PRIMARY;
  if (value.compare("B"sv) == 0)
    return SECONDARY;
  throw RuntimeError{R"(Unexpected: feed="{}")"sv, value};
}

template <typename T>
struct Handler final : public ConfigReader::Handler {
  explicit Handler(T &connections) : connections_(connections) {}
  void operator()(std::string_view const &channel_id, ConfigReader::Channel const &channel) override {
    for (auto &[connection_id, connection] : channel.connections) {
      try {
        auto type = get_type(connection.feed_type);
        auto priority = get_priority(connection.feed);
        Config::Connection tmp{
            .multicast_address = connection.ip,
            .port = core::from_chars<uint16_t>(connection.port),
        };
        connections_[channel_id][type].try_emplace(priority, std::move(tmp));
      } catch (RuntimeError &e) {
        log::warn("Exception: {}"sv, e);
      }
    }
  }

 private:
  T &connections_;
};

template <typename T>
auto read_connections(auto &filename) {
  T result;
  Handler handler{result};
  ConfigReader::read(handler, filename);
  return result;
}
}  // namespace

Config::Config(std::string_view const &filename) : connections_(read_connections<decltype(connections_)>(filename)) {
}

}  // namespace mdp
}  // namespace cme
}  // namespace roq
