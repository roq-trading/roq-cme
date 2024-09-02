/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/cme/mdp/config.hpp"

#include "roq/logging.hpp"

#include "roq/utils/charconv/from_chars.hpp"

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
  void operator()(uint16_t channel_id, ConfigReader::Channel const &channel) override {
    for (auto &[connection_id, connection] : channel.connections) {
      try {
        auto type = get_type(connection.feed_type);
        auto priority = get_priority(connection.feed);
        auto tmp = Config::Connection{
            .multicast_address = connection.ip,
            .port = utils::charconv::from_chars<uint16_t>(connection.port),
            .name = connection_id,
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
      for (auto &[priority, connection] : tmp_2)
        result[connection.multicast_address][connection.port] = {channel_id, connection_type, priority};
  return result;
}

template <typename R>
R create_names(auto &connections) {
  using result_type = std::remove_cvref<R>::type;
  result_type result;
  for (auto &[channel_id, tmp_1] : connections)
    for (auto &[connection_type, tmp_2] : tmp_1)
      for (auto &[priority, connection] : tmp_2)
        result[channel_id][connection_type][priority] = connection.name;
  return result;
}
}  // namespace

// === IMPLEMENTATION ===

Config::Config(std::string_view const &filename, bool verbose)
    : connections_{read_connections<decltype(connections_)>(filename, verbose)},
      connection_types_{create_connection_types<decltype(connection_types_)>(connections_)}, names_{create_names<decltype(names_)>(connections_)} {
}

std::string_view Config::get_name(uint16_t channel_id, ConnectionType connection_type, Priority priority) const {
  auto iter_1 = names_.find(channel_id);
  if (iter_1 != std::end(names_)) {
    auto &tmp_1 = (*iter_1).second;
    auto iter_2 = tmp_1.find(connection_type);
    if (iter_2 != std::end(tmp_1)) {
      auto &tmp_2 = (*iter_2).second;
      auto iter_3 = tmp_2.find(priority);
      if (iter_3 != std::end(tmp_2)) {
        return (*iter_3).second;
      }
    }
  }
  throw RuntimeError{"Unexpected: channel_id={}, connection_type={}, priority={}"sv, channel_id, magic_enum::enum_name(connection_type), priority};
}

}  // namespace mdp
}  // namespace cme
}  // namespace roq
