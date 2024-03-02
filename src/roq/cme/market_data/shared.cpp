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
/*
template <typename T, typename MS, typename D>
void read_secdef(T &securities, MS &market_segments, D &dispatcher, auto &settings) {
  auto config_file = settings.common.secdef_config_file;
  if (std::empty(config_file))
    return;
  log::info(R"(Reading instrument definitions from "{}"... (*** can be very slow ***))"sv, config_file);
  struct Handler final : public secdef::ConfigReader::Handler {
    Handler(T &securities, MS &market_segments, D &dispatcher)
        : securities_{securities}, market_segments_{market_segments}, dispatcher_{dispatcher} {}
    void operator()(secdef::ConfigReader::SecDef const &item) override {
      auto discard = dispatcher_.discard_symbol(item.symbol);
      // note! it's too much -- always discard
      if (discard)
        return;
      auto security = tools::Security{
          .exchange = item.exchange,
          .symbol = item.symbol,
          .display_factor = item.display_factor,
          .discard = discard,
      };
      securities_.try_emplace(item.security_id, std::move(security));
      market_segments_[item.market_segment_id].try_emplace(item.symbol, item.security_id);
      double multiplier = item.multiplier == 0 ? NaN : utils::safe_cast<double>(item.multiplier);
      auto reference_data = ReferenceData{
          .stream_id = {},
          .exchange = item.exchange,
          .symbol = item.symbol,
          .description = {},
          .security_type = SecurityType::FUTURES,  // ???
          .base_currency = {},
          .quote_currency = item.currency,
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
      TraceInfo trace_info;
      create_trace_and_dispatch(dispatcher_, trace_info, reference_data, true);
    }

   private:
    T &securities_;
    MS &market_segments_;
    D &dispatcher_;
  } handler{securities, market_segments, dispatcher};
  secdef::ConfigReader::read(handler, config_file);
}
*/
}  // namespace

// === IMPLEMENTATION ===

Shared::Shared() : buffer(BUFFER_SIZE) {
  // read_secdef(securities, market_segments, dispatcher, settings);
}

}  // namespace market_data
}  // namespace cme
}  // namespace roq
