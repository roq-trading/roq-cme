/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/cme/mdp/config.hpp"
#include "roq/cme/mdp/connection_type.hpp"

#include "roq/cme/market_data/manager.hpp"

#include "roq/cme/pcap_import/settings.hpp"

namespace roq {
namespace cme {
namespace pcap_import {

struct Controller final : public market_data::Manager::Handler {
  explicit Controller(Settings const &);

  Controller(Controller const &) = delete;
  Controller(Controller &&) = delete;

  void dispatch(std::string_view const &path);

 protected:
  // market_data::Manager::Handler
  void operator()(Trace<StreamStatus> const &) override;
  void operator()(Trace<ExternalLatency> const &) override;
  void operator()(Trace<ReferenceData> const &, bool is_last) override;
  void operator()(Trace<MarketStatus> const &, bool is_last) override;
  void operator()(Trace<TopOfBook> const &, bool is_last) override;
  void operator()(Trace<MarketByPriceUpdate> const &, bool is_last) override;
  void operator()(Trace<MarketByOrderUpdate> const &, bool is_last) override;
  void operator()(Trace<TradeSummary> const &, bool is_last) override;
  void operator()(Trace<StatisticsUpdate> const &, bool is_last) override;

 private:
  Settings const &settings_;
  mdp::Config config_;
  market_data::Manager market_data_;
};

}  // namespace pcap_import
}  // namespace cme
}  // namespace roq
