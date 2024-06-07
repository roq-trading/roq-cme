/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/cme/ilink/establish.hpp"

#include <cme_ilink/Establish503.h>

using namespace std::literals;

namespace roq {
namespace cme {
namespace ilink {

namespace {
using header_type = cme_ilink::MessageHeader;
using value_type = cme_ilink::Establish503;
}  // namespace

std::span<std::byte const> Establish::encode(std::span<std::byte> const &buffer) const {
  value_type value;
  std::string_view tmp{reinterpret_cast<char const *>(std::data(hmac_signature)), std::size(hmac_signature)};
  auto &result = value.wrapAndApplyHeader(reinterpret_cast<char *>(std::data(buffer)), 0, std::size(buffer));
  result  //
      .putHMACSignature(std::string_view{reinterpret_cast<char const *>(std::data(hmac_signature)), std::size(hmac_signature)})
      .putAccessKeyID(access_key_id)
      .putTradingSystemName(trading_system_name)
      .putTradingSystemVersion(trading_system_version)
      .putTradingSystemVendor(trading_system_vendor)
      .uUID(uuid)
      .requestTimestamp(request_timestamp.count())
      .nextSeqNo(next_seq_no)
      .putSession(session)
      .putFirm(firm)
      .keepAliveInterval(keep_alive_interval.count())
      .skipCredentials();
  auto length = header_type::encodedLength() + value_type::computeLength(0);
  return {std::data(buffer), length};
}

}  // namespace ilink
}  // namespace cme
}  // namespace roq
