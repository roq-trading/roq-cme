/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/cme/ilink/negotiate.hpp"

#include <cme_ilink/Negotiate500.h>

using namespace std::literals;

namespace roq {
namespace cme {
namespace ilink {

std::span<std::byte const> Negotiate::encode(std::span<std::byte> const &buffer) const {
  cme_ilink::Negotiate500 negotiate;
  auto &ngo = negotiate.wrapAndApplyHeader(reinterpret_cast<char *>(std::data(buffer)), 0, std::size(buffer));
  ngo  //
      .putHMACSignature(std::string_view{reinterpret_cast<char const *>(std::data(signature)), std::size(signature)})
      .uUID(uuid)
      .requestTimestamp(request_timestamp.count())
      .putSession(session)
      .putFirm(firm);
  std::span result{std::data(buffer), cme_ilink::Negotiate500::sbeBlockAndHeaderLength()};
  return result;
}

}  // namespace ilink
}  // namespace cme
}  // namespace roq
