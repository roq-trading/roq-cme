/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include <algorithm>

#include "roq/cme/multicast/config_reader.hpp"

using namespace std::literals;

using namespace Catch::literals;

using namespace roq;
using namespace roq::cme;

TEST_CASE("config_reader", "[config]") {
  auto message = R"(<?xml version="1.0" encoding="UTF-8"?>)"
                 R"(<configuration environment="PROD" updated="2022/08/02-21:12:03">)"
                 R"(<channel id="310" label="CME Globex Equity Futures">)"
                 R"(<products>)"
                 R"(<product code="ES">)"
                 R"(<group code="ES"/>)"
                 R"(</product>)"
                 R"(<product code="0ES">)"
                 R"(<group code="$E"/>)"
                 R"(</product>)"
                 R"(</products>)"
                 R"(<connections>)"
                 R"(<connection id="310H2A">)"
                 R"(<type feed-type="H">Historical Replay</type>)"
                 R"(<protocol>TCP/IP</protocol>)"
                 R"(<host-ip>205.209.218.10</host-ip>)"
                 R"(<host-ip>205.209.218.10</host-ip>)"
                 R"(<port>10000</port>)"
                 R"(<feed>A</feed>)"
                 R"(</connection>)"
                 R"(<connection id="310IA">)"
                 R"(<type feed-type="I">Incremental</type>)"
                 R"(<protocol>UDP/IP</protocol>)"
                 R"(<ip>224.0.31.1</ip>)"
                 R"(<host-ip>205.209.223.70</host-ip>)"
                 R"(<host-ip>205.209.221.70</host-ip>)"
                 R"(<port>14310</port>)"
                 R"(<feed>A</feed>)"
                 R"(</connection>)"
                 R"(</connections>)"
                 R"(</channel>)"
                 R"(</configuration>)"sv;
  struct MyHandler final : public multicast::ConfigReader::Handler {
    int counter = 0;
    void operator()(std::string_view const &channel_id, multicast::ConfigReader::Channel const &channel) override {
      switch (++counter) {
        case 1: {
          CHECK(channel_id == "310"sv);
          CHECK(channel.label == "CME Globex Equity Futures"sv);
          REQUIRE(std::size(channel.products) == 2);
          CHECK(channel.products.find("ES"sv) != std::end(channel.products));
          CHECK(channel.products.find("0ES"sv) != std::end(channel.products));
          REQUIRE(std::size(channel.connections) == 2);
          REQUIRE(channel.connections.find("310H2A"sv) != std::end(channel.connections));
          REQUIRE(channel.connections.find("310IA"sv) != std::end(channel.connections));
          {
            auto iter = channel.connections.find("310H2A"sv);
            REQUIRE(iter != std::end(channel.connections));
            auto &connection = (*iter).second;
            CHECK(connection.type == "Historical Replay"sv);
            CHECK(connection.protocol == "TCP/IP"sv);
            CHECK(std::empty(connection.ip));
            REQUIRE(std::size(connection.host_ips) == 2);
            CHECK(
                std::find(std::begin(connection.host_ips), std::end(connection.host_ips), "205.209.218.10"sv) !=
                std::end(connection.host_ips));
            // 2nd is the same
            CHECK(connection.port == "10000"sv);
            CHECK(connection.feed == "A"sv);
          }
          {
            auto iter = channel.connections.find("310IA"sv);
            REQUIRE(iter != std::end(channel.connections));
            auto &connection = (*iter).second;
            CHECK(connection.type == "Incremental"sv);
            CHECK(connection.protocol == "UDP/IP"sv);
            CHECK(connection.ip == "224.0.31.1"sv);
            REQUIRE(std::size(connection.host_ips) == 2);
            CHECK(
                std::find(std::begin(connection.host_ips), std::end(connection.host_ips), "205.209.223.70"sv) !=
                std::end(connection.host_ips));
            CHECK(
                std::find(std::begin(connection.host_ips), std::end(connection.host_ips), "205.209.221.70"sv) !=
                std::end(connection.host_ips));
            CHECK(connection.port == "14310"sv);
            CHECK(connection.feed == "A"sv);
          }
          break;
        }
      }
    }
  } handler;
  multicast::ConfigReader::dispatch(handler, message);
  CHECK(handler.counter == 1);
}
