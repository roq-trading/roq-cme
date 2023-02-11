/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/cme/tester/controller.hpp"

#include <magic_enum.hpp>

#include "roq/logging.hpp"

#include "roq/debug/hex/message.hpp"

#include "roq/io/engine/context_factory.hpp"

#include "roq/cme/tester/flags/flags.hpp"

using namespace std::literals;

namespace roq {
namespace cme {
namespace tester {

// === HELPERS ===

namespace {
auto create_receiver(auto &handler, auto &context) {
  auto network_address = io::NetworkAddress{flags::Flags::multicast_port()};
  auto socket_options = Mask{
      io::SocketOption::REUSE_ADDRESS,
  };
  auto receiver = context.create_udp_receiver(handler, network_address, socket_options);
  auto local_interface = io::NetworkAddress::create_blocking(flags::Flags::local_interface());
  auto multicast_address = io::NetworkAddress::create_blocking(flags::Flags::multicast_address());
  (*receiver).add_membership(multicast_address, local_interface);
  return receiver;
}
}  // namespace

// === IMPLEMENTATION ===

Controller::Controller()
    : context_{io::engine::ContextFactory::create_libevent()}, terminate_{(*context_).create_signal(
                                                                   *this, io::sys::Signal::Type::TERMINATE)},
      interrupt_{(*context_).create_signal(*this, io::sys::Signal::Type::INTERRUPT)},
      bus_error_{(*context_).create_signal(*this, io::sys::Signal::Type::BUS_ERROR)}, receiver_{create_receiver(
                                                                                          *this, *context_)},
      buffer_(4096) {
}

void Controller::dispatch() {
  (*context_).dispatch();
}

void Controller::operator()(io::sys::Signal::Event const &event) {
  log::warn("*** SIGNAL: {} ***"sv, magic_enum::enum_name(event.type));
  (*context_).stop();
}

void Controller::operator()(io::net::udp::Receiver::Read const &read) {
  auto &receiver = read.receiver;
  for (;;) {
    auto bytes = receiver.recv(buffer_);
    if (!bytes)
      return;
    auto message = std::span{std::data(buffer_), bytes};
    log::info<5>("received {} byte(s)"sv, std::size(message));
    log::print("{}\n"sv, debug::hex::Message{message});
  }
}

void Controller::operator()(io::net::udp::Receiver::Error const &error) {
  log::warn("Error: what={}"sv, error.what);
}

}  // namespace tester
}  // namespace cme
}  // namespace roq
