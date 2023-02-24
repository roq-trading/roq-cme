/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/cme/ilink3/parser.hpp"

#include "roq/logging.hpp"

#include "roq/debug/hex/message.hpp"

#include "roq/core/byte_order.hpp"

#include "roq/cme/ilink3/utils.hpp"

#include <iostream>

using namespace std::literals;

namespace roq {
namespace cme {
namespace ilink3 {

namespace {
struct MessageSize final {
  using value_type = uint16_t;
  explicit MessageSize(auto &buffer) {
    if (std::size(buffer) < sizeof(value_type))
      throw RuntimeError{"Unexpected: buffer too small {}"sv, std::size(buffer)};
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
        handler(frame);
        auto tmp = buffer.subspan(Frame::size());
        // SBE is not const-safe
        std::span message{reinterpret_cast<char *>(const_cast<std::byte *>(std::data(tmp))), std::size(tmp)};
        while (!std::empty(message)) {
          MessageSize message_size(message);
          cme_ilink3::MessageHeader header{std::data(message), std::size(message)};
          message = message.subspan(cme_ilink3::MessageHeader::encodedLength());
          auto block_length = header.blockLength();
          auto template_id = header.templateId();
          auto version = header.version();
          log::debug("block_length={}, template={}, version={}"sv, block_length, template_id, version);
          auto length =
              message_size.length - (sizeof(MessageSize::value_type) + cme_ilink3::MessageHeader::encodedLength());
          log::debug("length={}"sv, length);
          assert(std::size(message) >= length);
          auto tmp = message.subspan(0, length);
          switch (template_id) {
              // session
            case cme_ilink3::NegotiationResponse501::SBE_TEMPLATE_ID: {
              cme_ilink3::NegotiationResponse501 value{std::data(tmp), std::size(tmp), block_length, version};
              log::debug("{}"sv, value);
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_ilink3::NegotiationReject502::SBE_TEMPLATE_ID: {
              cme_ilink3::NegotiationReject502 value{std::data(tmp), std::size(tmp), block_length, version};
              log::debug("{}"sv, value);
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_ilink3::EstablishmentAck504::SBE_TEMPLATE_ID: {
              cme_ilink3::EstablishmentAck504 value{std::data(tmp), std::size(tmp), block_length, version};
              log::debug("{}"sv, value);
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_ilink3::EstablishmentReject505::SBE_TEMPLATE_ID: {
              cme_ilink3::EstablishmentReject505 value{std::data(tmp), std::size(tmp), block_length, version};
              log::debug("{}"sv, value);
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_ilink3::Sequence506::SBE_TEMPLATE_ID: {
              cme_ilink3::Sequence506 value{std::data(tmp), std::size(tmp), block_length, version};
              log::debug("{}"sv, value);
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_ilink3::Terminate507::SBE_TEMPLATE_ID: {
              cme_ilink3::Terminate507 value{std::data(tmp), std::size(tmp), block_length, version};
              log::debug("{}"sv, value);
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_ilink3::Retransmission509::SBE_TEMPLATE_ID: {
              cme_ilink3::Retransmission509 value{std::data(tmp), std::size(tmp), block_length, version};
              log::debug("{}"sv, value);
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_ilink3::RetransmitReject510::SBE_TEMPLATE_ID: {
              cme_ilink3::RetransmitReject510 value{std::data(tmp), std::size(tmp), block_length, version};
              log::debug("{}"sv, value);
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_ilink3::NotApplied513::SBE_TEMPLATE_ID: {
              cme_ilink3::NotApplied513 value{std::data(tmp), std::size(tmp), block_length, version};
              log::debug("{}"sv, value);
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
              // business
            case cme_ilink3::BusinessReject521::SBE_TEMPLATE_ID: {
              cme_ilink3::BusinessReject521 value{std::data(tmp), std::size(tmp), block_length, version};
              log::debug("{}"sv, value);
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
              // execution report
            case cme_ilink3::ExecutionReportNew522::SBE_TEMPLATE_ID: {
              cme_ilink3::ExecutionReportNew522 value{std::data(tmp), std::size(tmp), block_length, version};
              log::debug("{}"sv, value);
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_ilink3::ExecutionReportReject523::SBE_TEMPLATE_ID: {
              cme_ilink3::ExecutionReportReject523 value{std::data(tmp), std::size(tmp), block_length, version};
              log::debug("{}"sv, value);
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_ilink3::ExecutionReportTradeOutright525::SBE_TEMPLATE_ID: {
              cme_ilink3::ExecutionReportTradeOutright525 value{std::data(tmp), std::size(tmp), block_length, version};
              log::debug("{}"sv, value);
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_ilink3::ExecutionReportTradeSpread526::SBE_TEMPLATE_ID: {
              cme_ilink3::ExecutionReportTradeSpread526 value{std::data(tmp), std::size(tmp), block_length, version};
              log::debug("{}"sv, value);
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_ilink3::ExecutionReportTradeSpreadLeg527::SBE_TEMPLATE_ID: {
              cme_ilink3::ExecutionReportTradeSpreadLeg527 value{std::data(tmp), std::size(tmp), block_length, version};
              log::debug("{}"sv, value);
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_ilink3::ExecutionReportModify531::SBE_TEMPLATE_ID: {
              cme_ilink3::ExecutionReportModify531 value{std::data(tmp), std::size(tmp), block_length, version};
              log::debug("{}"sv, value);
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_ilink3::ExecutionReportStatus532::SBE_TEMPLATE_ID: {
              cme_ilink3::ExecutionReportStatus532 value{std::data(tmp), std::size(tmp), block_length, version};
              log::debug("{}"sv, value);
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_ilink3::ExecutionReportCancel534::SBE_TEMPLATE_ID: {
              cme_ilink3::ExecutionReportCancel534 value{std::data(tmp), std::size(tmp), block_length, version};
              log::debug("{}"sv, value);
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_ilink3::ExecutionReportPendingCancel564::SBE_TEMPLATE_ID: {
              cme_ilink3::ExecutionReportPendingCancel564 value{std::data(tmp), std::size(tmp), block_length, version};
              log::debug("{}"sv, value);
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_ilink3::ExecutionReportPendingReplace565::SBE_TEMPLATE_ID: {
              cme_ilink3::ExecutionReportPendingReplace565 value{std::data(tmp), std::size(tmp), block_length, version};
              log::debug("{}"sv, value);
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
              // order
            case cme_ilink3::OrderCancelReject535::SBE_TEMPLATE_ID: {
              cme_ilink3::OrderCancelReject535 value{std::data(tmp), std::size(tmp), block_length, version};
              log::debug("{}"sv, value);
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_ilink3::OrderCancelReplaceReject536::SBE_TEMPLATE_ID: {
              cme_ilink3::OrderCancelReplaceReject536 value{std::data(tmp), std::size(tmp), block_length, version};
              log::debug("{}"sv, value);
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
              // security definition
            case cme_ilink3::SecurityDefinitionResponse561::SBE_TEMPLATE_ID: {
              cme_ilink3::SecurityDefinitionResponse561 value{std::data(tmp), std::size(tmp), block_length, version};
              log::debug("{}"sv, value);
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            default: {
              log::warn("{}"sv, debug::hex::Message{buffer});
              log::warn("Unexpected: template_id={}"sv, template_id);
              result = false;
              return;
            }
          }
          message = message.subspan(length);
        }
      })) {
  } else {
    result = false;
  }
  return result;
}

}  // namespace ilink3
}  // namespace cme
}  // namespace roq
