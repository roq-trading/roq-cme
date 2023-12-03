/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/cme/ilink/sequence.hpp"

#include <cme_ilink/Sequence506.h>

using namespace std::literals;

namespace roq {
namespace cme {
namespace ilink {

namespace {
using header_type = cme_ilink::MessageHeader;
using value_type = cme_ilink::Sequence506;
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
}  // namespace cme
}  // namespace roq
