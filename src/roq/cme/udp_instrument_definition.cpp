/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/cme/udp_instrument_definition.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "roq/utils/update.hpp"

#include "roq/core/back_emplacer.hpp"

#include "roq/debug/hex/message.hpp"

#include "roq/core/metrics/factory.hpp"

#include "roq/io/network_address.hpp"

#include "roq/cme/utils.hpp"

#include "roq/cme/flags/common.hpp"
#include "roq/cme/flags/config.hpp"
#include "roq/cme/flags/multicast.hpp"

#include "roq/cme/sbe/utils.hpp"

using namespace std::literals;

namespace roq {
namespace cme {

namespace {
auto const NAME = "udpe"sv;

Mask<SupportType> const SUPPORTS{
    SupportType::REFERENCE_DATA,
};

struct create_metrics final : public core::metrics::Factory {
  explicit create_metrics(std::string_view const &group, std::string_view const &function)
      : core::metrics::Factory(server::Flags::name(), group, function) {}
};

auto create_receiver(auto &handler, auto &context, auto &shared) {
  auto [multicast_address, port] = shared.get_multicast_config(multicast::Type::INCREMENTAL, Priority::PRIMARY);
  log::info<0>("Create multicast socket port={}"sv, port);
  auto receiver = context.create_udp_receiver(handler, io::NetworkAddress{port});
  log::info<0>(R"(Local interface is "{}")"sv, flags::Multicast::multicast_local_interface());
  std::string local_interface{flags::Multicast::multicast_local_interface()};
  struct in_addr local = {};
  local.s_addr = inet_addr(local_interface.c_str());
  log::info<0>(R"(Add membership "{}")"sv, multicast_address);
  struct in_addr multicast = {};
  multicast.s_addr = inet_addr(multicast_address.c_str());
  (*receiver).add_membership(io::NetworkAddress{0, multicast}, io::NetworkAddress{0, local});
  return receiver;
}

bool test_sequence(auto &cache, auto instrument_id, auto sequence_number) {
  auto result = false;
  const constexpr uint32_t midpoint = 1 << 31;
  auto iter = cache.find(instrument_id);
  if (iter != cache.end()) {
    auto previous = (*iter).second;
    if (previous < sequence_number) {
      result = true;
    } else if (sequence_number < midpoint && midpoint < previous) {
      result = true;  // wraparound
    } else {
      // out of sequence
    }
  } else {
    iter = cache.emplace(instrument_id, sequence_number).first;
    result = true;
  }
  if (result)
    (*iter).second = sequence_number;
  return result;
}
}  // namespace

UDPInstrumentDefinition::UDPInstrumentDefinition(
    Handler &handler, io::Context &context, uint16_t stream_id, Shared &shared)
    : handler_(handler), stream_id_(stream_id), name_(fmt::format("{}:{}"sv, stream_id_, NAME)),
      receiver_(create_receiver(*this, context, shared)),
      counter_{
          .disconnect = create_metrics(name_, "disconnect"sv),
      },
      profile_{
          .parse = create_metrics(name_, "parse"sv),
      },
      shared_(shared) {
}

void UDPInstrumentDefinition::operator()(Event<Start> const &) {
  auto trace_info = server::create_trace_info();
  publish_stream_status(trace_info, ConnectionStatus::CONNECTING);
  last_update_time_ = trace_info.source_receive_time;
}

void UDPInstrumentDefinition::operator()(Event<Stop> const &) {
}

void UDPInstrumentDefinition::operator()(Event<Timer> const &event) {
  if (last_update_time_.count() && (last_update_time_ + flags::Multicast::multicast_timeout()) < event.value.now) {
    log::warn("*** DETECTED TIMEOUT ***"sv);
    last_update_time_ = {};
  }
}

void UDPInstrumentDefinition::operator()(io::net::udp::Receiver::Read const &) {
  auto trace_info = server::create_trace_info();
  last_update_time_ = trace_info.source_receive_time;
  publish_stream_status(trace_info, ConnectionStatus::READY);  // first message will publish
  while (receive_buffer_.append(*receiver_)) {
    auto message = receive_buffer_.data();
    // log::info<5>("received {} byte(s)"sv, std::size(message));
    // log::info<5>("{}"sv, debug::hex::Message{message});
    if (!sbe::Parser::dispatch(*this, message, trace_info)) {
      log::warn("{}"sv, debug::hex::Message{message});
      log::fatal("Failed to parse message"sv);
    }
    receive_buffer_.clear();
  }
}

void UDPInstrumentDefinition::operator()(io::net::udp::Receiver::Error const &error) {
  log::fatal("Error: what={}"sv, error.what);
}

// sbe::Parser::Handler

// - MDInstrumentDefinition

void UDPInstrumentDefinition::operator()(
    Trace<cme_mdp::MDInstrumentDefinitionFuture54> const &event, sbe::Frame const &frame) {
  auto &[trace_info, value] = event;
  log::info<5>("md_instrument_definition_future_54={}, frame={}"sv, const_cast<decltype(value) &>(value), frame);
}

void UDPInstrumentDefinition::operator()(
    Trace<cme_mdp::MDInstrumentDefinitionOption55> const &event, sbe::Frame const &frame) {
  auto &[trace_info, value] = event;
  log::info<5>("md_instrument_definition_option_55={}, frame={}"sv, const_cast<decltype(value) &>(value), frame);
}

void UDPInstrumentDefinition::operator()(
    Trace<cme_mdp::MDInstrumentDefinitionSpread56> const &event, sbe::Frame const &frame) {
  auto &[trace_info, value] = event;
  log::info<5>("md_instrument_definition_spread_56={}, frame={}"sv, const_cast<decltype(value) &>(value), frame);
}

void UDPInstrumentDefinition::operator()(
    Trace<cme_mdp::MDInstrumentDefinitionFixedIncome57> const &event, sbe::Frame const &frame) {
  auto &[trace_info, value] = event;
  log::info<5>("md_instrument_definition_fixed_income_57={}, frame={}"sv, const_cast<decltype(value) &>(value), frame);
}

void UDPInstrumentDefinition::operator()(
    Trace<cme_mdp::MDInstrumentDefinitionRepo58> const &event, sbe::Frame const &frame) {
  auto &[trace_info, value] = event;
  log::info<5>("md_instrument_definition_repo_58={}, frame={}"sv, const_cast<decltype(value) &>(value), frame);
}

void UDPInstrumentDefinition::operator()(
    Trace<cme_mdp::MDInstrumentDefinitionFX63> const &event, sbe::Frame const &frame) {
  auto &[trace_info, value] = event;
  log::info<5>("md_instrument_definition_fx_63={}, frame={}"sv, const_cast<decltype(value) &>(value), frame);
}
// - SnapshotFullRefresh

void UDPInstrumentDefinition::operator()(Trace<cme_mdp::SnapshotFullRefresh52> const &, sbe::Frame const &) {
  log::fatal("Unexpected"sv);
}

void UDPInstrumentDefinition::operator()(Trace<cme_mdp::SnapshotFullRefreshOrderBook53> const &, sbe::Frame const &) {
  log::fatal("Unexpected"sv);
}

void UDPInstrumentDefinition::operator()(Trace<cme_mdp::SnapshotFullRefreshLongQty69> const &, sbe::Frame const &) {
  log::fatal("Unexpected"sv);
}

// - MDIncrementalRefresh

void UDPInstrumentDefinition::operator()(Trace<cme_mdp::MDIncrementalRefreshVolume37> const &, sbe::Frame const &) {
  log::fatal("Unexpected"sv);
}

void UDPInstrumentDefinition::operator()(Trace<cme_mdp::MDIncrementalRefreshBook46> const &, sbe::Frame const &) {
  log::fatal("Unexpected"sv);
}

void UDPInstrumentDefinition::operator()(Trace<cme_mdp::MDIncrementalRefreshOrderBook47> const &, sbe::Frame const &) {
  log::fatal("Unexpected"sv);
}

void UDPInstrumentDefinition::operator()(
    Trace<cme_mdp::MDIncrementalRefreshTradeSummary48> const &, sbe::Frame const &) {
  log::fatal("Unexpected"sv);
}

void UDPInstrumentDefinition::operator()(
    Trace<cme_mdp::MDIncrementalRefreshDailyStatistics49> const &, sbe::Frame const &) {
  log::fatal("Unexpected"sv);
}

void UDPInstrumentDefinition::operator()(
    Trace<cme_mdp::MDIncrementalRefreshLimitsBanding50> const &, sbe::Frame const &) {
  log::fatal("Unexpected"sv);
}

void UDPInstrumentDefinition::operator()(
    Trace<cme_mdp::MDIncrementalRefreshSessionStatistics51> const &, sbe::Frame const &) {
  log::fatal("Unexpected"sv);
}

void UDPInstrumentDefinition::operator()(
    Trace<cme_mdp::MDIncrementalRefreshBookLongQty64> const &, sbe::Frame const &) {
  log::fatal("Unexpected"sv);
}

void UDPInstrumentDefinition::operator()(
    Trace<cme_mdp::MDIncrementalRefreshTradeSummaryLongQty65> const &, sbe::Frame const &) {
  log::fatal("Unexpected"sv);
}

void UDPInstrumentDefinition::operator()(
    Trace<cme_mdp::MDIncrementalRefreshVolumeLongQty66> const &, sbe::Frame const &) {
  log::fatal("Unexpected"sv);
}

void UDPInstrumentDefinition::operator()(
    Trace<cme_mdp::MDIncrementalRefreshSessionStatisticsLongQty67> const &, sbe::Frame const &) {
  log::fatal("Unexpected"sv);
}

// - MDIncrementalRefresh

void UDPInstrumentDefinition::operator()(metrics::Writer &writer) {
  writer  //
      .write(counter_.disconnect, metrics::COUNTER)
      .write(profile_.parse, metrics::PROFILE);
}

void UDPInstrumentDefinition::publish_stream_status(TraceInfo const &trace_info, ConnectionStatus connection_status) {
  if (!utils::update(connection_status_, connection_status))
    return;
  const StreamStatus stream_status{
      .stream_id = stream_id_,
      .account = {},
      .supports = SUPPORTS,
      .transport = Transport::UDP,
      .protocol = Protocol::SBE,
      .encoding = {Encoding::SBE},
      .priority = Priority::PRIMARY,
      .connection_status = connection_status_,
  };
  log::info("stream_status={}"sv, stream_status);
  create_trace_and_dispatch(handler_, trace_info, stream_status);
}

Aggregator &UDPInstrumentDefinition::get_aggregator(uint16_t channel_id) {
  auto iter = aggregator_.find(channel_id);
  if (iter == std::end(aggregator_)) {
    iter = aggregator_.emplace(channel_id, server::Flags::cache_mbp_max_depth()).first;
  }
  return (*iter).second;
}

}  // namespace cme
}  // namespace roq
