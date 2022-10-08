/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/cme/multicast/config_reader.hpp"

#include <absl/strings/ascii.h>

#include <fmt/ranges.h>

#include "roq/logging.hpp"

#include "roq/core/charconv.hpp"

#include "roq/core/fs/file.hpp"

#include "roq/core/memory/mapping.hpp"

#include "roq/core/expat/parser.hpp"

using namespace std::literals;

namespace roq {
namespace cme {
namespace multicast {

namespace {
void trim(auto &value) {
  auto tmp_1 = absl::StripLeadingAsciiWhitespace(value);
  auto tmp_2 = absl::StripTrailingAsciiWhitespace(tmp_1);
  value = std::string{tmp_2};
}
}  // namespace

void ConfigReader::read(Handler &handler, std::string_view const &filename) {
  core::fs::File file{filename, {core::fs::Flags::READ_ONLY}};
  core::memory::Mapping memory(
      std::size(file),
      {
          core::memory::Protections::READ,
          core::memory::Protections::WRITE,
      },
      {
          core::memory::Flags::PRIVATE,
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
      if (name.compare("channel"sv) == 0) {  // channel
        for (auto &[key, value] : attributes) {
          if (key.compare("id"sv) == 0) {
            channel_id_.append(value);
          } else if (key.compare("label"sv) == 0) {
            channel_.label = value;
          }
        }
      } else if (name.compare("product"sv) == 0) {  // product
        for (auto &[key, value] : attributes) {
          if (key.compare("code"sv) == 0) {
            channel_.products.emplace(value);
          }
        }
      } else if (name.compare("connection"sv) == 0) {  // connection
        for (auto &[key, value] : attributes) {
          if (key.compare("id"sv) == 0) {
            connection_id_ = value;
          }
        }
      } else if (!std::empty(connection_id_)) {  // children of connection
        using enum ConnectionField;
        if (name.compare("type"sv) == 0) {
          connection_field_ = TYPE;
          for (auto &[key, value] : attributes) {
            if (key.compare("feed-type"sv) == 0) {
              connection_.feed_type = value;
            }
          }
        } else if (name.compare("protocol"sv) == 0) {
          connection_field_ = PROTOCOL;
        } else if (name.compare("ip"sv) == 0) {
          connection_field_ = IP;
        } else if (name.compare("host-ip"sv) == 0) {
          connection_field_ = HOST_IP;
        } else if (name.compare("port"sv) == 0) {
          connection_field_ = PORT;
        } else if (name.compare("feed"sv) == 0) {
          connection_field_ = FEED;
        } else {
          log::warn(R"(Unexpected: connection.{})"sv, name);
        }
      }
    }
    void operator()(core::expat::Parser::EndElement const &end_element) override {
      auto &name = end_element.name;
      if (name.compare("connection"sv) == 0) {
        assert(!std::empty(connection_id_));
        channel_.connections.try_emplace(connection_id_, connection_);
        connection_id_ = {};
        connection_ = {};
      } else if (name.compare("channel"sv) == 0) {
        assert(!std::empty(channel_id_));
        handler_(channel_id_, channel_);
        channel_id_.clear();
        channel_ = {};
      } else {
        switch (connection_field_) {
          using enum ConnectionField;
          case UNDEFINED:
            break;
          case TYPE:
            trim(connection_.type);
            break;
          case PROTOCOL:
            trim(connection_.protocol);
            break;
          case IP:
            trim(connection_.ip);
            break;
          case HOST_IP:
            // do nothing
            break;
          case PORT:
            trim(connection_.port);
            break;
          case FEED:
            trim(connection_.feed);
            break;
        }
        connection_field_ = {};
      }
    }
    void operator()(core::expat::Parser::CharacterData const &character_data) override {
      auto &data = character_data.data;
      switch (connection_field_) {
        using enum ConnectionField;
        case UNDEFINED:
          break;
        case TYPE:
          connection_.type.append(data);
          break;
        case PROTOCOL:
          connection_.protocol.append(data);
          break;
        case IP:
          connection_.ip.append(data);
          break;
        case HOST_IP:
          // note! assumes full update
          connection_.host_ips.emplace_back(data);
          break;
        case PORT:
          connection_.port.append(data);
          break;
        case FEED:
          connection_.feed.append(data);
          break;
      }
    }

   private:
    ConfigReader::Handler &handler_;
    // state
    std::string channel_id_;
    Channel channel_ = {};
    std::string connection_id_;
    enum class ConnectionField {
      UNDEFINED,
      TYPE,
      PROTOCOL,
      IP,
      HOST_IP,
      PORT,
      FEED,
    } connection_field_ = {};
    Connection connection_ = {};
  } handler_2(handler);
  core::expat::Parser parser(handler_2);
  parser.parse(buffer, true);
}

}  // namespace multicast
}  // namespace cme
}  // namespace roq
