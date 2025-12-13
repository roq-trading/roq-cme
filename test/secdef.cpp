/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/cme/secdef/config_reader.hpp"

using namespace std::literals;

using namespace Catch::literals;

using namespace roq;
using namespace roq::cme;

TEST_CASE("secdef_reader", "[secdef]") {
  auto message =
      // line 1
      "35=d\0015799=00000000\001980=A\001779=20220731160356000824\0011180=310\0011300=64\001462=5\001207=XCME\0011151="
      "ES\0016937=ES\00155=ESU4\00148=118\00122=8\001167=FUT\001461=FFIXSX\001200=202409\00115=USD\0011142=F\001562="
      "1\0011140=3000\001969=25.000000000\0019787=0.010000000\001996=IPNT\0011147=50.000000000\0011150=428400."
      "000000000\001731=00000011\0015796=20220801\0011149=457200.000000000\0011148=399550.000000000\0011143=600."
      "000000000\0011146=12.500000000\0019779=N\001864=2\001865=5\0011145=20220617133000000000\001865=7\0011145="
      "20240920133000000000\0011141=1\0011022=GBX\001264=10\001870=1\001871=24\001872="
      "00000000000001000010000000001111\0011234=0\001\n"
      // line2
      "35=d\0015799=00000000\001980=A\001779=20220731160356000824\0011180=310\0011300=64\001462=5\001207=XCME\0011151=$E\0016937=0ES\00155=0ESZ2\00148=1312\00122=8\001167=FUT\001461=FFIXSX\001200=202212\00115=USD\0011142=F\001562=1\0011140=3000\001969=25.000000000\0019787=0.010000000\001996=IPNT\0011147=50.000000000\0011150=413750.000000000\001731=00000011\0015796=20220801\0011148=25.000000000\0011143=0.000000000\0011146=12.500000000\0019779=N\001864=2\001865=5\0011145=20220617133000000000\001865=7\0011145=20221216143000000000\0011141=1\0011022=GBX\001264=10\001870=1\001871=24\001872=00000000000001000010000000001111\0011234=0\001\n"sv;
  struct Handler final : public secdef::ConfigReader::Handler {
    int counter = 0;
    void operator()(secdef::ConfigReader::SecDef const &sec_def) override {
      switch (++counter) {
        case 1:
          CHECK(sec_def.security_id == 118);
          CHECK(sec_def.exchange == "XCME"sv);
          CHECK(sec_def.symbol == "ESU4"sv);
          CHECK(sec_def.security_type == "FUT"sv);
          CHECK(sec_def.currency == "USD"sv);
          CHECK(sec_def.multiplier == 0);
          CHECK(sec_def.min_trade_vol == 1);
          CHECK(sec_def.max_trade_vol == 3000);
          CHECK(sec_def.min_price_increment == 25.0_a);
          CHECK(sec_def.display_factor == 0.01_a);
          CHECK(sec_def.asset == "ES"sv);
          break;
        case 2:
          CHECK(sec_def.security_id == 1312);
          CHECK(sec_def.exchange == "XCME"sv);
          CHECK(sec_def.symbol == "0ESZ2"sv);
          CHECK(sec_def.security_type == "FUT"sv);
          CHECK(sec_def.currency == "USD"sv);
          CHECK(sec_def.multiplier == 0);
          CHECK(sec_def.min_trade_vol == 1);
          CHECK(sec_def.max_trade_vol == 3000);
          CHECK(sec_def.min_price_increment == 25.0_a);
          CHECK(sec_def.display_factor == 0.01_a);
          CHECK(sec_def.asset == "0ES"sv);
          break;
      }
    }
  } handler;
  secdef::ConfigReader::dispatch(handler, message);
  CHECK(handler.counter == 2);
}

/*
TEST_CASE("secdef_reader_read", "[secdef]") {
  struct MyHandler final : public tools::SecDefReader::Handler {
    int counter = 0;
    void operator()(tools::SecDefReader::SecDef const &sec_def) override { ++counter; }
  } handler;
  tools::SecDefReader::read(handler, "/home/thraneh/tmp/secdef.dat"sv);
  CHECK(handler.counter == 666816);
}
*/
