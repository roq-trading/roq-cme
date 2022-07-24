/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/deribit/fix/utils.hpp"

using namespace std::literals;

namespace roq {
namespace deribit {
namespace fix {

SecurityType map_security_type(std::string_view const &value) {
  if (std::size(value) == 3) {
    switch (std::data(value)[0]) {
      case 'F':
        if (value.compare("FUT"sv) == 0)
          return SecurityType::FUTURES;
        break;
      case 'O':
        if (value.compare("OPT"sv) == 0)
          return SecurityType::OPTION;
        break;
    }
  }
  return SecurityType::UNDEFINED;
}

Error map_error(std::string_view const &value) {
  if (std::size(value) > 0) {
    switch (std::data(value)[0]) {
      case 'c':
        if (value.compare("canceled"sv) == 0)
          return Error::UNDEFINED;
        break;
      case 's':
        if (value.compare("success"sv) == 0)
          return Error::UNDEFINED;
        break;
    }
  }
  return Error::UNKNOWN;
}

std::string_view map(Mask<ExecutionInstruction> const &execution_instructions) {
  if (std::empty(execution_instructions))
    return {};
  if (execution_instructions.has(ExecutionInstruction::PARTICIPATE_DO_NOT_INITIATE))
    return "6"sv;
  if (execution_instructions.has(ExecutionInstruction::DO_NOT_INCREASE))
    return "E"sv;
  throw RuntimeError("Not a supported execution instruction"sv);
}

Error reject_to_error(std::string_view const &reason, std::string_view const &text) {
  if (std::empty(reason) && text.compare("rate_limit_exceeded"sv) == 0)
    return Error::REQUEST_RATE_LIMIT_REACHED;
  return {};
}

}  // namespace fix
}  // namespace deribit
}  // namespace roq
