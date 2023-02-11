/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <string>
#include <vector>

#include "roq/io/context.hpp"

#include "roq/web/rest/client.hpp"

namespace roq {
namespace cme {
namespace tester {

struct Controller final : public io::sys::Signal::Handler,
                          public io::sys::Timer::Handler,
                          public web::rest::Client::Handler {
  explicit Controller(std::vector<std::string> const &arguments);

  Controller(Controller const &) = delete;
  Controller(Controller &&) = delete;

  void dispatch();

 protected:
  void shutdown();

  void operator()(io::sys::Signal::Event const &) override;
  void operator()(io::sys::Timer::Event const &) override;

  void operator()(Trace<web::rest::Client::Connected> const &) override;
  void operator()(Trace<web::rest::Client::Disconnected> const &) override;
  void operator()(Trace<web::rest::Client::Latency> const &) override;
  void operator()(Trace<web::rest::Response> const &, uint64_t request_id, uint64_t opaque) override;

 private:
  const std::vector<std::string> arguments_;
  std::unique_ptr<io::Context> context_;
  std::unique_ptr<io::sys::Signal> terminate_;
  std::unique_ptr<io::sys::Signal> interrupt_;
  std::unique_ptr<io::sys::Signal> bus_error_;
  std::unique_ptr<io::sys::Timer> timer_;
  std::unique_ptr<web::rest::Client> client_;
};

}  // namespace tester
}  // namespace cme
}  // namespace roq
