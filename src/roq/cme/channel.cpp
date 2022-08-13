/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/cme/channel.hpp"

using namespace std::literals;

namespace roq {
namespace cme {

namespace {
size_t BUFFER_SIZE = 4096;
size_t REPEAT = 16;
}  // namespace

Channel::Channel(std::string_view const &channel_id) : channel_id(channel_id), buffer(BUFFER_SIZE, REPEAT) {
}

}  // namespace cme
}  // namespace roq
