/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/cme/market_data/shared.hpp"

#include "roq/logging.hpp"

#include "roq/utils/safe_cast.hpp"

#include "roq/cme/secdef/config_reader.hpp"

using namespace std::literals;

namespace roq {
namespace cme {
namespace market_data {

// === CONSTANTS ===

namespace {
auto const BUFFER_SIZE = 4096uz;
}

// === HELPERS ===

namespace {
/*
template <typename R>
R read_ilink_config(auto const &filename) {
R result;
if (!std::empty(filename)) {
  struct Handler final : public ilink::ConfigReader::Handler {
    explicit Handler(R &result) : result_{result} {}

   protected:
    void operator()(uint8_t market_segment_id, ilink::ConfigReader::MarketSegment const &market_segment) override {
      result_.try_emplace(market_segment_id, market_segment);
    }

   private:
    R &result_;
  } handler{result};
  ilink::ConfigReader::read(handler, filename);
}
return result;
}
*/
template <typename SD, typename D>
struct Handler final : public secdef::ConfigReader::Handler {
  Handler(SD &security_definitions, D &dispatcher, std::chrono::nanoseconds first_timestamp)
      : security_definitions_{security_definitions}, dispatcher_{dispatcher},
        origin_create_time_utc_{first_timestamp.count() ? first_timestamp : clock::get_realtime()} {}

  void operator()(secdef::ConfigReader::SecDef const &item) override {
    auto discard = dispatcher_.discard_symbol(item.symbol);
    // note! it's too much -- always discard
    if (discard)
      return;
    if (security_definitions_.get_security(item.security_id, [&](auto &security) {
          double multiplier = item.multiplier == 0 ? NaN : utils::safe_cast<double>(item.multiplier);
          auto reference_data = ReferenceData{
              .stream_id = {},
              .exchange = item.exchange,
              .symbol = item.symbol,
              .description = {},
              .security_type = SecurityType::FUTURES,  // ???
              .cfi_code = {},
              .base_currency = {},
              .quote_currency = item.currency,
              .settlement_currency = {},
              .margin_currency = {},
              .commission_currency = {},
              .tick_size = item.min_price_increment * item.display_factor,
              .multiplier = multiplier,
              .min_trade_vol = utils::safe_cast(item.min_trade_vol),
              .max_trade_vol = utils::safe_cast(item.max_trade_vol),
              .trade_vol_step_size = NaN,
              .option_type = {},
              .strike_currency = {},
              .strike_price = NaN,
              .underlying = {},
              .time_zone = {},
              .issue_date = {},
              .settlement_date = {},
              .expiry_datetime = {},  // MaturityMonthYear ???
              .expiry_datetime_utc = {},
              .exchange_time_utc = {},
              .exchange_sequence = {},
              .sending_time_utc = {},
              .discard = security.discard,
          };
          auto now = clock::get_system();
          TraceInfo trace_info{now, now, origin_create_time_utc_};
          create_trace_and_dispatch(dispatcher_, trace_info, reference_data, true);
        })) {
    } else {
      log::fatal("Unexpected"sv);
    }
  }

 private:
  SD &security_definitions_;
  D &dispatcher_;
  std::chrono::nanoseconds const origin_create_time_utc_;
};
}  // namespace

// === IMPLEMENTATION ===

Shared::Shared(server::md::Dispatcher &dispatcher, Options const &options, SecurityDefinitions &security_definitions)
    : dispatcher_{dispatcher}, options{options}, security_definitions{security_definitions}, buffer(BUFFER_SIZE) {
}

void Shared::read_secdef(std::string_view const &config_file, std::chrono::nanoseconds first_timestamp) {
  if (std::empty(config_file))
    return;
  log::info(R"(Publishing instrument definitions from "{}"...)"sv, config_file);
  Handler handler{security_definitions, dispatcher_, first_timestamp};
  secdef::ConfigReader::read(handler, config_file);
}

}  // namespace market_data
}  // namespace cme
}  // namespace roq
