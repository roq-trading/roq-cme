/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <deque>
#include <span>
#include <vector>

namespace roq {
namespace cme {

struct Buffer final {
  Buffer(size_t buffer_size, size_t repeat);

  Buffer(Buffer &&) = default;
  Buffer(Buffer const &) = delete;

  template <typename Callback>
  bool next(Callback callback) {
    if (std::empty(available_))
      return false;
    auto index = *std::rbegin(available_);
    std::span buffer{&buffer_[index * buffer_size_], buffer_size_};
    available_.pop_back();
    taken_.push_back(index);
    if (callback(buffer)) {
      available_.push_back(index);
      taken_.pop_back();
    }
    return true;
  }

  template <typename Callback>
  void for_each(Callback callback) {
    for (auto iter = std::begin(taken_); iter != std::end(taken_);) {
      auto index = *iter;
      std::span buffer{&buffer_[index * buffer_size_], buffer_size_};
      if (callback(buffer)) {
        iter = taken_.erase(iter);
      } else {
        ++iter;
      }
    }
  }

  void clear();

 private:
  const size_t buffer_size_;
  std::vector<std::byte> buffer_;
  std::deque<size_t> available_;
  std::deque<size_t> taken_;
};

}  // namespace cme
}  // namespace roq
