/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/cme/ilink/terminate.hpp"

#include <cme/sbe/ilink/Terminate507.h>

using namespace std::literals;

namespace roq {
namespace cme {
namespace ilink {

namespace {
using header_type = ::cme::sbe::ilink::MessageHeader;
using value_type = ::cme::sbe::ilink::Terminate507;
}  // namespace

std::span<std::byte const> Terminate::encode(std::span<std::byte> const &buffer) const {
  value_type value;
  auto &result = value.wrapAndApplyHeader(reinterpret_cast<char *>(std::data(buffer)), 0, std::size(buffer));
  result  //
      .putReason(reason)
      .uUID(uuid)
      .requestTimestamp(request_timestamp.count())
      .errorCodes(error_codes)
      .splitMsg(split_msg);
  auto length = header_type::encodedLength() + value_type::computeLength();
  return {std::data(buffer), length};
}

}  // namespace ilink
}  // namespace cme
}  // namespace roq
