/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include <algorithm>

#include "roq/cme/ilink/config_reader.hpp"

using namespace std::literals;

using namespace Catch::literals;

using namespace roq;
using namespace roq::cme;

TEST_CASE("simple", "[ilink_config_reader]") {
  auto message = R"(<?xml version='1.0' encoding='UTF-8'?>)"
                 R"(<configuration environment="Production" updated="Thu Feb 23 18:00:05 2023 CST">)"
                 R"(<marketsegment id="70" label="CME Commodity Futures">)"
                 R"(<protocol>TCP/IP</protocol>)"
                 R"(<primary-host-ip>205.209.201.117</primary-host-ip>)"
                 R"(<backup-host-ip>205.209.201.41</backup-host-ip>)"
                 R"(<dr-primary-host-ip>205.209.201.117</dr-primary-host-ip>)"
                 R"(<dr-backup-host-ip>205.209.201.41</dr-backup-host-ip>)"
                 R"(<ToCMESchemaVersion>8</ToCMESchemaVersion>)"
                 R"(<FromCMESchemaVersion>8</FromCMESchemaVersion>)"
                 R"(</marketsegment>)"
                 R"(</configuration>)"sv;
  struct MyHandler final : public ilink::ConfigReader::Handler {
    int counter = 0;
    void operator()(uint8_t market_segment_id, ilink::ConfigReader::MarketSegment const &market_segment) override {
      switch (++counter) {
        case 1: {
          CHECK(market_segment_id == 70);
          CHECK(market_segment.label == "CME Commodity Futures"sv);
          CHECK(market_segment.protocol == "TCP/IP"sv);
          CHECK(market_segment.primary_host_ip == "205.209.201.117"sv);
          CHECK(market_segment.backup_host_ip == "205.209.201.41"sv);
          CHECK(market_segment.dr_primary_host_ip == "205.209.201.117"sv);
          CHECK(market_segment.dr_backup_host_ip == "205.209.201.41"sv);
          CHECK(market_segment.to_cme_schema_version == "8"sv);
          CHECK(market_segment.from_cme_schema_version == "8"sv);
          break;
        }
      }
    }
  } handler;
  ilink::ConfigReader::dispatch(handler, message);
  CHECK(handler.counter == 1);
}
