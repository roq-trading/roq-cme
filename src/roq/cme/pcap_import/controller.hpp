/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "roq/cache/market_by_order.hpp"
#include "roq/cache/market_by_price.hpp"

#include "roq/utils/container.hpp"

#include "roq/utils/regex/pattern.hpp"

#include "roq/core/event_log/producer.hpp"

#include "roq/cme/mdp/config.hpp"
#include "roq/cme/mdp/connection_type.hpp"

#include "roq/cme/market_data/dispatcher.hpp"
#include "roq/cme/market_data/manager.hpp"

#include "roq/cme/pcap_import/settings.hpp"

namespace roq {
namespace cme {
namespace pcap_import {

struct Controller final : public market_data::Dispatcher {
  explicit Controller(Settings const &);

  Controller(Controller const &) = delete;
  Controller(Controller &&) = delete;

  void dispatch(std::string_view const &path);

 protected:
  // market_data::Dispatcher
  bool discard_symbol(std::string_view const &symbol) override;
  void operator()(Trace<StreamStatus> const &) override;
  void operator()(Trace<ExternalLatency> const &) override;
  void operator()(Trace<ReferenceData> const &, bool is_last) override;
  void operator()(Trace<MarketStatus> const &, bool is_last) override;
  void operator()(Trace<TopOfBook> const &, bool is_last) override;
  void operator()(Trace<MarketByPriceUpdate> const &, bool is_last) override;
  void operator()(Trace<MarketByOrderUpdate> const &, bool is_last) override;
  void operator()(Trace<TradeSummary> const &, bool is_last) override;
  void operator()(Trace<StatisticsUpdate> const &, bool is_last) override;
  roq::cache::MarketByPrice &get_market_by_price(
      std::string_view const &exchange, std::string_view const &symbol) override;
  roq::cache::MarketByOrder &get_market_by_order(
      std::string_view const &exchange, std::string_view const &symbol) override;

  // helpers

  void create_producer(std::chrono::nanoseconds timestamp_utc);

  template <typename T>
  void append(Trace<T> const &);

  MessageInfo create_message_info(TraceInfo const &);

 private:
  Settings const &settings_;
  mdp::Config config_;
  market_data::Manager market_data_;
  std::vector<utils::regex::Pattern> const symbols_regex_;
  utils::unordered_map<std::string, bool> discard_symbol_;
  std::unique_ptr<core::event_log::Producer> producer_;
  std::vector<std::byte> encode_buffer_;
  UUID const source_session_id_ = {};
  uint64_t source_seqno_ = {};
  utils::unordered_map<std::string, std::unique_ptr<cache::MarketByPrice>> market_by_price_;
  utils::unordered_map<std::string, std::unique_ptr<cache::MarketByOrder>> market_by_order_;
};

}  // namespace pcap_import
}  // namespace cme
}  // namespace roq
