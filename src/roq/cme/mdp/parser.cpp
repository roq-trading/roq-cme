/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/cme/mdp/parser.hpp"

#include <cme_mdp/AdminLogout16.h>
#include <cme_mdp/CollateralMarketValue62.h>
#include <cme_mdp/QuoteRequest39.h>
#include <cme_mdp/SecurityStatusWorkup60.h>
#include <cme_mdp/SnapshotFullRefreshTCP61.h>
#include <cme_mdp/SnapshotFullRefreshTCPLongQty68.h>
#include <cme_mdp/SnapshotRefreshTopOrders59.h>

#include "roq/logging.hpp"

#include "roq/utils/byte_order.hpp"

#include "roq/utils/debug/hex/message.hpp"

#include "roq/cme/mdp/frame.hpp"
#include "roq/cme/mdp/utils.hpp"

#include <iostream>

using namespace std::literals;

namespace roq {
namespace cme {
namespace mdp {

namespace {
struct MessageSize final {
  using value_type = uint16_t;
  explicit MessageSize(auto &buffer) {
    if (std::size(buffer) < sizeof(value_type)) {
      throw RuntimeError{"Unexpected: buffer too small {}"sv, std::size(buffer)};
    }
    value_type tmp;
    std::memcpy(&tmp, std::data(buffer), sizeof(value_type));
    length = utils::little_endian_to_host(tmp);
    buffer = buffer.subspan(sizeof(value_type));
  }
  value_type length = {};
};
}  // namespace

bool Parser::dispatch(Handler &handler, std::span<std::byte const> const &buffer, TraceInfo const &trace_info) {
  auto result = true;
  if (Frame::parse(buffer, [&](auto &frame) {
        log::info<5>("frame={}"sv, frame);
        handler(frame);
        auto tmp = buffer.subspan(Frame::size());
        // SBE is not const-safe
        std::span message{reinterpret_cast<char *>(const_cast<std::byte *>(std::data(tmp))), std::size(tmp)};
        while (!std::empty(message)) {
          MessageSize message_size{message};
          cme_mdp::MessageHeader header{std::data(message), std::size(message)};
          message = message.subspan(cme_mdp::MessageHeader::encodedLength());
          auto block_length = header.blockLength();
          auto template_id = header.templateId();
          auto version = header.version();
          // log::debug("block_length={}, template={}, version={}"sv, block_length, template_id, version);
          auto length = message_size.length - (sizeof(MessageSize::value_type) + cme_mdp::MessageHeader::encodedLength());
          // log::debug("length={}"sv, length);
          assert(std::size(message) >= length);
          auto tmp = message.subspan(0, length);
          switch (template_id) {
              // admin
            case cme_mdp::AdminHeartbeat12::SBE_TEMPLATE_ID: {
              cme_mdp::AdminHeartbeat12 value{std::data(tmp), std::size(tmp), block_length, version};
              log::info<5>("admin_heartbeat_12={}"sv, value);
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_mdp::ChannelReset4::SBE_TEMPLATE_ID: {
              cme_mdp::ChannelReset4 value{std::data(tmp), std::size(tmp), block_length, version};
              log::info<5>("channel_reset_4={}"sv, value);
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_mdp::SecurityStatus30::SBE_TEMPLATE_ID: {
              cme_mdp::SecurityStatus30 value{std::data(tmp), std::size(tmp), block_length, version};
              log::info<5>("security_status_30={}"sv, value);
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
              // instrument definitions
            case cme_mdp::MDInstrumentDefinitionFuture54::SBE_TEMPLATE_ID: {
              cme_mdp::MDInstrumentDefinitionFuture54 value{std::data(tmp), std::size(tmp), block_length, version};
              log::info<5>("md_instrument_definition_future_54={}"sv, value);
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_mdp::MDInstrumentDefinitionOption55::SBE_TEMPLATE_ID: {
              cme_mdp::MDInstrumentDefinitionOption55 value{std::data(tmp), std::size(tmp), block_length, version};
              log::info<5>("md_instrument_definition_option_55={}"sv, value);
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_mdp::MDInstrumentDefinitionSpread56::SBE_TEMPLATE_ID: {
              cme_mdp::MDInstrumentDefinitionSpread56 value{std::data(tmp), std::size(tmp), block_length, version};
              log::info<5>("md_instrument_definition_spread_56={}"sv, value);
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_mdp::MDInstrumentDefinitionFixedIncome57::SBE_TEMPLATE_ID: {
              cme_mdp::MDInstrumentDefinitionFixedIncome57 value{std::data(tmp), std::size(tmp), block_length, version};
              log::info<5>("md_instrument_definition_fixed_income_57={}"sv, value);
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_mdp::MDInstrumentDefinitionRepo58::SBE_TEMPLATE_ID: {
              cme_mdp::MDInstrumentDefinitionRepo58 value{std::data(tmp), std::size(tmp), block_length, version};
              log::info<5>("md_instrument_definition_repo_58={}"sv, value);
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_mdp::MDInstrumentDefinitionFX63::SBE_TEMPLATE_ID: {
              cme_mdp::MDInstrumentDefinitionFX63 value{std::data(tmp), std::size(tmp), block_length, version};
              log::info<5>("md_instrument_definition_fx_63={}"sv, value);
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
              // market by price
            case cme_mdp::SnapshotFullRefresh52::SBE_TEMPLATE_ID: {
              cme_mdp::SnapshotFullRefresh52 value{std::data(tmp), std::size(tmp), block_length, version};
              log::info<5>("snapshot_full_refresh_52={}"sv, value);
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_mdp::SnapshotFullRefreshLongQty69::SBE_TEMPLATE_ID: {
              cme_mdp::SnapshotFullRefreshLongQty69 value{std::data(tmp), std::size(tmp), block_length, version};
              log::info<5>("snapshot_full_refresh_long_qty_69={}"sv, value);
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_mdp::MDIncrementalRefreshBook46::SBE_TEMPLATE_ID: {
              cme_mdp::MDIncrementalRefreshBook46 value{std::data(tmp), std::size(tmp), block_length, version};
              log::info<5>("md_incremental_refresh_book_46={}"sv, value);
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_mdp::MDIncrementalRefreshBookLongQty64::SBE_TEMPLATE_ID: {
              cme_mdp::MDIncrementalRefreshBookLongQty64 value{std::data(tmp), std::size(tmp), block_length, version};
              log::info<5>("md_incremental_refresh_book_long_qty_64={}"sv, value);
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
              // market by order
            case cme_mdp::SnapshotFullRefreshOrderBook53::SBE_TEMPLATE_ID: {
              cme_mdp::SnapshotFullRefreshOrderBook53 value{std::data(tmp), std::size(tmp), block_length, version};
              log::info<5>("snapshot_full_refresh_order_book_53={}"sv, value);
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_mdp::MDIncrementalRefreshOrderBook47::SBE_TEMPLATE_ID: {
              cme_mdp::MDIncrementalRefreshOrderBook47 value{std::data(tmp), std::size(tmp), block_length, version};
              log::info<5>("md_incremental_refresh_order_book_47={}"sv, value);
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
              // trade summary
            case cme_mdp::MDIncrementalRefreshTradeSummary48::SBE_TEMPLATE_ID: {
              cme_mdp::MDIncrementalRefreshTradeSummary48 value{std::data(tmp), std::size(tmp), block_length, version};
              log::info<5>("md_incremental_refresh_trade_summary_48={}"sv, value);
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_mdp::MDIncrementalRefreshTradeSummaryLongQty65::SBE_TEMPLATE_ID: {
              cme_mdp::MDIncrementalRefreshTradeSummaryLongQty65 value{std::data(tmp), std::size(tmp), block_length, version};
              log::info<5>("md_incremental_refresh_trade_summary_long_qty_65={}"sv, value);
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
              // statistics
            case cme_mdp::MDIncrementalRefreshDailyStatistics49::SBE_TEMPLATE_ID: {
              cme_mdp::MDIncrementalRefreshDailyStatistics49 value{std::data(tmp), std::size(tmp), block_length, version};
              log::info<5>("md_incremental_refresh_daily_statistics_49={}"sv, value);
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_mdp::MDIncrementalRefreshSessionStatistics51::SBE_TEMPLATE_ID: {
              cme_mdp::MDIncrementalRefreshSessionStatistics51 value{std::data(tmp), std::size(tmp), block_length, version};
              log::info<5>("md_incremental_refresh_session_statistics_51={}"sv, value);
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_mdp::MDIncrementalRefreshSessionStatisticsLongQty67::SBE_TEMPLATE_ID: {
              cme_mdp::MDIncrementalRefreshSessionStatisticsLongQty67 value{std::data(tmp), std::size(tmp), block_length, version};
              log::info<5>("md_incremental_refresh_session_statistics_long_qty_67={}"sv, value);
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_mdp::MDIncrementalRefreshVolume37::SBE_TEMPLATE_ID: {
              cme_mdp::MDIncrementalRefreshVolume37 value{std::data(tmp), std::size(tmp), block_length, version};
              log::info<5>("md_incremental_refresh_volume_37={}"sv, value);
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_mdp::MDIncrementalRefreshVolumeLongQty66::SBE_TEMPLATE_ID: {
              cme_mdp::MDIncrementalRefreshVolumeLongQty66 value{std::data(tmp), std::size(tmp), block_length, version};
              log::info<5>("md_incremental_refresh_volume_long_qty_66={}"sv, value);
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
              // misc
            case cme_mdp::MDIncrementalRefreshLimitsBanding50::SBE_TEMPLATE_ID: {
              cme_mdp::MDIncrementalRefreshLimitsBanding50 value{std::data(tmp), std::size(tmp), block_length, version};
              log::info<5>("md_incremental_refresh_limits_banding_50={}"sv, value);
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_mdp::QuoteRequest39::SBE_TEMPLATE_ID: {
              cme_mdp::QuoteRequest39 value{std::data(tmp), std::size(tmp), block_length, version};
              log::info<5>("quote_request_39={}"sv, value);
              create_trace_and_dispatch(handler, trace_info, value, frame);
              break;
            }
            case cme_mdp::AdminLogout16::SBE_TEMPLATE_ID:
            case cme_mdp::SnapshotRefreshTopOrders59::SBE_TEMPLATE_ID:
            case cme_mdp::SecurityStatusWorkup60::SBE_TEMPLATE_ID:
            case cme_mdp::SnapshotFullRefreshTCP61::SBE_TEMPLATE_ID:
            case cme_mdp::CollateralMarketValue62::SBE_TEMPLATE_ID:
            case cme_mdp::SnapshotFullRefreshTCPLongQty68::SBE_TEMPLATE_ID:
              log::warn("{}"sv, utils::debug::hex::Message{buffer});
              // don't parse / silent drop
              log::warn("Drop: template_id={}"sv, template_id);
              break;
            default: {
              log::warn("{}"sv, utils::debug::hex::Message{buffer});
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

}  // namespace mdp
}  // namespace cme
}  // namespace roq
