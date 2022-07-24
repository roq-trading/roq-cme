/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/deribit/udp_snapshot.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "roq/utils/update.hpp"

#include "roq/core/back_emplacer.hpp"

#include "roq/debug/hex/message.hpp"

#include "roq/io/network_address.hpp"

#include "roq/core/metrics/factory.hpp"

#include "roq/deribit/utils.hpp"

#include "roq/deribit/flags/common.hpp"
#include "roq/deribit/flags/config.hpp"
#include "roq/deribit/flags/multicast.hpp"

#include "roq/deribit/sbe/utils.hpp"

using namespace std::literals;

namespace roq {
namespace deribit {

namespace {
auto const NAME = "udps"sv;

auto get_supports(auto publish_market_by_price) {
  Mask<SupportType> result;
  if (publish_market_by_price)
    result |= SupportType::MARKET_BY_PRICE;
  return result;
}

struct create_metrics final : public core::metrics::Factory {
  explicit create_metrics(std::string_view const &group, std::string_view const &function)
      : core::metrics::Factory(server::Flags::name(), group, function) {}
};

auto create_receiver(auto &handler, auto &context, auto port) {
  log::info<1>("Create multicast socket port={}"sv, port);
  auto receiver = context.create_udp_receiver(handler, io::NetworkAddress{port});
  log::info<1>(R"(Local interface is "{}")"sv, flags::Multicast::local_interface());
  std::string local_interface{flags::Multicast::local_interface()};
  struct in_addr local = {};
  local.s_addr = inet_addr(local_interface.c_str());
  for (auto &multicast_address : flags::Multicast::multicast_address()) {
    log::info<1>(R"(Add membership "{}")"sv, multicast_address);
    struct in_addr multicast = {};
    multicast.s_addr = inet_addr(multicast_address.c_str());
    (*receiver).add_membership(io::NetworkAddress{0, multicast}, io::NetworkAddress{0, local});
  }
  return receiver;
}
}  // namespace

UDPSnapshot::UDPSnapshot(Handler &handler, io::Context &context, uint16_t stream_id, Shared &shared)
    : handler_(handler), stream_id_(stream_id), name_(fmt::format("{}:{}"sv, stream_id_, NAME)),
      publish_market_by_price_(!flags::Multicast::multicast_disable_market_by_price()),
      supports_(get_supports(publish_market_by_price_)),
      receiver_(create_receiver(*this, context, flags::Multicast::multicast_port_snapshot())),
      counter_{
          .disconnect = create_metrics(name_, "disconnect"sv),
      },
      profile_{
          .parse = create_metrics(name_, "parse"sv),
      },
      shared_(shared) {
  log::info("DEBUG: publish_market_by_price={}"sv, publish_market_by_price_);
}

void UDPSnapshot::operator()(Event<Start> const &) {
  auto trace_info = server::create_trace_info();
  last_update_time_ = trace_info.source_receive_time;
  publish_stream_status(trace_info, ConnectionStatus::CONNECTING);
}

void UDPSnapshot::operator()(Event<Stop> const &) {
}

void UDPSnapshot::operator()(Event<Timer> const &event) {
  if (last_update_time_.count() && (last_update_time_ + flags::Multicast::multicast_timeout()) < event.value.now) {
    log::warn("*** DETECTED TIMEOUT ***"sv);
    last_update_time_ = {};
  }
}

void UDPSnapshot::operator()(io::net::udp::Receiver::Read const &) {
  auto trace_info = server::create_trace_info();
  last_update_time_ = trace_info.source_receive_time;
  publish_stream_status(trace_info, ConnectionStatus::READY);  // first message will publish
  while (receive_buffer_.append(*receiver_)) {
    auto message = receive_buffer_.data();
    log::info<5>("received {} byte(s)"sv, std::size(message));
    if (!sbe::Parser::dispatch(*this, message, trace_info)) {
      log::warn("{}"sv, debug::hex::Message{message});
      log::fatal("Failed to parse message"sv);
    }
    receive_buffer_.drain(std::size(message));
  }
}

void UDPSnapshot::operator()(io::net::udp::Receiver::Error const &error) {
  log::fatal("Error: what={}"sv, error.what);
}

void UDPSnapshot::operator()(Trace<deribit_multicast::Instrument> const &event, sbe::Frame const &frame) {
  auto &instrument = event.value;
  log::info<2>("instrument={}, frame={}"sv, instrument, frame);
  auto &aggregator = get_aggregator(frame.channel_id);
  if (aggregator(frame.sequence_number)) {
    // note! always include
    auto const instrument_id = instrument.instrumentId();
    shared_.find_instrument_name_with_create(instrument_id, [&]() {
      auto contract_size = instrument.contractSize();
      auto multiplier = compute_contracts_multiplier(contract_size);
      return Instrument{
          sbe::get_instrument_name(instrument),  // note! alloc
          contract_size,
          multiplier,
      };
    });
  }
}

void UDPSnapshot::operator()(Trace<deribit_multicast::Book> const &event, sbe::Frame const &frame) {
  auto &book = event.value;
  log::warn("book={}, frame={}"sv, book, frame);
  log::fatal("Unexpected"sv);
}

void UDPSnapshot::operator()(Trace<deribit_multicast::Ticker> const &event, sbe::Frame const &frame) {
  auto &ticker = event.value;
  log::info<4>("ticker={}, frame={}"sv, ticker, frame);
}

void UDPSnapshot::operator()(Trace<deribit_multicast::Trades> const &event, sbe::Frame const &frame) {
  auto &trades = event.value;
  log::warn("trades={}, frame={}"sv, trades, frame);
  log::fatal("Unexpected"sv);
}

void UDPSnapshot::operator()(Trace<deribit_multicast::Snapshot> const &event, sbe::Frame const &frame) {
  auto &trace_info = event.trace_info;
  auto &snapshot = event.value;
  log::info<4>("snapshot={}, frame={}"sv, snapshot, frame);
  auto const instrument_id = snapshot.instrumentId();
  auto const change_id = snapshot.changeId();
  auto const is_last = snapshot.isLastInBook();
  auto &aggregator = get_aggregator(frame.channel_id);
  aggregator(frame.sequence_number, instrument_id, change_id, is_last, [&](auto &bids, auto &asks) {
    if (!publish_market_by_price_)
      return;
    if (shared_.find_instrument(instrument_id, [&](auto &instrument) {
          snapshot.sbeRewind();
          snapshot.levelsList().forEach(
              [&](auto const &item) { emplace_back(item, instrument.multiplier, bids, asks); });
          log::info<5>(
              "DEBUG: sequence_number={}, instrument_id={}, change_id={}, is_last={}, bids=[{}], asks=[{}]"sv,
              frame.sequence_number,
              instrument_id,
              change_id,
              is_last,
              fmt::join(bids, ","sv),
              fmt::join(asks, ","sv));
          // XXX allow empty? (after all, we need to record the sequence number...)
          if (is_last && !(std::empty(bids) && std::empty(asks))) {
            std::chrono::milliseconds const timestamp{snapshot.timestampMs()};
            auto &collector = shared_.mbp_collector[instrument.symbol];
            collector(
                bids,
                asks,
                change_id,
                [&](auto &bids, auto &asks, auto sequence) {  // snapshot
                  log::info<1>(R"(Received snapshot: symbol="{}")"sv, instrument.symbol);
                  // log::debug(R"(PUBLISH SNAPSHOT symbol="{}", sequence={})"sv, symbol, sequence);
                  log::info<5>(
                      R"(DEBUG: PUBLISH SNAPSHOT symbol="{}", sequence={}, change_id={}, timestamp={})"sv,
                      instrument.symbol,
                      sequence,
                      change_id,
                      timestamp);
                  const MarketByPriceUpdate market_by_price_update{
                      .stream_id = stream_id_,
                      .exchange = flags::Config::exchange(),
                      .symbol = instrument.symbol,
                      .bids = bids,
                      .asks = asks,
                      .update_type = UpdateType::SNAPSHOT,
                      .exchange_time_utc = timestamp,
                      .exchange_sequence = sequence,
                      .price_decimals = {},
                      .quantity_decimals = {},
                      .checksum = {},
                  };
                  Trace event(trace_info, market_by_price_update);
                  shared_(
                      event, true, [&](auto &market_by_price) { collector.apply(market_by_price, sequence, true); });
                },
                [&](auto retries) {  // request
                  log::info<1>(R"(Waiting for snapshot: symbol="{}")"sv, instrument.symbol);
                  // log::debug(R"(REQUEST symbol="{}" (retries={}))"sv, instrument.symbol, retries);
                  log::info<5>(R"(DEBUG: REQUEST symbol="{}" (retries={}))"sv, instrument.symbol, retries);
                  // note! don't have to do anything -- just wait for snapshot
                });
          }
        })) {
    } else {
      // unknown instrument_id
      log::info<5>("DEBUG: unknown instrument_id={}"sv, instrument_id);
    }
  });
}

void UDPSnapshot::operator()(metrics::Writer &writer) {
  writer  //
      .write(counter_.disconnect, metrics::COUNTER)
      .write(profile_.parse, metrics::PROFILE);
}

void UDPSnapshot::publish_stream_status(TraceInfo const &trace_info, ConnectionStatus connection_status) {
  if (!utils::update(connection_status_, connection_status))
    return;
  const StreamStatus stream_status{
      .stream_id = stream_id_,
      .account = {},
      .supports = supports_,
      .transport = Transport::UDP,
      .protocol = Protocol::SBE,
      .encoding = {Encoding::SBE},
      .priority = Priority::PRIMARY,
      .connection_status = connection_status_,
  };
  log::info("stream_status={}"sv, stream_status);
  create_trace_and_dispatch(handler_, trace_info, stream_status);
}

template <typename T, typename U>
void UDPSnapshot::emplace_back(const T &item, double multiplier, U &bids, U &asks) {
  const MBPUpdate mbp_update{
      .price = item.price(),
      .quantity = item.amount() * multiplier,
      .implied_quantity = NaN,
      .number_of_orders = {},
      .update_action = {},
      .price_level = {},
  };
  auto side = sbe::map_book_side(deribit_multicast::BookSide::get(item.side()));
  switch (side) {
    case Side::UNDEFINED:
      assert(false);
      break;
    case Side::BUY:
      bids.emplace_back(std::move(mbp_update));
      break;
    case Side::SELL:
      asks.emplace_back(std::move(mbp_update));
      break;
  }
}

Aggregator &UDPSnapshot::get_aggregator(uint16_t channel_id) {
  auto iter = aggregator_.find(channel_id);
  if (iter == std::end(aggregator_)) {
    iter = aggregator_.emplace(channel_id, server::Flags::cache_mbp_max_depth()).first;
  }
  return (*iter).second;
}

}  // namespace deribit
}  // namespace roq
