/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/cme/channel_tester/controller.hpp"

#include <fmt/format.h>

#include <magic_enum/magic_enum_format.hpp>

#include "roq/logging.hpp"

#include "roq/io/engine/context_factory.hpp"

#include "roq/cme/mdp/parser.hpp"

using namespace std::literals;

namespace roq {
namespace cme {
namespace channel_tester {

// === CONSTANTS ===

namespace {
size_t const BUFFER_SIZE = 4096;
}

// === HELPERS ===

namespace {
auto create_context() {
  return io::engine::ContextFactory::create();
}

auto create_receiver(auto &handler, auto &settings, auto &context) {
  auto network_address = io::NetworkAddress{settings.multicast_port};
  auto socket_options = Mask{
      io::SocketOption::REUSE_ADDRESS,
  };
  auto receiver = context.create_udp_receiver(handler, network_address, socket_options);
  auto local_interface = io::NetworkAddress::create_blocking(settings.local_interface);
  auto multicast_address = io::NetworkAddress::create_blocking(settings.multicast_address);
  (*receiver).add_membership(multicast_address, local_interface);
  return receiver;
}
}  // namespace

// === IMPLEMENTATION ===

Controller::Controller(Settings const &settings)
    : settings_{settings}, context_{create_context()}, terminate_{(*context_).create_signal(*this, io::sys::Signal::Type::TERMINATE)},
      interrupt_{(*context_).create_signal(*this, io::sys::Signal::Type::INTERRUPT)},
      bus_error_{(*context_).create_signal(*this, io::sys::Signal::Type::BUS_ERROR)}, receiver_{create_receiver(*this, settings, *context_)},
      buffer_(BUFFER_SIZE) {
}

void Controller::dispatch() {
  (*context_).dispatch();
}

void Controller::operator()(io::sys::Signal::Event const &event) {
  log::warn("*** SIGNAL: {} ***"sv, event.type);
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
    if (mdp::Frame::parse(message, [&](auto &frame) {
          log::info<1>("frame={}"sv, frame);
          auto sequence_number = frame.sequence_number;
          if (sequence_number < settings_.filter_snapshot_from_incremental)
            return;
          auto now = clock::get_realtime();
          if (last_sequence_number_) {
            auto delta = static_cast<int64_t>(sequence_number) - static_cast<int64_t>(last_sequence_number_);
            if (delta != 1) {
              auto gap = now - last_update_;
              log::info(
                  "timestamp={}, last_timestamp={}, gap={}, sequence_number={}, last_sequence_number={}, delta={}"sv,
                  now,
                  last_update_,
                  gap,
                  sequence_number,
                  last_sequence_number_,
                  delta);
            }
          } else {
            log::info("sequence_number={} (INITIALIZE)"sv, sequence_number);
          }
          last_sequence_number_ = sequence_number;
          last_update_ = now;
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
