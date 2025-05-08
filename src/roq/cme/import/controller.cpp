/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/cme/import/controller.hpp"

#include "roq/logging.hpp"

#include "roq/utils/compare.hpp"
#include "roq/utils/datetime.hpp"

#include "roq/utils/debug/hex/message.hpp"

#include "roq/utils/pcap/reader.hpp"

#include "roq/market/mbp/factory.hpp"

#include "roq/market/mbo/factory.hpp"

#include "roq/core/codec/encoder.hpp"

using namespace std::literals;

namespace roq {
namespace cme {
namespace import {

// === CONSTANTS ===

namespace {
auto const ENABLE_MARKET_BY_ORDER = true;
auto const FILTER_SNAPSHOT_FROM_INCREMENTAL = 0uz;
auto const LOCAL_INTERFACE = "pcap"sv;
auto const MULTICAST_TIMEOUT = 10s;
std::vector<core::event_log::User> const USERS;
auto const TIMER_FREQUENCY = 100ms;
}  // namespace

// === HELPERS ===

namespace {
auto get_first_timestamp(auto &path) {
  struct Bridge final : public utils::pcap::Reader::Handler {
    std::chrono::nanoseconds result = {};

   protected:
    bool operator()(
        std::chrono::nanoseconds timestamp,
        [[maybe_unused]] std::string_view const &source_address,
        [[maybe_unused]] uint16_t source_port,
        [[maybe_unused]] std::string_view const &destination_address,
        [[maybe_unused]] uint16_t destination_port,
        [[maybe_unused]] std::span<std::byte const> const &payload) override {
      result = timestamp;
      return result.count();
    }
  };
  log::info(R"(Fetching first timestamp... (path="{}"))"sv, path);
  Bridge bridge;
  utils::pcap::Reader::dispatch(bridge, path);
  log::info("timestamp={}"sv, bridge.result);
  return bridge.result;
}

auto create_gateway_settings(auto &settings) -> GatewaySettings {
  return {
      .supports = {},
      .mbp_max_depth = settings.misc.mbp_max_depth,
      .mbp_tick_size_multiplier = NaN,
      .mbp_min_trade_vol_multiplier = NaN,
      .mbp_allow_remove_non_existing = false,
      .mbp_allow_price_inversion = settings.misc.mbp_allow_price_inversion,
      .mbp_checksum = {},
      .oms_download_has_state = {},
      .oms_download_has_routing_id = {},
      .oms_request_id_type = {},
      .oms_cancel_all_orders = {},
  };
}

auto create_market_data(auto &handler, auto &settings, auto &config, auto &security_definitions, auto pcap_first_timestamp) {
  auto options = market_data::Options{
      .cache_all_reference_data = settings.cache_all_reference_data,
      .enable_market_by_order = ENABLE_MARKET_BY_ORDER,
      .mbp_to_mbo_clear_price_level = settings.test.mbp_to_mbo_clear_price_level,
      .filter_snapshot_from_incremental = FILTER_SNAPSHOT_FROM_INCREMENTAL,
      .local_interface = LOCAL_INTERFACE,
      .multicast_timeout = MULTICAST_TIMEOUT,
      .secdef_config_file = settings.cme.secdef_file,
      .pcap_first_timestamp = pcap_first_timestamp,
  };
  uint16_t stream_id = {};
  return market_data::Manager{handler, options, security_definitions, settings.channel_ids, config, stream_id};
}

template <typename R>
R create_symbols_regex(auto &symbols) {
  using result_type = std::remove_cvref<R>::type;
  result_type result;
  for (auto &item : symbols) {
    result.emplace_back(item);
  }
  return result;
}
}  // namespace

// === IMPLEMENTATION ===

Controller::Controller(Settings const &settings, std::string_view const &pcap_path)
    : settings_{settings}, pcap_path_{pcap_path}, gateway_settings_{create_gateway_settings(settings)}, config_{settings.cme.config_file, false},
      symbols_regex_{create_symbols_regex<decltype(symbols_regex_)>(settings.symbols)}, first_timestamp_{get_first_timestamp(pcap_path_)},
      security_definitions_{*this, settings.cme.secdef_file},
      market_data_{create_market_data(*this, settings, config_, security_definitions_, first_timestamp_)}, encode_buffer_(settings.misc.encode_buffer_size),
      mbp_depth_(settings.test.depth), mbo_depth_(settings.test.depth) {
  log::info("test={}"sv, settings_.test.mbp_mbo);
  log::info("gateway_settings={}"sv, gateway_settings_);
}

void Controller::dispatch() {
  utils::pcap::Reader::dispatch(*this, pcap_path_);
  if (producer_) {
    (*producer_).close();
  }
}

bool Controller::operator()(
    std::chrono::nanoseconds timestamp,
    [[maybe_unused]] std::string_view const &source_address,
    [[maybe_unused]] uint16_t source_port,
    std::string_view const &destination_address,
    uint16_t destination_port,
    std::span<std::byte const> const &payload) {
  auto include = [](auto connection_type, auto priority) {
    if (connection_type == mdp::ConnectionType::INCREMENTAL) {
      return true;
    }
    return priority == Priority::PRIMARY;  // XXX same as live
  };
  TraceInfo trace_info{timestamp, timestamp, timestamp};
  if (!initialized_) {
    initialized_ = true;
    auto message_info = create_message_info(trace_info);
    Start start;
    Event event{message_info, start};
    market_data_(event);
  }
  while (last_timer_update_ < timestamp) {
    if (last_timer_update_.count()) {
      last_timer_update_ += TIMER_FREQUENCY;
      TraceInfo trace_info_2{last_timer_update_, last_timer_update_, last_timer_update_};
      auto message_info = create_message_info(trace_info_2);
      Timer timer;
      Event event{message_info, timer};
      market_data_(event);
    } else {
      last_timer_update_ = timestamp;
    }
  }
  log::info<5>("timestamp={}, address={}, port={}, payload={}"sv, timestamp, destination_address, destination_port, utils::debug::hex::Message{payload});
  if (config_.find(destination_address, destination_port, [&](auto channel_id, auto connection_type, auto priority) {
        if (include(connection_type, priority)) {
          TraceInfo trace_info{timestamp, timestamp, timestamp};
          market_data_.dispatch(channel_id, connection_type, priority, payload, trace_info);
        }
      })) {
  } else {
    log::warn(R"(Unexpected: address="{}", port={})"sv, destination_address, destination_port);
  }
  return false;
}

// market_data::Manager::Handler

bool Controller::discard_symbol(std::string_view const &symbol) {
  auto iter = discard_symbol_.find(symbol);
  if (iter != std::end(discard_symbol_)) {
    return (*iter).second;
  }
  bool discard = !std::empty(symbols_regex_);
  for (auto &regex : symbols_regex_) {  // note! O(n)
    if (regex.match(symbol)) {
      discard = false;
      break;
    }
  }
  if (discard) {
    log::info<1>(R"(Discard symbol="{}" (reason: no regex match))"sv, symbol);
  }
  discard_symbol_.emplace(symbol, discard);
  return discard;
}

void Controller::operator()(Trace<StreamStatus> const &event) {
  // log::info("event={}"sv, event);
  append(event);
}

void Controller::operator()(Trace<ExternalLatency> const &event) {
  // log::info("event={}"sv, event);
  append(event);
}

void Controller::operator()(Trace<RateLimitsUpdate> const &event) {
  // log::info("event={}"sv, event);
  append(event);
}

void Controller::operator()(Trace<ReferenceData> const &event, [[maybe_unused]] bool is_last) {
  // log::info("event={}"sv, event);
  auto &[trace_info, reference_data] = event;
  if (settings_.cache_all_reference_data || !reference_data.discard) {
    append(event);
  }
  auto &exchange = event.value.exchange;
  auto &symbol = event.value.symbol;
  get_market_by_price(exchange, symbol)(event.value);
  get_market_by_order(exchange, symbol)(event.value);
}

void Controller::operator()(Trace<MarketStatus> const &event, [[maybe_unused]] bool is_last) {
  // log::info("event={}"sv, event);
  append(event);
}

void Controller::operator()(Trace<TopOfBook> const &event, [[maybe_unused]] bool is_last) {
  // log::info("event={}"sv, event);
  append(event);
}

void Controller::operator()(Trace<MarketByPriceUpdate> const &event, [[maybe_unused]] bool is_last) {
  // log::info("event={}"sv, event);
  append(event);
}

// static bool test = false;

void Controller::operator()(Trace<MarketByOrderUpdate> const &event, [[maybe_unused]] bool is_last) {
  log::info<5>("event={}"sv, event);
  append(event);
  if (settings_.test.mbp_mbo) {
    DEBUG_compare(event.value.exchange, event.value.symbol, event.value.exchange_time_utc);
  }
}

void Controller::operator()(Trace<TradeSummary> const &event, [[maybe_unused]] bool is_last) {
  // log::info("event={}"sv, event);
  append(event);
}

void Controller::operator()(Trace<StatisticsUpdate> const &event, [[maybe_unused]] bool is_last) {
  // log::info("event={}"sv, event);
  append(event);
}

roq::cache::MarketByPrice &Controller::get_market_by_price([[maybe_unused]] std::string_view const &exchange, std::string_view const &symbol) {
  auto iter = market_by_price_.find(symbol);
  if (iter == std::end(market_by_price_)) {
    auto market_by_price = market::mbp::Factory::create(exchange, symbol, gateway_settings_);
    auto res = market_by_price_.emplace(symbol, std::move(market_by_price));
    iter = res.first;
  }
  return *(*iter).second;
}

roq::cache::MarketByOrder &Controller::get_market_by_order([[maybe_unused]] std::string_view const &exchange, std::string_view const &symbol) {
  auto iter = market_by_order_.find(symbol);
  if (iter == std::end(market_by_order_)) {
    auto market_by_order = market::mbo::Factory::create(exchange, symbol, gateway_settings_);
    auto res = market_by_order_.emplace(symbol, std::move(market_by_order));
    iter = res.first;
  }
  return *(*iter).second;
}

// helpers

void Controller::create_producer(std::chrono::nanoseconds timestamp_utc) {
  auto path = settings_.event_log.output_file;
  auto paths = std::make_tuple<std::string, std::string, std::string>(std::string{path}, {}, {});
  if (std::empty(std::get<0>(paths))) {
    auto create_directories = true;
    auto create_symlink = false;
    paths = core::event_log::Producer::create_paths(
        settings_.event_log.dir,
        Category::PUBLIC,
        settings_.name,
        timestamp_utc,
        core::event_log::Producer::DirectoryFormat::ISO_WEEK,
        create_directories,
        create_symlink);
  }
  auto config = core::event_log::Producer::Config{
      .input_buffer_size = settings_.event_log.buffer_size,
      .output_buffer_size = settings_.event_log.buffer_size,
      .compression_level = static_cast<uint8_t>(settings_.event_log.compression_level),
      .encoding = core::event_log::Encoding::FLATBUFFERS,
      .utimes_on_sync = false,
  };
  producer_ = std::make_unique<core::event_log::Producer>(ROQ_PACKAGE_NAME, settings_.name, source_session_id_, paths, USERS, config);
}

template <typename T>
void Controller::append(Trace<T> const &event) {
  auto &[trace_info, value] = event;
  auto message_info = create_message_info(trace_info);
  auto message = core::codec::Encoder{encode_buffer_}.encode(value);
  if (!producer_) {
    create_producer(message_info.origin_create_time_utc);
  }
  // XXX HANS -- this should be cleaned up -- why core::queue ???
  auto header = core::queue::Message::Header{
      .boundary = {},
      .origin_create_time = {},
      .source_receive_dtime = {},
      .source_send_dtime = {},
      .source_seqno = message_info.source_seqno,
      .opaque = {},
      .message_length = {},
      .type = core::queue::Message::Type::MESSAGE,  // REQUIRED
      .is_last = {},
      .source_id = {},
      .unused = {},
      .origin_create_time_utc = static_cast<uint64_t>(message_info.origin_create_time.count()),  // REQUIRED
      .receive_time_utc = static_cast<uint64_t>(message_info.receive_time_utc.count()),          // REQUIRED
  };
  (*producer_).write(header, message, message_info);
}

MessageInfo Controller::create_message_info(TraceInfo const &trace_info) {
  return {
      .source = SOURCE_NONE,
      .source_name = settings_.name,
      .source_session_id = source_session_id_,
      .source_seqno = ++source_seqno_,
      .receive_time_utc = trace_info.origin_create_time_utc,
      .receive_time = trace_info.origin_create_time,
      .source_send_time = {},
      .source_receive_time = trace_info.source_receive_time,
      .origin_create_time = trace_info.origin_create_time,
      .origin_create_time_utc = trace_info.origin_create_time_utc,
      .is_last = true,
      .opaque = {},
  };
}

void Controller::DEBUG_compare(std::string_view const &exchange, std::string_view const &symbol, std::chrono::nanoseconds exchange_time_utc) {
  auto &market_by_price = get_market_by_price(exchange, symbol);
  auto &market_by_order = get_market_by_order(exchange, symbol);
  market_by_price.extract(mbp_depth_);
  market_by_order.extract(mbo_depth_, settings_.test.depth);
  print("{} {} {}\n"sv, symbol, utils::DateTime_iso8601{exchange_time_utc}, exchange_time_utc);
  auto length = std::min(std::size(mbp_depth_), std::size(mbo_depth_));
  auto print = [](std::string_view prefix, auto lhs, auto rhs) {
    auto same = utils::compare(lhs, rhs) == 0;
    fmt::print(" {}={{"sv, prefix);
    fmt::print("{:10}|{:10}"sv, lhs, rhs);
    fmt::print("}}"sv);
    return same;
  };
  for (size_t i = 0; i < length; ++i) {
    auto &lhs = mbp_depth_[i];
    auto &rhs = mbo_depth_[i];
    fmt::print("{} [{:2}]"sv, symbol, i);
    auto same = true;
    same &= print("bp"sv, lhs.bid_price, rhs.bid_price);
    same &= print("bq"sv, lhs.bid_quantity, rhs.bid_quantity);
    same &= print("ap"sv, lhs.ask_price, rhs.ask_price);
    same &= print("aq"sv, lhs.ask_quantity, rhs.ask_quantity);
    if (!same) {
      fmt::print(" WRONG"sv);
    }
    fmt::print("\n"sv);
  }
}

}  // namespace import
}  // namespace cme
}  // namespace roq
