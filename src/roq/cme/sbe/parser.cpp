/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/cme/sbe/parser.hpp"

#include "roq/logging.hpp"

#include "roq/debug/hex/message.hpp"

#include "roq/core/byte_order.hpp"

#include "roq/cme/sbe/frame.hpp"
#include "roq/cme/sbe/utils.hpp"

#include <iostream>

using namespace std::literals;

namespace roq {
namespace cme {
namespace sbe {

namespace {
struct MessageSize final {
  using value_type = uint16_t;
  explicit MessageSize(auto &buffer) {
    if (std::size(buffer) < sizeof(value_type))
      throw RuntimeError("Unexpected: buffer too small"sv);
    value_type tmp;
    std::memcpy(&tmp, std::data(buffer), sizeof(value_type));
    length = core::little_endian_to_host(tmp);
    buffer = buffer.subspan(sizeof(value_type));
  }
  value_type length = {};
};
}  // namespace

bool Parser::dispatch(Handler &handler, std::span<std::byte const> const &buffer, TraceInfo const &trace_info) {
  auto result = true;
  if (Frame::parse(buffer, [&](auto &frame) {
        auto tmp = buffer.subspan(Frame::size());
        // SBE not const-safe
        std::span message{reinterpret_cast<char *>(const_cast<std::byte *>(std::data(tmp))), std::size(tmp)};
        while (!std::empty(message)) {
          MessageSize message_size(message);
          cme_mdp::MessageHeader header{std::data(message), std::size(message)};
          message = message.subspan(cme_mdp::MessageHeader::encodedLength());
          log::info("template={}"sv, header.templateId());
          auto length =
              message_size.length - (sizeof(MessageSize::value_type) + cme_mdp::MessageHeader::encodedLength());
          log::info("length={}"sv, length);
          assert(std::size(message) >= length);
          auto tmp = message.subspan(0, length);
          switch (header.templateId()) {
            case cme_mdp::MDIncrementalRefreshBook46::SBE_TEMPLATE_ID: {
              cme_mdp::MDIncrementalRefreshBook46 value{std::data(tmp), std::size(tmp)};
              create_trace_and_dispatch(handler, trace_info, value, frame);
            } break;
          }
          message = message.subspan(length);
        }
      })) {
  } else {
    return false;
  }
  return result;
}

}  // namespace sbe
}  // namespace cme
}  // namespace roq
