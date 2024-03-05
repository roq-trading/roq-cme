/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/cme/mdp/config.hpp"

#include "roq/logging.hpp"

#include "roq/utils/charconv.hpp"

#include "roq/cme/mdp/config_reader.hpp"

using namespace std::literals;

namespace roq {
namespace cme {
namespace mdp {

// === HELPERS ===

namespace {
auto get_type(auto &value) {
  using enum ConnectionType;
  if (value.compare("H"sv) == 0)
    return HISTORICAL_REPLAY;
  if (value.compare("N"sv) == 0)
    return INSTRUMENT_DEFINITION;
  if (value.compare("S"sv) == 0)
    return MBP_MARKET_RECOVERY;
  if (value.compare("SMBO"sv) == 0)
    return MBOFD_MARKET_RECOVERY;
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
  explicit Handler(T &connections) : connections_{connections} {}
  void operator()(std::string_view const &channel_id, ConfigReader::Channel const &channel) override {
    for (auto &[connection_id, connection] : channel.connections) {
      try {
        auto type = get_type(connection.feed_type);
        auto priority = get_priority(connection.feed);
        auto tmp = Config::Connection{
            .multicast_address = connection.ip,
            .port = utils::from_chars<uint16_t>(connection.port),
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
auto read_connections(auto &filename, bool verbose) {
  if (verbose)
    log::info(R"(Reading "{}"...)"sv, filename);
  T result;
  Handler handler{result};
  ConfigReader::read(handler, filename);
  return result;
}

template <typename R>
R create_connection_types(auto &connections) {
  using result_type = std::remove_cvref<R>::type;
  result_type result;
  for (auto &[channel_id, tmp_1] : connections)
    for (auto &[connection_type, tmp_2] : tmp_1)
      for (auto &[priority, connection] : tmp_2) {
        auto channel_id_2 = utils::from_chars<uint16_t>(channel_id);
        result[connection.multicast_address][connection.port] = {channel_id_2, connection_type, priority};
      }
  return result;
}
}  // namespace

// === IMPLEMENTATION ===

Config::Config(std::string_view const &filename, bool verbose)
    : connections_{read_connections<decltype(connections_)>(filename, verbose)},
      connection_types_{create_connection_types<decltype(connection_types_)>(connections_)} {
}

}  // namespace mdp
}  // namespace cme
}  // namespace roq
