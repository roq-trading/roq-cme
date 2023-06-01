/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/cme/ilink/negotiate.hpp"

#include <cme_ilink/Negotiate500.h>

using namespace std::literals;

namespace roq {
namespace cme {
namespace ilink {

namespace {
using header_type = cme_ilink::MessageHeader;
using value_type = cme_ilink::Negotiate500;
}  // namespace

std::span<std::byte const> Negotiate::encode(std::span<std::byte> const &buffer) const {
  value_type value;
  std::string_view tmp{reinterpret_cast<char const *>(std::data(hmac_signature)), std::size(hmac_signature)};
  auto &result = value.wrapAndApplyHeader(reinterpret_cast<char *>(std::data(buffer)), 0, std::size(buffer));
  result  //
      .putHMACSignature(tmp)
      .putAccessKeyID(access_key_id)
      .uUID(uuid)
      .requestTimestamp(request_timestamp.count())
      .putSession(session)
      .putFirm(firm)
      .skipCredentials();
  auto length = header_type::encodedLength() + value_type::computeLength(0);
  return {std::data(buffer), length};
}

}  // namespace ilink
}  // namespace cme
}  // namespace roq
