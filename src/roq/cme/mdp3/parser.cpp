/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/cme/mdp3/parser.hpp"

#include <cme_mdp3/AdminLogout16.h>
#include <cme_mdp3/CollateralMarketValue62.h>
#include <cme_mdp3/QuoteRequest39.h>
#include <cme_mdp3/SecurityStatusWorkup60.h>
#include <cme_mdp3/SnapshotFullRefreshTCP61.h>
#include <cme_mdp3/SnapshotFullRefreshTCPLongQty68.h>
#include <cme_mdp3/SnapshotRefreshTopOrders59.h>

#include "roq/logging.hpp"

#include "roq/debug/hex/message.hpp"

#include "roq/core/byte_order.hpp"

#include "roq/cme/mdp3/frame.hpp"
#include "roq/cme/mdp3/utils.hpp"

#include <iostream>

using namespace std::literals;

namespace roq {
namespace cme {
namespace mdp3 {

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
          cme_mdp3::MessageHeader header{std::data(message), std::size(message)};
          message = message.subspan(cme_mdp3::MessageHeader::encodedLength());
          auto block_length = header.blockLength();
          auto template_id = header.templateId();
          auto version = header.version();
          log::debug("block_length={}, template={}, version={}"sv, block_length, template_id, version);
          auto length =
              message_size.length - (sizeof(MessageSize::value_type) + cme_mdp3::MessageHeader::encodedLength());
          log::debug("length={}"sv, length);
          assert(std::size(message) >= length);
          auto tmp = message.subspan(0, length);
          switch (template_id) {
              // admin
            case cme_mdp3::AdminHeartbeat12::SBE_TEMPLATE_ID: {
              cme_mdp3::AdminHeartbeat12 value{std::data(tmp), std::size(tmp), block_length, version};
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_mdp3::ChannelReset4::SBE_TEMPLATE_ID: {
              cme_mdp3::ChannelReset4 value{std::data(tmp), std::size(tmp), block_length, version};
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_mdp3::SecurityStatus30::SBE_TEMPLATE_ID: {
              cme_mdp3::SecurityStatus30 value{std::data(tmp), std::size(tmp), block_length, version};
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
              // instrument definitions
            case cme_mdp3::MDInstrumentDefinitionFuture54::SBE_TEMPLATE_ID: {
              cme_mdp3::MDInstrumentDefinitionFuture54 value{std::data(tmp), std::size(tmp), block_length, version};
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_mdp3::MDInstrumentDefinitionOption55::SBE_TEMPLATE_ID: {
              cme_mdp3::MDInstrumentDefinitionOption55 value{std::data(tmp), std::size(tmp), block_length, version};
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_mdp3::MDInstrumentDefinitionSpread56::SBE_TEMPLATE_ID: {
              cme_mdp3::MDInstrumentDefinitionSpread56 value{std::data(tmp), std::size(tmp), block_length, version};
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_mdp3::MDInstrumentDefinitionFixedIncome57::SBE_TEMPLATE_ID: {
              cme_mdp3::MDInstrumentDefinitionFixedIncome57 value{
                  std::data(tmp), std::size(tmp), block_length, version};
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_mdp3::MDInstrumentDefinitionRepo58::SBE_TEMPLATE_ID: {
              cme_mdp3::MDInstrumentDefinitionRepo58 value{std::data(tmp), std::size(tmp), block_length, version};
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_mdp3::MDInstrumentDefinitionFX63::SBE_TEMPLATE_ID: {
              cme_mdp3::MDInstrumentDefinitionFX63 value{std::data(tmp), std::size(tmp), block_length, version};
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
              // market by price
            case cme_mdp3::SnapshotFullRefresh52::SBE_TEMPLATE_ID: {
              cme_mdp3::SnapshotFullRefresh52 value{std::data(tmp), std::size(tmp), block_length, version};
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_mdp3::SnapshotFullRefreshLongQty69::SBE_TEMPLATE_ID: {
              cme_mdp3::SnapshotFullRefreshLongQty69 value{std::data(tmp), std::size(tmp), block_length, version};
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_mdp3::MDIncrementalRefreshBook46::SBE_TEMPLATE_ID: {
              cme_mdp3::MDIncrementalRefreshBook46 value{std::data(tmp), std::size(tmp), block_length, version};
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_mdp3::MDIncrementalRefreshBookLongQty64::SBE_TEMPLATE_ID: {
              cme_mdp3::MDIncrementalRefreshBookLongQty64 value{std::data(tmp), std::size(tmp), block_length, version};
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
              // market by order
            case cme_mdp3::SnapshotFullRefreshOrderBook53::SBE_TEMPLATE_ID: {
              cme_mdp3::SnapshotFullRefreshOrderBook53 value{std::data(tmp), std::size(tmp), block_length, version};
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_mdp3::MDIncrementalRefreshOrderBook47::SBE_TEMPLATE_ID: {
              cme_mdp3::MDIncrementalRefreshOrderBook47 value{std::data(tmp), std::size(tmp), block_length, version};
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
              // trade summary
            case cme_mdp3::MDIncrementalRefreshTradeSummary48::SBE_TEMPLATE_ID: {
              cme_mdp3::MDIncrementalRefreshTradeSummary48 value{std::data(tmp), std::size(tmp), block_length, version};
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_mdp3::MDIncrementalRefreshTradeSummaryLongQty65::SBE_TEMPLATE_ID: {
              cme_mdp3::MDIncrementalRefreshTradeSummaryLongQty65 value{
                  std::data(tmp), std::size(tmp), block_length, version};
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
              // statistics
            case cme_mdp3::MDIncrementalRefreshDailyStatistics49::SBE_TEMPLATE_ID: {
              cme_mdp3::MDIncrementalRefreshDailyStatistics49 value{
                  std::data(tmp), std::size(tmp), block_length, version};
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_mdp3::MDIncrementalRefreshSessionStatistics51::SBE_TEMPLATE_ID: {
              cme_mdp3::MDIncrementalRefreshSessionStatistics51 value{
                  std::data(tmp), std::size(tmp), block_length, version};
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_mdp3::MDIncrementalRefreshSessionStatisticsLongQty67::SBE_TEMPLATE_ID: {
              cme_mdp3::MDIncrementalRefreshSessionStatisticsLongQty67 value{
                  std::data(tmp), std::size(tmp), block_length, version};
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_mdp3::MDIncrementalRefreshVolume37::SBE_TEMPLATE_ID: {
              cme_mdp3::MDIncrementalRefreshVolume37 value{std::data(tmp), std::size(tmp), block_length, version};
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_mdp3::MDIncrementalRefreshVolumeLongQty66::SBE_TEMPLATE_ID: {
              cme_mdp3::MDIncrementalRefreshVolumeLongQty66 value{
                  std::data(tmp), std::size(tmp), block_length, version};
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
              // misc
            case cme_mdp3::MDIncrementalRefreshLimitsBanding50::SBE_TEMPLATE_ID: {
              cme_mdp3::MDIncrementalRefreshLimitsBanding50 value{
                  std::data(tmp), std::size(tmp), block_length, version};
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_mdp3::QuoteRequest39::SBE_TEMPLATE_ID: {
              cme_mdp3::QuoteRequest39 value{std::data(tmp), std::size(tmp), block_length, version};
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_mdp3::AdminLogout16::SBE_TEMPLATE_ID:
            case cme_mdp3::SnapshotRefreshTopOrders59::SBE_TEMPLATE_ID:
            case cme_mdp3::SecurityStatusWorkup60::SBE_TEMPLATE_ID:
            case cme_mdp3::SnapshotFullRefreshTCP61::SBE_TEMPLATE_ID:
            case cme_mdp3::CollateralMarketValue62::SBE_TEMPLATE_ID:
            case cme_mdp3::SnapshotFullRefreshTCPLongQty68::SBE_TEMPLATE_ID:
              log::warn("{}"sv, debug::hex::Message{buffer});
              // don't parse / silent drop
              log::warn("Drop: template_id={}"sv, template_id);
              break;
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

}  // namespace mdp3
}  // namespace cme
}  // namespace roq
