/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/cme/channel.hpp"

#include "roq/logging.hpp"

using namespace std::literals;

namespace roq {
namespace cme {

// === HELPER ===

namespace {
auto create_reorder_buffer(auto buffer_size, auto buffer_depth) -> Channel::ReorderBuffer {
  return {
      .last_sequence = {},
      .buffer = {buffer_size, buffer_depth},
  };
}
}  // namespace

// === IMPLEMENTATION ===

Channel::Channel(std::string_view const &channel_id, size_t buffer_size, size_t buffer_depth)
    : channel_id{channel_id}, instrument_definition{create_reorder_buffer(buffer_size, buffer_depth)},
      incremental{create_reorder_buffer(buffer_size, buffer_depth)},
      mbp_market_recovery{create_reorder_buffer(buffer_size, buffer_depth)}, mbo_market_recovery{create_reorder_buffer(
                                                                                 buffer_size, buffer_depth)} {
  log::info(R"(Create channel id="{}")"sv, channel_id);
}

}  // namespace cme
}  // namespace roq
