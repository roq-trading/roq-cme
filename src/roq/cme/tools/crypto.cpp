/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/cme/tools/crypto.hpp"

#include <fmt/chrono.h>
#include <fmt/format.h>

#include <cassert>
#include <vector>

#include "roq/clock.hpp"

#include "roq/logging.hpp"

#include "roq/utils/safe_cast.hpp"

#include "roq/utils/codec/base64.hpp"

#include "roq/utils/text/writer.hpp"

#include "roq/debug/hex/message.hpp"

using namespace std::literals;

namespace roq {
namespace cme {
namespace tools {

// === HELPERS ===

namespace {
template <typename R>
R create_hmac(auto &access_secret) {
  std::vector<std::byte> buffer;
  buffer.resize(utils::codec::Base64::get_max_binary_length(std::size(access_secret)));
  auto key = utils::codec::Base64::decode(buffer, access_secret, true, true);
  return R{key};
}
}  // namespace

// === IMPLEMENTATION ===

Crypto::Crypto(std::string_view const &secret) : mac_{create_hmac<decltype(mac_)>(secret)} {
}

std::span<std::byte const> Crypto::create_signature(
    std::span<std::byte> const &buffer, CanonicalMessage const &message) {
  utils::text::Writer writer{buffer};
  static_assert(std::is_same<decltype(message.request_timestamp)::rep, int64_t>::value);
  writer  //
      .write(message.request_timestamp.count())
      .write('\n')
      .write(message.uuid)
      .write('\n')
      .write(message.session)
      .write('\n')
      .write(message.firm_id);
  if (!std::empty(message.trading_system_name)) {
    writer  //
        .write('\n')
        .write(message.trading_system_name)
        .write('\n')
        .write(message.trading_system_version)
        .write('\n')
        .write(message.trading_system_vendor)
        .write('\n')
        .write(message.next_seq_no)
        .write('\n')
        .write(message.keep_alive_interval.count());
  }
  auto tmp = static_cast<std::string_view>(writer);
  mac_.clear();
  mac_.update(tmp);
  auto result = mac_.final(digest_);
  return result;
}

}  // namespace tools
}  // namespace cme
}  // namespace roq
