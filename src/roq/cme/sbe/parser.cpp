/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/cme/sbe/parser.hpp"

#include "roq/logging.hpp"

#include "roq/debug/hex/message.hpp"

#include "roq/cme/sbe/frame.hpp"
#include "roq/cme/sbe/utils.hpp"

#include <iostream>

using namespace std::literals;

namespace roq {
namespace cme {
namespace sbe {

bool Parser::dispatch(Handler &handler, std::span<std::byte const> const &buffer, TraceInfo const &trace_info) {
  auto result = true;
  if (Frame::parse(buffer, [&](auto &frame) {
        // log::debug("skip frame"sv);
        auto tmp = buffer.subspan(Frame::size());
        // sbe headers are not const-safe
        std::span message{reinterpret_cast<char *>(const_cast<std::byte *>(std::data(tmp))), std::size(tmp)};
        while (result) {
          // log::debug("message: size={}"sv, std::size(message));
          cme_mdp::MessageHeader header{std::data(message), std::size(message)};
          auto template_id = header.templateId();
          // log::debug("template_id={}"sv, template_id);
          switch (header.templateId()) {
            case cme_mdp::MDInstrumentDefinitionFX63::SBE_TEMPLATE_ID: {
              cme_mdp::MDInstrumentDefinitionFX63 value{std::data(message), std::size(message)};
              auto length = compute_length(value);
              value.sbeRewind();  // note! important
              create_trace_and_dispatch(handler, trace_info, value, frame);
              message = message.subspan(length);
              break;
            }
            default: {
              log::warn("Unknown template_id={}"sv, template_id);
              result = false;
              debug::hex::Message message{buffer};
              log::info<5>("DEBUG: {}"sv, message);
              return;
            }
          }
          if (std::empty(message))
            break;
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
