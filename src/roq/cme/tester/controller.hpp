/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <vector>

#include "roq/io/buffer.hpp"
#include "roq/io/context.hpp"
#include "roq/io/net/udp/receiver.hpp"
#include "roq/io/sys/signal.hpp"

namespace roq {
namespace cme {
namespace tester {

struct Controller final : public io::sys::Signal::Handler, public io::net::udp::Receiver::Handler {
  Controller();

  Controller(Controller const &) = delete;
  Controller(Controller &&) = delete;

  void dispatch();

 protected:
  void operator()(io::sys::Signal::Event const &) override;

  void operator()(io::net::udp::Receiver::Read const &) override;
  void operator()(io::net::udp::Receiver::Error const &) override;

 private:
  std::unique_ptr<io::Context> const context_;
  std::unique_ptr<io::sys::Signal> const terminate_;
  std::unique_ptr<io::sys::Signal> const interrupt_;
  std::unique_ptr<io::sys::Signal> const bus_error_;
  std::unique_ptr<io::net::udp::Receiver> const receiver_;
  std::vector<std::byte> buffer_;
};

}  // namespace tester
}  // namespace cme
}  // namespace roq
