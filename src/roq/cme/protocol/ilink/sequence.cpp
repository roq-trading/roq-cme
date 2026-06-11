/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/cme/protocol/ilink/sequence.hpp"

#include <cme/sbe/ilink/Sequence506.h>

using namespace std::literals;

namespace roq {
namespace cme {
namespace protocol {
namespace ilink {

namespace {
using header_type = ::cme::sbe::ilink::MessageHeader;
using value_type = ::cme::sbe::ilink::Sequence506;
}  // namespace

std::span<std::byte const> Sequence::encode(std::span<std::byte> const &buffer) const {
  value_type value;
  auto &result = value.wrapAndApplyHeader(reinterpret_cast<char *>(std::data(buffer)), 0, std::size(buffer));
  result  //
      .uUID(uuid)
      .nextSeqNo(next_seq_no)
      .faultToleranceIndicator(fault_tolerance_indicator)
      .keepAliveIntervalLapsed(keep_alive_interval_lapsed);
  auto length = header_type::encodedLength() + value_type::computeLength();
  return {std::data(buffer), length};
}

}  // namespace ilink
}  // namespace protocol
}  // namespace cme
}  // namespace roq
