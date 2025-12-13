/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/cme/secdef/config_reader.hpp"

#include "roq/logging.hpp"

#include "roq/utils/charconv/from_chars.hpp"

#include "roq/io/fs/file.hpp"

#include "roq/io/memory/mapping.hpp"

using namespace std::literals;

namespace roq {
namespace cme {
namespace secdef {

void set(auto &result, auto &key, auto &value) {
  // XXX we could use discard to avoid conversion
  auto tag = utils::charconv::from_chars<uint32_t>(key);
  switch (tag) {
    case 15:  // currency (string)
      result.currency = value;
      break;
    case 48:  // security id (int32)
      result.security_id = utils::charconv::from_chars<decltype(result.security_id)>(value);
      break;
    case 55:  // symbol (string)
      result.symbol = value;
      break;
    case 167:  // security type (string)
      result.security_type = value;
      break;
    case 207:  // exchange (string)
      result.exchange = value;
      break;
    case 231:  // contract multiplier (int32)
      result.multiplier = utils::charconv::from_chars<decltype(result.multiplier)>(value);
      break;
    case 562:  // min trade vol (uint32)
      result.min_trade_vol = utils::charconv::from_chars<decltype(result.min_trade_vol)>(value);
      break;
    case 969:  // min price increment (price9)
      result.min_price_increment = utils::charconv::from_chars<decltype(result.min_price_increment)>(value);
      break;
    case 1140:  // max trade vol (uint32)
      result.max_trade_vol = utils::charconv::from_chars<decltype(result.max_trade_vol)>(value);
      break;
    case 1300:  // market segment id (uint8)
      result.market_segment_id = utils::charconv::from_chars<decltype(result.market_segment_id)>(value);
      break;
    case 6937:  // asset
      result.asset = value;
      break;
    case 9787:  // display factor (decimal9)
      result.display_factor = utils::charconv::from_chars<decltype(result.display_factor)>(value);
      break;
  }
}

void ConfigReader::read(Handler &handler, std::string_view const &filename) {
  io::fs::File file{filename, {io::fs::Flags::READ_ONLY}};
  io::memory::Mapping memory(
      std::size(file),
      {
          io::memory::Protections::READ,
          io::memory::Protections::WRITE,
      },
      {
          io::memory::Flags::PRIVATE,
      },
      file);
  auto buffer = static_cast<std::span<std::byte const>>(memory);
  dispatch(handler, {reinterpret_cast<char const *>(std::data(buffer)), std::size(buffer)});
}

void ConfigReader::dispatch(Handler &handler, std::string_view const &buffer) {
  auto tmp = buffer;
  SecDef sec_def = {};
  while (!std::empty(tmp)) {
    auto key_value = tmp.substr(0, tmp.find_first_of("\001\n"sv));
    if (std::empty(key_value)) {
      handler(sec_def);
      sec_def = {};
    } else {
      auto key = key_value.substr(0, key_value.find('='));
      auto value = key_value.substr(std::size(key) + 1);
      set(sec_def, key, value);
    }
    tmp = tmp.substr(std::size(key_value) + 1);
  }
}

}  // namespace secdef
}  // namespace cme
}  // namespace roq
