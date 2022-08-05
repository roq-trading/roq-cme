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
      throw RuntimeError("Unexpected: buffer too small {}"sv, std::size(buffer));
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
        // SBE is not const-safe
        std::span message{reinterpret_cast<char *>(const_cast<std::byte *>(std::data(tmp))), std::size(tmp)};
        while (!std::empty(message)) {
          MessageSize message_size(message);
          cme_mdp::MessageHeader header{std::data(message), std::size(message)};
          message = message.subspan(cme_mdp::MessageHeader::encodedLength());
          auto block_length = header.blockLength();
          auto template_id = header.templateId();
          auto version = header.version();
          log::debug("block_length={}, template={}, version={}"sv, block_length, template_id, version);
          auto length =
              message_size.length - (sizeof(MessageSize::value_type) + cme_mdp::MessageHeader::encodedLength());
          log::debug("length={}"sv, length);
          assert(std::size(message) >= length);
          auto tmp = message.subspan(0, length);
          switch (template_id) {
              // - MDInstrumentDefinition
            case cme_mdp::MDInstrumentDefinitionFuture54::SBE_TEMPLATE_ID: {
              cme_mdp::MDInstrumentDefinitionFuture54 value{std::data(tmp), std::size(tmp), block_length, version};
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_mdp::MDInstrumentDefinitionOption55::SBE_TEMPLATE_ID: {
              cme_mdp::MDInstrumentDefinitionOption55 value{std::data(tmp), std::size(tmp), block_length, version};
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_mdp::MDInstrumentDefinitionSpread56::SBE_TEMPLATE_ID: {
              cme_mdp::MDInstrumentDefinitionSpread56 value{std::data(tmp), std::size(tmp), block_length, version};
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_mdp::MDInstrumentDefinitionFixedIncome57::SBE_TEMPLATE_ID: {
              cme_mdp::MDInstrumentDefinitionFixedIncome57 value{std::data(tmp), std::size(tmp), block_length, version};
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_mdp::MDInstrumentDefinitionRepo58::SBE_TEMPLATE_ID: {
              cme_mdp::MDInstrumentDefinitionRepo58 value{std::data(tmp), std::size(tmp), block_length, version};
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_mdp::MDInstrumentDefinitionFX63::SBE_TEMPLATE_ID: {
              cme_mdp::MDInstrumentDefinitionFX63 value{std::data(tmp), std::size(tmp), block_length, version};
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
              // - SnapshotFullRefresh
            case cme_mdp::SnapshotFullRefresh52::SBE_TEMPLATE_ID: {
              cme_mdp::SnapshotFullRefresh52 value{std::data(tmp), std::size(tmp), block_length, version};
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_mdp::SnapshotFullRefreshOrderBook53::SBE_TEMPLATE_ID: {
              cme_mdp::SnapshotFullRefreshOrderBook53 value{std::data(tmp), std::size(tmp), block_length, version};
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_mdp::SnapshotFullRefreshLongQty69::SBE_TEMPLATE_ID: {
              cme_mdp::SnapshotFullRefreshLongQty69 value{std::data(tmp), std::size(tmp), block_length, version};
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
              // - MDIncrementalRefresh
            case cme_mdp::MDIncrementalRefreshVolume37::SBE_TEMPLATE_ID: {
              cme_mdp::MDIncrementalRefreshVolume37 value{std::data(tmp), std::size(tmp), block_length, version};
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_mdp::MDIncrementalRefreshBook46::SBE_TEMPLATE_ID: {
              cme_mdp::MDIncrementalRefreshBook46 value{std::data(tmp), std::size(tmp), block_length, version};
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_mdp::MDIncrementalRefreshOrderBook47::SBE_TEMPLATE_ID: {
              cme_mdp::MDIncrementalRefreshOrderBook47 value{std::data(tmp), std::size(tmp), block_length, version};
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_mdp::MDIncrementalRefreshTradeSummary48::SBE_TEMPLATE_ID: {
              cme_mdp::MDIncrementalRefreshTradeSummary48 value{std::data(tmp), std::size(tmp), block_length, version};
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_mdp::MDIncrementalRefreshDailyStatistics49::SBE_TEMPLATE_ID: {
              cme_mdp::MDIncrementalRefreshDailyStatistics49 value{
                  std::data(tmp), std::size(tmp), block_length, version};
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_mdp::MDIncrementalRefreshLimitsBanding50::SBE_TEMPLATE_ID: {
              cme_mdp::MDIncrementalRefreshLimitsBanding50 value{std::data(tmp), std::size(tmp), block_length, version};
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_mdp::MDIncrementalRefreshSessionStatistics51::SBE_TEMPLATE_ID: {
              cme_mdp::MDIncrementalRefreshSessionStatistics51 value{
                  std::data(tmp), std::size(tmp), block_length, version};
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_mdp::MDIncrementalRefreshBookLongQty64::SBE_TEMPLATE_ID: {
              cme_mdp::MDIncrementalRefreshBookLongQty64 value{std::data(tmp), std::size(tmp), block_length, version};
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_mdp::MDIncrementalRefreshTradeSummaryLongQty65::SBE_TEMPLATE_ID: {
              cme_mdp::MDIncrementalRefreshTradeSummaryLongQty65 value{
                  std::data(tmp), std::size(tmp), block_length, version};
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_mdp::MDIncrementalRefreshVolumeLongQty66::SBE_TEMPLATE_ID: {
              cme_mdp::MDIncrementalRefreshVolumeLongQty66 value{std::data(tmp), std::size(tmp), block_length, version};
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_mdp::MDIncrementalRefreshSessionStatisticsLongQty67::SBE_TEMPLATE_ID: {
              cme_mdp::MDIncrementalRefreshSessionStatisticsLongQty67 value{
                  std::data(tmp), std::size(tmp), block_length, version};
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            default: {
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

}  // namespace sbe
}  // namespace cme
}  // namespace roq
