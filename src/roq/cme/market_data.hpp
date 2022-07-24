/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <absl/container/flat_hash_set.h>

#include <string>
#include <vector>

#include "roq/core/download.hpp"

#include "roq/core/stack/buffer.hpp"

#include "roq/core/metrics/counter.hpp"
#include "roq/core/metrics/latency.hpp"
#include "roq/core/metrics/profile.hpp"

#include "roq/io/context.hpp"
#include "roq/io/net/connection_factory.hpp"
#include "roq/io/net/connection_manager.hpp"

#include "roq/server.hpp"

#include "roq/deribit/market_data_state.hpp"
#include "roq/deribit/security.hpp"
#include "roq/deribit/shared.hpp"

// session
#include "roq/deribit/fix/heartbeat.hpp"
#include "roq/deribit/fix/logon.hpp"
#include "roq/deribit/fix/logout.hpp"
#include "roq/deribit/fix/resend_request.hpp"
#include "roq/deribit/fix/test_request.hpp"

// business (inbound)
#include "roq/deribit/fix/market_data_incremental_refresh.hpp"
#include "roq/deribit/fix/market_data_request_reject.hpp"
#include "roq/deribit/fix/market_data_snapshot_full_refresh.hpp"
#include "roq/deribit/fix/security_list.hpp"
#include "roq/deribit/fix/security_status.hpp"

// business (outbound)
#include "roq/deribit/fix/market_data_request.hpp"
#include "roq/deribit/fix/security_list_request.hpp"
#include "roq/deribit/fix/security_status_request.hpp"

namespace roq {
namespace deribit {

class MarketData final : public io::net::ConnectionManager::Handler {
 public:
  struct SymbolsUpdate final {
    std::vector<Symbol> &symbols;
  };

  struct Handler {
    virtual void operator()(Trace<StreamStatus const> const &) = 0;
    virtual void operator()(Trace<ExternalLatency const> const &) = 0;
    virtual void operator()(Trace<ReferenceData const> const &, bool is_last) = 0;
    virtual void operator()(Trace<MarketByPriceUpdate const> const &, bool is_last, bool refresh) = 0;
    virtual void operator()(Trace<TradeSummary const> const &, bool is_last) = 0;
    virtual void operator()(Trace<StatisticsUpdate const> const &, bool is_last) = 0;
    // cross-communication
    virtual void operator()(SymbolsUpdate &) = 0;
  };

  MarketData(Handler &, io::Context &, uint16_t stream_id, Security &, Shared &, size_t index, bool master);

  MarketData(MarketData const &) = delete;
  MarketData(MarketData &&) = delete;

  bool ready() const { return ready_; }

  void operator()(Event<Start> const &);
  void operator()(Event<Stop> const &);
  void operator()(Event<Timer> const &);

  void operator()(metrics::Writer &);

  void subscribe(size_t start_from = 0);

  void operator()(Trace<fix::Heartbeat const> const &, core::fix::Header const &);
  void operator()(Trace<fix::Logon const> const &, core::fix::Header const &);
  void operator()(Trace<fix::Logout const> const &, core::fix::Header const &);
  void operator()(Trace<fix::ResendRequest const> const &, core::fix::Header const &);
  void operator()(Trace<fix::TestRequest const> const &, core::fix::Header const &);

  void operator()(Trace<fix::SecurityList const> const &, core::fix::Header const &);
  void operator()(Trace<fix::SecurityStatus const> const &, core::fix::Header const &);

  void operator()(Trace<fix::MarketDataIncrementalRefresh const> const &, core::fix::Header const &);
  void operator()(Trace<fix::MarketDataRequestReject const> const &, core::fix::Header const &);
  void operator()(Trace<fix::MarketDataSnapshotFullRefresh const> const &, core::fix::Header const &);

 protected:
  void operator()(io::net::ConnectionManager::Connected const &) override;
  void operator()(io::net::ConnectionManager::Disconnected const &) override;
  void operator()(io::net::ConnectionManager::Read const &) override;

 private:
  void operator()(ConnectionStatus);

  void send_logon();
  void send_logout(std::string_view const &text);
  void send_heartbeat(std::string_view const &test_req_id);
  void send_test_request(std::chrono::nanoseconds now);

  uint32_t download(MarketDataState);

  void download_securities();

  void subscribe(std::span<Symbol const> const &symbols);
  void unsubscribe(std::span<Symbol const> const &symbols);

  void resubscribe(std::string_view const &symbol);

  void parse(Trace<core::fix::Message const> const &);
  void parse_helper(Trace<core::fix::Message const> const &);

  // utilities

  template <typename T>
  void send(const T &event);

  template <typename T>
  void send(const T &event, std::chrono::nanoseconds sending_time);

  void check(core::fix::Header const &);

 private:
  Handler &handler_;
  // config
  const uint16_t stream_id_;
  const std::string name_;
  const size_t index_;
  bool const master_;
  bool const publish_market_by_price_;
  bool const publish_trade_summary_;
  Mask<SupportType> const supports_;
  // connection
  std::unique_ptr<io::net::ConnectionFactory> connection_factory_;
  std::unique_ptr<io::net::ConnectionManager> connection_manager_;
  // buffers
  core::Buffer encode_buffer_;
  core::Buffer decode_buffer_;
  core::stack::Buffer<char, 32> stack_buffer_;
  // metrics
  struct {
    core::metrics::Counter disconnect;
  } counter_;
  struct {
    core::metrics::Profile parse, security_list, security_status, market_data_incremental_refresh,
        market_data_request_reject, market_data_snapshot_full_refresh, market_data_request;
  } profile_;
  struct {
    core::metrics::Latency ping;
  } latency_;
  // state
  struct {
    uint64_t msg_seq_num = {};
  } outbound_;
  struct {
    uint64_t msg_seq_num = {};
  } inbound_;
  // security
  Security &security_;
  // cache
  Shared &shared_;
  // state
  bool ready_ = false;
  std::chrono::nanoseconds next_heartbeat_ = {};
  ConnectionStatus status_ = {};
  core::Download<MarketDataState> download_;
  std::chrono::nanoseconds last_logon_or_heartbeat_ = {};
  absl::flat_hash_set<Symbol> latch_;
  // EXPERIMENTAL
  std::chrono::nanoseconds test_disconnect_time_ = {};
};

}  // namespace deribit
}  // namespace roq
