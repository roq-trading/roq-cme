/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/cme/mdp/md_instrument_definition.hpp"

#include "roq/utils/safe_cast.hpp"

#include "roq/logging.hpp"

#include "roq/cme/mdp/map.hpp"

using namespace std::literals;

namespace roq {
namespace cme {
namespace mdp {

// reference data

ReferenceData create_reference_data(cme_mdp::MDInstrumentDefinitionFuture54 const &message, uint16_t stream_id, tools::Security &security) {
  using value_type = std::remove_cvref_t<decltype(message)>;
  auto &value = const_cast<value_type &>(message);  // note! not const-safe
  auto quote_currency = mdp::get_string_view(value.currency(), value.currencyLength());
  auto settlement_currency = mdp::get_string_view(value.settlCurrency(), value.settlCurrencyLength());
  auto min_price_increment = map(value.minPriceIncrement()).template get<double>();
  auto contract_multiplier = mdp::get_int(value.contractMultiplier(), value.contractMultiplierNullValue());
  auto multiplier = contract_multiplier == 0 ? NaN : utils::safe_cast<double>(contract_multiplier);
  auto min_trade_vol = utils::safe_cast(value.minTradeVol());
  auto max_trade_vol = utils::safe_cast(value.maxTradeVol());
  return {
      .stream_id = stream_id,
      .exchange = security.exchange,
      .symbol = security.symbol,
      .description = {},
      .security_type = SecurityType::FUTURES,
      .cfi_code = {},
      .base_currency = {},
      .quote_currency = quote_currency,
      .settlement_currency = settlement_currency,
      .margin_currency = {},
      .commission_currency = {},
      .tick_size = min_price_increment * security.display_factor,
      .tick_size_steps = {},
      .multiplier = multiplier,
      .min_trade_vol = min_trade_vol,
      .max_trade_vol = max_trade_vol,
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
      .discard = security.discard,
  };
}

ReferenceData create_reference_data(cme_mdp::MDInstrumentDefinitionOption55 const &message, uint16_t stream_id, tools::Security &security) {
  using value_type = std::remove_cvref_t<decltype(message)>;
  auto &value = const_cast<value_type &>(message);  // note! not const-safe
  auto quote_currency = mdp::get_string_view(value.currency(), value.currencyLength());
  auto settlement_currency = mdp::get_string_view(value.settlCurrency(), value.settlCurrencyLength());
  auto min_price_increment = map(value.minPriceIncrement()).template get<double>();
  auto min_trade_vol = utils::safe_cast(value.minTradeVol());
  auto max_trade_vol = utils::safe_cast(value.maxTradeVol());
  auto strike_currency = mdp::get_string_view(value.strikeCurrency(), value.strikeCurrencyLength());
  return {
      .stream_id = stream_id,
      .exchange = security.exchange,
      .symbol = security.symbol,
      .description = {},
      .security_type = SecurityType::OPTION,
      .cfi_code = {},
      .base_currency = {},
      .quote_currency = quote_currency,
      .settlement_currency = settlement_currency,
      .margin_currency = {},
      .commission_currency = {},
      .tick_size = min_price_increment * security.display_factor,
      .tick_size_steps = {},
      .multiplier = NaN,
      .min_notional = NaN,
      .min_trade_vol = min_trade_vol,
      .max_trade_vol = max_trade_vol,
      .trade_vol_step_size = NaN,
      .option_type = {},
      .strike_currency = strike_currency,
      .strike_price = NaN,
      .underlying = {},
      .time_zone = {},
      .issue_date = {},
      .settlement_date = {},
      .expiry_datetime = {},  // MaturityMonthYear ???
      .expiry_datetime_utc = {},
      .discard = security.discard,
  };
}

ReferenceData create_reference_data(cme_mdp::MDInstrumentDefinitionSpread56 const &message, uint16_t stream_id, tools::Security &security) {
  using value_type = std::remove_cvref_t<decltype(message)>;
  auto &value = const_cast<value_type &>(message);  // note! not const-safe
  auto quote_currency = mdp::get_string_view(value.currency(), value.currencyLength());
  auto min_trade_vol = utils::safe_cast(value.minTradeVol());
  auto max_trade_vol = utils::safe_cast(value.maxTradeVol());
  return {
      .stream_id = stream_id,
      .exchange = security.exchange,
      .symbol = security.symbol,
      .description = {},
      .security_type = SecurityType::FUTURES,
      .cfi_code = {},
      .base_currency = {},
      .quote_currency = quote_currency,
      .settlement_currency = {},
      .margin_currency = {},
      .commission_currency = {},
      .tick_size = map(value.minPriceIncrement()),
      .tick_size_steps = {},
      .multiplier = NaN,
      .min_trade_vol = min_trade_vol,
      .max_trade_vol = max_trade_vol,
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
      .discard = security.discard,
  };
}

ReferenceData create_reference_data(cme_mdp::MDInstrumentDefinitionFixedIncome57 const &message, uint16_t stream_id, tools::Security &security) {
  using value_type = std::remove_cvref_t<decltype(message)>;
  auto &value = const_cast<value_type &>(message);  // note! not const-safe
  auto quote_currency = mdp::get_string_view(value.currency(), value.currencyLength());
  auto settlement_currency = mdp::get_string_view(value.settlCurrency(), value.settlCurrencyLength());
  auto min_trade_vol = utils::safe_cast(value.minTradeVol());
  auto max_trade_vol = utils::safe_cast(value.maxTradeVol());
  return {
      .stream_id = stream_id,
      .exchange = security.exchange,
      .symbol = security.symbol,
      .description = {},
      .security_type = SecurityType::FUTURES,
      .cfi_code = {},
      .base_currency = {},
      .quote_currency = quote_currency,
      .settlement_currency = settlement_currency,
      .margin_currency = {},
      .commission_currency = {},
      .tick_size = map(value.minPriceIncrement()),
      .tick_size_steps = {},
      .multiplier = NaN,
      .min_trade_vol = min_trade_vol,
      .max_trade_vol = max_trade_vol,
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
      .discard = security.discard,
  };
}

ReferenceData create_reference_data(cme_mdp::MDInstrumentDefinitionRepo58 const &message, uint16_t stream_id, tools::Security &security) {
  using value_type = std::remove_cvref_t<decltype(message)>;
  auto &value = const_cast<value_type &>(message);  // note! not const-safe
  auto quote_currency = mdp::get_string_view(value.currency(), value.currencyLength());
  auto settlement_currency = mdp::get_string_view(value.settlCurrency(), value.settlCurrencyLength());
  auto min_trade_vol = utils::safe_cast(value.minTradeVol());
  auto max_trade_vol = utils::safe_cast(value.maxTradeVol());
  return {
      .stream_id = stream_id,
      .exchange = security.exchange,
      .symbol = security.symbol,
      .description = {},
      .security_type = SecurityType::FUTURES,
      .cfi_code = {},
      .base_currency = {},
      .quote_currency = quote_currency,
      .settlement_currency = settlement_currency,
      .margin_currency = {},
      .commission_currency = {},
      .tick_size = map(value.minPriceIncrement()),
      .tick_size_steps = {},
      .multiplier = NaN,
      .min_trade_vol = min_trade_vol,
      .max_trade_vol = max_trade_vol,
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
      .discard = security.discard,
  };
}

ReferenceData create_reference_data(cme_mdp::MDInstrumentDefinitionFX63 const &message, uint16_t stream_id, tools::Security &security) {
  using value_type = std::remove_cvref_t<decltype(message)>;
  auto &value = const_cast<value_type &>(message);  // note! not const-safe
  auto quote_currency = mdp::get_string_view(value.currency(), value.currencyLength());
  auto settlement_currency = mdp::get_string_view(value.settlCurrency(), value.settlCurrencyLength());
  auto min_trade_vol = utils::safe_cast(value.minTradeVol());
  auto max_trade_vol = utils::safe_cast(value.maxTradeVol());
  return {
      .stream_id = stream_id,
      .exchange = security.exchange,
      .symbol = security.symbol,
      .description = {},
      .security_type = SecurityType::FUTURES,
      .cfi_code = {},
      .base_currency = {},
      .quote_currency = quote_currency,
      .settlement_currency = settlement_currency,
      .margin_currency = {},
      .commission_currency = {},
      .tick_size = map(value.minPriceIncrement()),
      .tick_size_steps = {},
      .multiplier = NaN,
      .min_trade_vol = min_trade_vol,
      .max_trade_vol = max_trade_vol,
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
      .discard = security.discard,
  };
}

// market status

MarketStatus create_market_status(cme_mdp::MDInstrumentDefinitionFuture54 const &message, uint16_t stream_id, tools::Security &security) {
  using value_type = std::remove_cvref_t<decltype(message)>;
  auto &value = const_cast<value_type &>(message);  // note! not const-safe
  return {
      .stream_id = stream_id,
      .exchange = security.exchange,
      .symbol = security.symbol,
      .trading_status = map(value.mDSecurityTradingStatus()),
  };
}

MarketStatus create_market_status(cme_mdp::MDInstrumentDefinitionOption55 const &message, uint16_t stream_id, tools::Security &security) {
  using value_type = std::remove_cvref_t<decltype(message)>;
  auto &value = const_cast<value_type &>(message);  // note! not const-safe
  return {
      .stream_id = stream_id,
      .exchange = security.exchange,
      .symbol = security.symbol,
      .trading_status = map(value.mDSecurityTradingStatus()),
  };
}

MarketStatus create_market_status(cme_mdp::MDInstrumentDefinitionSpread56 const &message, uint16_t stream_id, tools::Security &security) {
  using value_type = std::remove_cvref_t<decltype(message)>;
  auto &value = const_cast<value_type &>(message);  // note! not const-safe
  return {
      .stream_id = stream_id,
      .exchange = security.exchange,
      .symbol = security.symbol,
      .trading_status = map(value.mDSecurityTradingStatus()),
  };
}

MarketStatus create_market_status(cme_mdp::MDInstrumentDefinitionFixedIncome57 const &message, uint16_t stream_id, tools::Security &security) {
  using value_type = std::remove_cvref_t<decltype(message)>;
  auto &value = const_cast<value_type &>(message);  // note! not const-safe
  return {
      .stream_id = stream_id,
      .exchange = security.exchange,
      .symbol = security.symbol,
      .trading_status = map(value.mDSecurityTradingStatus()),
  };
}

MarketStatus create_market_status(cme_mdp::MDInstrumentDefinitionRepo58 const &message, uint16_t stream_id, tools::Security &security) {
  using value_type = std::remove_cvref_t<decltype(message)>;
  auto &value = const_cast<value_type &>(message);  // note! not const-safe
  return {
      .stream_id = stream_id,
      .exchange = security.exchange,
      .symbol = security.symbol,
      .trading_status = map(value.mDSecurityTradingStatus()),
  };
}

MarketStatus create_market_status(cme_mdp::MDInstrumentDefinitionFX63 const &message, uint16_t stream_id, tools::Security &security) {
  using value_type = std::remove_cvref_t<decltype(message)>;
  auto &value = const_cast<value_type &>(message);  // note! not const-safe
  return {
      .stream_id = stream_id,
      .exchange = security.exchange,
      .symbol = security.symbol,
      .trading_status = map(value.mDSecurityTradingStatus()),
  };
}

}  // namespace mdp
}  // namespace cme
}  // namespace roq
