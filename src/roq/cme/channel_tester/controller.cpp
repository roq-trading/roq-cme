/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/cme/channel_tester/controller.hpp"

#include <magic_enum.hpp>

#include "roq/logging.hpp"

#include "roq/debug/hex/message.hpp"

#include "roq/io/engine/context_factory.hpp"

#include "roq/cme/sbe/parser.hpp"

#include "roq/cme/channel_tester/flags/flags.hpp"

using namespace std::literals;

namespace roq {
namespace cme {
namespace channel_tester {

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
    // log::print("{}\n"sv, debug::hex::Message{message});
    if (sbe::Frame::parse(message, [&](auto &frame) {
          log::info<1>("frame={}"sv, frame);
          auto sequence_number = frame.sequence_number;
          if (last_sequence_number_) {
            auto delta = static_cast<int64_t>(sequence_number) - static_cast<int64_t>(last_sequence_number_);
            if (delta != 1) {
              log::info(
                  "sequence_number={}, last_sequence_number={}, delta={}"sv,
                  sequence_number,
                  last_sequence_number_,
                  delta);
            }
          } else {
            log::info("sequence_number={} (INITIALIZE)"sv, sequence_number);
          }
          last_sequence_number_ = sequence_number;
          if (flags::Flags::test_low_sequence_numbers() && sequence_number < 256)
            log::print("{}\n"sv, debug::hex::Message{message});
        })) {
    } else {
      log::warn("Unexpected"sv);
    }
  }
}

void Controller::operator()(io::net::udp::Receiver::Error const &error) {
  log::warn("Error: what={}"sv, error.what);
}

}  // namespace channel_tester
}  // namespace cme
}  // namespace roq
