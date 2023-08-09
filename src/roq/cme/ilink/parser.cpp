/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/cme/ilink/parser.hpp"

#include "roq/logging.hpp"

#include "roq/debug/hex/message.hpp"

#include "roq/core/byte_order.hpp"

#include "roq/cme/ilink/utils.hpp"

#include <iostream>

using namespace std::literals;

namespace roq {
namespace cme {
namespace ilink {

namespace {
template <typename Callback>
size_t parse_helper(auto &buffer, Callback callback) {
  if (std::size(buffer) < 4)
    return false;
  uint16_t encoding;
  std::memcpy(&encoding, &buffer[2], sizeof(encoding));
  if (encoding != 0xCAFE)
    log::fatal("Unexpected"sv);
  uint16_t tmp;
  std::memcpy(&tmp, &buffer[0], sizeof(tmp));
  auto length = core::little_endian_to_host(tmp);
  if (std::size(buffer) < length)
    return 0;
  auto message = buffer.subspan(4, length - 4);
  callback(message);
  return length;
}
}  // namespace

size_t Parser::dispatch(Handler &handler, std::span<std::byte const> const &buffer, TraceInfo const &trace_info) {
  log::error(R"(DEBUG Received message="{}")"sv, debug::hex::Message{buffer});
  log::info<5>(R"(Received message="{}")"sv, debug::hex::Message{buffer});
  return parse_helper(buffer, [&](auto &message_2) {
    // SBE is not const-safe
    std::span message{reinterpret_cast<char *>(const_cast<std::byte *>(std::data(message_2))), std::size(message_2)};
    while (!std::empty(message)) {
      cme_ilink::MessageHeader header{std::data(message), std::size(message)};
      message = message.subspan(cme_ilink::MessageHeader::encodedLength());
      auto block_length = header.blockLength();
      auto template_id = header.templateId();
      auto version = header.version();
      // log::info("DEBUG block_length={}, template={}, version={}"sv, block_length, template_id, version);
      auto length = std::size(message);
      // log::info("DEBUG length={}"sv, length);
      assert(std::size(message) >= length);
      auto tmp = message.subspan(0, length);
      switch (template_id) {
          // session
        case cme_ilink::NegotiationResponse501::SBE_TEMPLATE_ID: {
          cme_ilink::NegotiationResponse501 value{std::data(tmp), std::size(tmp), block_length, version};
          // log::info("DEBUG {}"sv, value);
          create_trace_and_dispatch(handler, trace_info, value);
          break;
        }
        case cme_ilink::NegotiationReject502::SBE_TEMPLATE_ID: {
          cme_ilink::NegotiationReject502 value{std::data(tmp), std::size(tmp), block_length, version};
          // log::info("DEBUG {}"sv, value);
          create_trace_and_dispatch(handler, trace_info, value);
          break;
        }
        case cme_ilink::EstablishmentAck504::SBE_TEMPLATE_ID: {
          cme_ilink::EstablishmentAck504 value{std::data(tmp), std::size(tmp), block_length, version};
          // log::info("DEBUG {}"sv, value);
          create_trace_and_dispatch(handler, trace_info, value);
          break;
        }
        case cme_ilink::EstablishmentReject505::SBE_TEMPLATE_ID: {
          cme_ilink::EstablishmentReject505 value{std::data(tmp), std::size(tmp), block_length, version};
          // log::info("DEBUG {}"sv, value);
          create_trace_and_dispatch(handler, trace_info, value);
          break;
        }
        case cme_ilink::Sequence506::SBE_TEMPLATE_ID: {
          cme_ilink::Sequence506 value{std::data(tmp), std::size(tmp), block_length, version};
          // log::info("DEBUG {}"sv, value);
          create_trace_and_dispatch(handler, trace_info, value);
          break;
        }
        case cme_ilink::Terminate507::SBE_TEMPLATE_ID: {
          cme_ilink::Terminate507 value{std::data(tmp), std::size(tmp), block_length, version};
          // log::info("DEBUG {}"sv, value);
          create_trace_and_dispatch(handler, trace_info, value);
          break;
        }
        case cme_ilink::Retransmission509::SBE_TEMPLATE_ID: {
          cme_ilink::Retransmission509 value{std::data(tmp), std::size(tmp), block_length, version};
          // log::info("DEBUG {}"sv, value);
          create_trace_and_dispatch(handler, trace_info, value);
          break;
        }
        case cme_ilink::RetransmitReject510::SBE_TEMPLATE_ID: {
          cme_ilink::RetransmitReject510 value{std::data(tmp), std::size(tmp), block_length, version};
          // log::info("DEBUG {}"sv, value);
          create_trace_and_dispatch(handler, trace_info, value);
          break;
        }
        case cme_ilink::NotApplied513::SBE_TEMPLATE_ID: {
          cme_ilink::NotApplied513 value{std::data(tmp), std::size(tmp), block_length, version};
          // log::info("DEBUG {}"sv, value);
          create_trace_and_dispatch(handler, trace_info, value);
          break;
        }
          // business
        case cme_ilink::PartyDetailsDefinitionRequestAck519::SBE_TEMPLATE_ID: {
          cme_ilink::PartyDetailsDefinitionRequestAck519 value{std::data(tmp), std::size(tmp), block_length, version};
          // log::info("DEBUG {}"sv, value);
          create_trace_and_dispatch(handler, trace_info, value);
          break;
        }
        case cme_ilink::BusinessReject521::SBE_TEMPLATE_ID: {
          cme_ilink::BusinessReject521 value{std::data(tmp), std::size(tmp), block_length, version};
          // log::info("DEBUG {}"sv, value);
          create_trace_and_dispatch(handler, trace_info, value);
          break;
        }
          // execution report
        case cme_ilink::ExecutionReportNew522::SBE_TEMPLATE_ID: {
          cme_ilink::ExecutionReportNew522 value{std::data(tmp), std::size(tmp), block_length, version};
          // log::info("DEBUG {}"sv, value);
          create_trace_and_dispatch(handler, trace_info, value);
          break;
        }
        case cme_ilink::ExecutionReportReject523::SBE_TEMPLATE_ID: {
          cme_ilink::ExecutionReportReject523 value{std::data(tmp), std::size(tmp), block_length, version};
          // log::info("DEBUG {}"sv, value);
          create_trace_and_dispatch(handler, trace_info, value);
          break;
        }
        case cme_ilink::ExecutionReportTradeOutright525::SBE_TEMPLATE_ID: {
          cme_ilink::ExecutionReportTradeOutright525 value{std::data(tmp), std::size(tmp), block_length, version};
          // log::info("DEBUG {}"sv, value);
          create_trace_and_dispatch(handler, trace_info, value);
          break;
        }
        case cme_ilink::ExecutionReportTradeSpread526::SBE_TEMPLATE_ID: {
          cme_ilink::ExecutionReportTradeSpread526 value{std::data(tmp), std::size(tmp), block_length, version};
          // log::info("DEBUG {}"sv, value);
          create_trace_and_dispatch(handler, trace_info, value);
          break;
        }
        case cme_ilink::ExecutionReportTradeSpreadLeg527::SBE_TEMPLATE_ID: {
          cme_ilink::ExecutionReportTradeSpreadLeg527 value{std::data(tmp), std::size(tmp), block_length, version};
          // log::info("DEBUG {}"sv, value);
          create_trace_and_dispatch(handler, trace_info, value);
          break;
        }
        case cme_ilink::ExecutionReportModify531::SBE_TEMPLATE_ID: {
          cme_ilink::ExecutionReportModify531 value{std::data(tmp), std::size(tmp), block_length, version};
          // log::info("DEBUG {}"sv, value);
          create_trace_and_dispatch(handler, trace_info, value);
          break;
        }
        case cme_ilink::ExecutionReportStatus532::SBE_TEMPLATE_ID: {
          cme_ilink::ExecutionReportStatus532 value{std::data(tmp), std::size(tmp), block_length, version};
          // log::info("DEBUG {}"sv, value);
          create_trace_and_dispatch(handler, trace_info, value);
          break;
        }
        case cme_ilink::ExecutionReportCancel534::SBE_TEMPLATE_ID: {
          cme_ilink::ExecutionReportCancel534 value{std::data(tmp), std::size(tmp), block_length, version};
          // log::info("DEBUG {}"sv, value);
          create_trace_and_dispatch(handler, trace_info, value);
          break;
        }
        case cme_ilink::ExecutionReportPendingCancel564::SBE_TEMPLATE_ID: {
          cme_ilink::ExecutionReportPendingCancel564 value{std::data(tmp), std::size(tmp), block_length, version};
          // log::info("DEBUG {}"sv, value);
          create_trace_and_dispatch(handler, trace_info, value);
          break;
        }
        case cme_ilink::ExecutionReportPendingReplace565::SBE_TEMPLATE_ID: {
          cme_ilink::ExecutionReportPendingReplace565 value{std::data(tmp), std::size(tmp), block_length, version};
          // log::info("DEBUG {}"sv, value);
          create_trace_and_dispatch(handler, trace_info, value);
          break;
        }
          // order
        case cme_ilink::OrderCancelReject535::SBE_TEMPLATE_ID: {
          cme_ilink::OrderCancelReject535 value{std::data(tmp), std::size(tmp), block_length, version};
          // log::info("DEBUG {}"sv, value);
          create_trace_and_dispatch(handler, trace_info, value);
          break;
        }
        case cme_ilink::OrderCancelReplaceReject536::SBE_TEMPLATE_ID: {
          cme_ilink::OrderCancelReplaceReject536 value{std::data(tmp), std::size(tmp), block_length, version};
          // log::info("DEBUG {}"sv, value);
          create_trace_and_dispatch(handler, trace_info, value);
          break;
        }
          // order mass action
        case cme_ilink::OrderMassActionReport562::SBE_TEMPLATE_ID: {
          cme_ilink::OrderMassActionReport562 value{std::data(tmp), std::size(tmp), block_length, version};
          // log::info("DEBUG {}"sv, value);
          create_trace_and_dispatch(handler, trace_info, value);
          break;
        }
          // security definition
        case cme_ilink::SecurityDefinitionResponse561::SBE_TEMPLATE_ID: {
          cme_ilink::SecurityDefinitionResponse561 value{std::data(tmp), std::size(tmp), block_length, version};
          // log::info("DEBUG {}"sv, value);
          create_trace_and_dispatch(handler, trace_info, value);
          break;
        }
        default: {
          log::warn("{}"sv, debug::hex::Message{buffer});
          log::warn("Unexpected: template_id={}"sv, template_id);
          // result = false; // XXX HANS ???
          return;
        }
      }
      message = message.subspan(length);
    }
  });
}

}  // namespace ilink
}  // namespace cme
}  // namespace roq
