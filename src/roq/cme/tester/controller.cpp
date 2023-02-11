/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/cme/tester/controller.hpp"

#include "roq/logging.hpp"

#include "roq/io/engine/context_factory.hpp"

#include "roq/web/rest/client_factory.hpp"

// #include "roq/cme/tester/flags/flags.hpp"

using namespace std::literals;

using namespace fmt::literals;

namespace roq {
namespace cme {
namespace tester {

// === CONSTANTS ===

namespace {
auto const POLL_FREQUENCY = 10ms;
}  // namespace

// === HELPERS ===

namespace {
auto create_client(auto &handler, auto &context) {
  return std::unique_ptr<web::rest::Client>{};
}
}  // namespace

// === IMPLEMENTATION ===

Controller::Controller(std::vector<std::string> const &arguments)
    : arguments_{arguments}, context_{io::engine::ContextFactory::create_libevent()},
      terminate_{(*context_).create_signal(*this, io::sys::Signal::Type::TERMINATE)},
      interrupt_{(*context_).create_signal(*this, io::sys::Signal::Type::INTERRUPT)},
      bus_error_{(*context_).create_signal(*this, io::sys::Signal::Type::BUS_ERROR)},
      timer_{(*context_).create_timer(*this, POLL_FREQUENCY)}, client_{create_client(*this, *context_)} {
  if (std::empty(arguments_))
    log::fatal("Unexpected"sv);
}

void Controller::dispatch() {
  (*client_).start();
  (*timer_).resume();
  (*context_).dispatch();
}

void Controller::shutdown() {
  (*client_).stop();
  (*context_).stop();
}

void Controller::operator()(io::sys::Signal::Event const &event) {
  log::warn("*** SIGNAL: {} ***"sv, magic_enum::enum_name(event.type));
  shutdown();
}

void Controller::operator()(io::sys::Timer::Event const &event) {
  (*client_).refresh(event.now);
  if (!(*client_).ready()) {
    (*client_).bump();
  }
}

void Controller::operator()(Trace<web::rest::Client::Connected> const &) {
  log::info<1>("Connected"sv);
}

void Controller::operator()(Trace<web::rest::Client::Disconnected> const &) {
  log::info<1>("Disconnected"sv);
  shutdown();
}

void Controller::operator()(Trace<web::rest::Client::Latency> const &) {
}

void Controller::operator()(
    Trace<web::rest::Response> const &, [[maybe_unused]] uint64_t request_id, [[maybe_unused]] uint64_t opaque) {
}

}  // namespace tester
}  // namespace cme
}  // namespace roq
