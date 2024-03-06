/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/cme/market_data/channel.hpp"

#include "roq/logging.hpp"

using namespace std::literals;

namespace roq {
namespace cme {
namespace market_data {

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

Channel::Channel(uint16_t channel_id, size_t buffer_size, size_t buffer_depth)
    : channel_id{channel_id}, incremental{create_reorder_buffer(buffer_size, buffer_depth)} {
  log::info(R"(Create channel id="{}")"sv, channel_id);
}

std::string Channel::get_channel_name(std::string_view const &prefix, Priority priority) const {
  auto postfix = [&]() {
    switch (priority) {
      using enum Priority;
      case UNDEFINED:
        return std::string_view{};
      case PRIMARY:
        return "A"sv;
      case SECONDARY:
        return "B"sv;
    }
    return "?"sv;
  }();
  return fmt::format("{}{}{}"sv, prefix, channel_id, postfix);
}

}  // namespace market_data
}  // namespace cme
}  // namespace roq
