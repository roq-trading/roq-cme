/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <cstdint>
#include <deque>
#include <span>
#include <tuple>
#include <vector>

namespace roq {
namespace cme {

template <typename T>
struct Buffer final {
  using value_type = T;

  Buffer(size_t size, size_t depth) : buffer_size_(size), buffer_(size * depth) { initialize(); }

  Buffer(Buffer &&) = default;
  Buffer(Buffer const &) = delete;

  bool empty() const { return std::empty(taken_); }

  template <typename Callback>
  bool next(Callback callback) {
    if (std::empty(available_))
      return false;
    auto index = *std::rbegin(available_);
    auto buffer = buffer_from_index(index);
    auto [size, sequence] = callback(buffer);
    if (size && !has(sequence)) {
      taken_.emplace_back(index, size, sequence);
      available_.pop_back();
    }
    return !std::empty(available_);
  }

  // note! linear search (we expect N to be small)
  auto find(T sequence) const {
    for (auto iter = std::begin(taken_); iter != std::end(taken_); ++iter)
      if (sequence == std::get<2>(*iter))
        return iter;
    return std::end(taken_);
  }

  bool has(T sequence) const { return find(sequence) != std::end(taken_); }

  template <typename Callback>
  bool get(T sequence, Callback callback) {
    auto iter = find(sequence);
    if (iter == std::end(taken_))
      return false;
    auto [index, size, _] = *iter;
    auto buffer = buffer_from_index(index, size);
    callback(buffer);
    available_.push_back(index);
    taken_.erase(iter);
    return true;
  }

  void clear() {
    taken_.clear();
    available_.clear();
    initialize();
  }

 protected:
  std::span<std::byte> buffer_from_index(size_t index, size_t size = 0) {
    return {&buffer_[index * buffer_size_], size ? size : buffer_size_};
  }

  void initialize() {
    auto repeat = std::size(buffer_) / buffer_size_;
    for (size_t i = 0; i < repeat; ++i)
      available_.emplace_back(i);
  }

 private:
  const size_t buffer_size_;
  std::vector<std::byte> buffer_;
  std::deque<size_t> available_;
  std::deque<std::tuple<size_t, size_t, T>> taken_;
};

}  // namespace cme
}  // namespace roq
