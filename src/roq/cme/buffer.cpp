/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/cme/buffer.hpp"

#include "roq/logging.hpp"

using namespace std::literals;

namespace roq {
namespace cme {

Buffer::Buffer(size_t buffer_size, size_t repeat) : buffer_size_(buffer_size), buffer_(buffer_size * repeat) {
  for (size_t i = 0; i < repeat; ++i)
    available_.emplace_back(i);
}

void Buffer::clear() {
  taken_.clear();
  available_.clear();
  auto repeat = std::size(buffer_) / buffer_size_;
  for (size_t i = 0; i < repeat; ++i)
    available_.emplace_back(i);
}

}  // namespace cme
}  // namespace roq
