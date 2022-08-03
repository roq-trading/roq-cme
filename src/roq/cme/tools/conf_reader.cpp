/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/cme/tools/conf_reader.hpp"

#include <fmt/ranges.h>

#include "roq/logging.hpp"

#include "roq/core/charconv.hpp"

#include "roq/core/filesystem/file.hpp"

#include "roq/core/memory/mapping.hpp"

#include "roq/core/expat/parser.hpp"

using namespace std::literals;

namespace roq {
namespace cme {
namespace tools {

void ConfReader::read(Handler &handler, std::string_view const &filename) {
  core::filesystem::File file{filename, {core::filesystem::Flags::READ_ONLY}};
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

void ConfReader::dispatch(Handler &handler, std::string_view const &buffer) {
  struct MyHandler final : public core::expat::Parser::Handler {
    explicit MyHandler(ConfReader::Handler &handler) : handler_(handler) {}
    void operator()(core::expat::Parser::StartElement const &start_element) override {
      auto &[name, attributes] = start_element;
      if (name.compare("channel"sv) == 0) {  // channel
        for (auto &[key, value] : attributes) {
          if (key.compare("id"sv) == 0) {
            channel_id_ = core::from_chars<decltype(channel_id_)>(value);
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
          log::fatal(R"(Unexpected: connection.{})"sv, name);
        }
      }
    }
    void operator()(core::expat::Parser::EndElement const &end_element) override {
      auto &name = end_element.name;
      if (name.compare("connection"sv) == 0) {
        assert(!std::empty(connection_id_));
        channel_.connections.try_emplace(std::move(connection_id_), std::move(connection_));
        connection_id_ = {};
        connection_field_ = {};
        connection_ = {};
      } else if (name.compare("channel"sv) == 0) {
        assert(channel_id_ != 0);
        handler_(channel_id_, std::move(channel_));
        channel_id_ = {};
        channel_ = {};
      }
    }
    void operator()(core::expat::Parser::CharacterData const &character_data) override {
      auto &data = character_data.data;
      switch (connection_field_) {
        using enum ConnectionField;
        case UNDEFINED:
          log::fatal("Unexpeted"sv);
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
          // note! assumes full update
          connection_.port = core::from_chars<decltype(connection_.port)>(data);
          break;
        case FEED:
          connection_.feed.append(data);
          break;
      }
    }

   private:
    ConfReader::Handler &handler_;
    // state
    uint32_t channel_id_ = {};
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

}  // namespace tools
}  // namespace cme
}  // namespace roq
