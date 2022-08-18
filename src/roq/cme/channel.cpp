/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/cme/channel.hpp"

#include "roq/logging.hpp"

using namespace std::literals;

namespace roq {
namespace cme {

Channel::Channel(std::string_view const &channel_id, size_t buffer_size, size_t buffer_depth)
    : channel_id(channel_id), buffer(buffer_size, buffer_depth) {
  log::info(R"(Create channel id="{}")"sv, channel_id);
}

}  // namespace cme
}  // namespace roq
