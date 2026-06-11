/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/cme/gateway/mdp_receiver.hpp"

#include "roq/utils/metrics/factory.hpp"

#include "roq/utils/debug/hex/message.hpp"

#include "roq/io/network_address.hpp"

using namespace std::literals;

namespace roq {
namespace cme {
namespace gateway {

// === CONSTANTS ===

namespace {
auto const SOCKET_OPTIONS = Mask{
    io::SocketOption::REUSE_ADDRESS,
};
}  // namespace

// === HELPERS ===

namespace {
auto create_name(auto stream_id, auto &channel_name) {
  return fmt::format("{}:{}"sv, stream_id, channel_name);
}

auto create_receiver(auto &handler, auto &context, auto &shared, auto channel_id, auto connection_type, auto priority) {
  log::info(R"(Create channel_id="{}, priority={}")"sv, channel_id, priority);
  auto [multicast_address, port] = shared.get_multicast_config(channel_id, connection_type, priority);
  log::info("Create multicast receiver port={}"sv, port);
  auto network_address = io::NetworkAddress{port};
  auto receiver = context.create_udp_receiver(handler, network_address, SOCKET_OPTIONS);
  log::info(R"(Local interface is "{}")"sv, shared.settings.multicast.local_interface);
  auto local_interface = io::NetworkAddress::create_blocking(shared.settings.multicast.local_interface);
  log::info(R"(Add membership "{}")"sv, multicast_address);
  auto multicast_address_2 = io::NetworkAddress::create_blocking(multicast_address);
  (*receiver).add_membership(multicast_address_2, local_interface);
  return receiver;
}

struct create_metrics final : public utils::metrics::Factory {
  create_metrics(auto &settings, auto &group, auto const &function) : utils::metrics::Factory{settings.app.name, group, function} {}
};
}  // namespace

// === IMPLEMENTATION ===

MDPReceiver::MDPReceiver(
    io::Context &context, Shared &shared, market_data::Manager &manager, uint16_t channel_id, protocol::mdp::ConnectionType connection_type, Priority priority)
    : channel_id{channel_id}, connection_type{connection_type}, priority{priority}, manager_{manager},
      name_{manager_.get_name(channel_id, connection_type, priority)},
      receiver_{create_receiver(*this, context, shared, channel_id, connection_type, priority)},
      counter_{
          .disconnect = create_metrics(shared.settings, name_, "disconnect"sv),
      },
      profile_{
          .parse = create_metrics(shared.settings, name_, "parse"sv),
      },
      shared_{shared} {
}

void MDPReceiver::operator()(io::net::udp::Receiver::Read const &) {
  TraceInfo trace_info;
  while (true) {
    auto bytes = (*receiver_).recv(shared_.buffer);
    log::info<5>("Received {} byte(s) (channel_id={})"sv, bytes, channel_id);
    if (!bytes) {
      return;
    }
    std::span payload{std::data(shared_.buffer), bytes};
    log::info<5>("{}"sv, utils::debug::hex::Message{payload});
    manager_.dispatch(channel_id, connection_type, priority, payload, trace_info);
  }
}

void MDPReceiver::operator()(io::net::udp::Receiver::Error const &error) {
  log::fatal("Error: what={}"sv, error.what);
}

void MDPReceiver::operator()(metrics::Writer &writer) const {
  writer  //
      .write(counter_.disconnect, metrics::Type::COUNTER)
      .write(profile_.parse, metrics::Type::PROFILE);
}

}  // namespace gateway
}  // namespace cme
}  // namespace roq
