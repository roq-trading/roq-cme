/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/cme/tools/crypto.hpp"

#include <fmt/chrono.h>
#include <fmt/format.h>

#include <cassert>
#include <vector>

#include "roq/clock.hpp"

#include "roq/logging.hpp"

#include "roq/utils/safe_cast.hpp"

#include "roq/core/text/writer.hpp"

#include "roq/core/codec/base64.hpp"

#include "roq/debug/hex/message.hpp"

using namespace std::literals;

using namespace fmt::literals;

namespace roq {
namespace cme {
namespace tools {

// === HELPERS ===

namespace {
template <typename R>
R create_hmac(auto &access_secret) {
  log::info("DEBUG {}"sv, access_secret);
  std::vector<std::byte> buffer;
  buffer.resize(core::codec::Base64::get_max_binary_length(std::size(access_secret)));
  auto key = core::codec::Base64::decode(buffer, access_secret, true, true);
  log::info("DEBUG {}"sv, debug::hex::Message{key});
  return R{key};
}
}  // namespace

// === IMPLEMENTATION ===

Crypto::Crypto(std::string_view const &secret) : mac_{create_hmac<decltype(mac_)>(secret)} {
}

std::span<std::byte const> Crypto::create_signature(
    std::span<std::byte> const &buffer, CanonicalMessage const &message) {
  core::text::Writer writer{buffer};
  static_assert(std::is_same<decltype(message.request_timestamp)::rep, int64_t>::value);
  log::info("DEBUG request_timestamp={}"sv, message.request_timestamp);
  log::info("DEBUG uuid={}"sv, message.uuid);
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
  log::info(R"(DEBUG message="{}")"sv, tmp);
  std::span tmp2 = {reinterpret_cast<std::byte const *>(std::data(tmp)), std::size(tmp)};  // debug
  log::info("DEBUG {}"sv, debug::hex::Message{tmp2});
  mac_.clear();
  mac_.update(tmp);
  auto result = mac_.final(digest_);
  log::info("DEBUG {}"sv, debug::hex::Message{result});
  return result;
}

}  // namespace tools
}  // namespace cme
}  // namespace roq
