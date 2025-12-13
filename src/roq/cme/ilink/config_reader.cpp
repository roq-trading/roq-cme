/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/cme/ilink/config_reader.hpp"

#include <absl/strings/ascii.h>

#include <fmt/ranges.h>

#include "roq/logging.hpp"

#include "roq/utils/charconv/from_chars.hpp"

#include "roq/io/fs/file.hpp"

#include "roq/io/memory/mapping.hpp"

#include "roq/core/expat/parser.hpp"

using namespace std::literals;

namespace roq {
namespace cme {
namespace ilink {

namespace {
void trim(auto &value) {
  auto tmp_1 = absl::StripLeadingAsciiWhitespace(value);
  auto tmp_2 = absl::StripTrailingAsciiWhitespace(tmp_1);
  value = std::string{tmp_2};
}
}  // namespace

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
  struct MyHandler final : public core::expat::Parser::Handler {
    explicit MyHandler(ConfigReader::Handler &handler) : handler_(handler) {}
    void operator()(core::expat::Parser::StartElement const &start_element) override {
      auto &[name, attributes] = start_element;
      if (name.compare("marketsegment"sv) == 0) {  // marketsegment
        for (auto &[key, value] : attributes) {
          if (key.compare("id"sv) == 0) {
            market_segment_id_.append(value);
          } else if (key.compare("label"sv) == 0) {
            market_segment_.label = value;
          }
        }
      } else if (!std::empty(market_segment_id_)) {  // children of marketsegment
        using enum MarketSegmentField;
        if (name.compare("protocol"sv) == 0) {
          market_segment_field_ = PROTOCOL;
        } else if (name.compare("primary-host-ip"sv) == 0) {
          market_segment_field_ = PRIMARY_HOST_IP;
        } else if (name.compare("backup-host-ip"sv) == 0) {
          market_segment_field_ = BACKUP_HOST_IP;
        } else if (name.compare("dr-primary-host-ip"sv) == 0) {
          market_segment_field_ = DR_PRIMARY_HOST_IP;
        } else if (name.compare("dr-backup-host-ip"sv) == 0) {
          market_segment_field_ = DR_BACKUP_HOST_IP;
        } else if (name.compare("ToCMESchemaVersion"sv) == 0) {
          market_segment_field_ = TO_CME_SCHEMA_VERSION;
        } else if (name.compare("FromCMESchemaVersion"sv) == 0) {
          market_segment_field_ = FROM_CME_SCHEMA_VERSION;
        } else {
          log::warn(R"(Unexpected: marketsegment.{})"sv, name);
        }
      }
    }
    void operator()(core::expat::Parser::EndElement const &end_element) override {
      auto &name = end_element.name;
      if (name.compare("marketsegment"sv) == 0) {
        assert(!std::empty(market_segment_id_));
        auto market_segment_id = utils::charconv::from_chars<uint8_t>(market_segment_id_);
        handler_(market_segment_id, market_segment_);
        market_segment_id_ = {};
        market_segment_ = {};
      } else {
        switch (market_segment_field_) {
          using enum MarketSegmentField;
          case UNDEFINED:
            break;
          case PROTOCOL:
            trim(market_segment_.protocol);
            break;
          case PRIMARY_HOST_IP:
            trim(market_segment_.primary_host_ip);
            break;
          case BACKUP_HOST_IP:
            trim(market_segment_.backup_host_ip);
            break;
          case DR_PRIMARY_HOST_IP:
            trim(market_segment_.dr_primary_host_ip);
            break;
          case DR_BACKUP_HOST_IP:
            trim(market_segment_.dr_backup_host_ip);
            break;
          case TO_CME_SCHEMA_VERSION:
            trim(market_segment_.to_cme_schema_version);
            break;
          case FROM_CME_SCHEMA_VERSION:
            trim(market_segment_.from_cme_schema_version);
            break;
        }
        market_segment_field_ = {};
      }
    }
    void operator()(core::expat::Parser::CharacterData const &character_data) override {
      auto &data = character_data.data;
      switch (market_segment_field_) {
        using enum MarketSegmentField;
        case UNDEFINED:
          break;
        case PROTOCOL:
          market_segment_.protocol.append(data);
          break;
        case PRIMARY_HOST_IP:
          market_segment_.primary_host_ip.append(data);
          break;
        case BACKUP_HOST_IP:
          market_segment_.backup_host_ip.append(data);
          break;
        case DR_PRIMARY_HOST_IP:
          market_segment_.dr_primary_host_ip.append(data);
          break;
        case DR_BACKUP_HOST_IP:
          market_segment_.dr_backup_host_ip.append(data);
          break;
        case TO_CME_SCHEMA_VERSION:
          market_segment_.to_cme_schema_version.append(data);
          break;
        case FROM_CME_SCHEMA_VERSION:
          market_segment_.from_cme_schema_version.append(data);
          break;
      }
    }

   private:
    ConfigReader::Handler &handler_;
    // state
    std::string market_segment_id_;
    MarketSegment market_segment_;
    enum class MarketSegmentField {
      UNDEFINED,
      PROTOCOL,
      PRIMARY_HOST_IP,
      BACKUP_HOST_IP,
      DR_PRIMARY_HOST_IP,
      DR_BACKUP_HOST_IP,
      TO_CME_SCHEMA_VERSION,
      FROM_CME_SCHEMA_VERSION,
    } market_segment_field_ = {};
  } handler_2(handler);
  core::expat::Parser parser{handler_2};
  parser.parse(buffer, true);
}

}  // namespace ilink
}  // namespace cme
}  // namespace roq
