/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/cme/shared.hpp"

#include "roq/logging.hpp"

#include "roq/utils/safe_cast.hpp"

#include "roq/cme/flags/common.hpp"
#include "roq/cme/flags/multicast.hpp"

#include "roq/cme/secdef/config_reader.hpp"

using namespace std::literals;

namespace roq {
namespace cme {

// === CONSTANTS ===

namespace {
auto const SLICE = size_t{65536};
}

// === HELPERS ===

namespace {
template <typename T, typename D>
void read_secdef(T &securities, D &dispatcher) {
  auto config_file = flags::Common::secdef_config_file();
  if (std::empty(config_file))
    return;
  log::info(R"(Reading instrument definitions from "{}"... (*** can be very slow ***))"sv, config_file);
  struct Handler final : public secdef::ConfigReader::Handler {
    Handler(T &securities, D &dispatcher) : securities_(securities), dispatcher_(dispatcher) {}
    void operator()(secdef::ConfigReader::SecDef const &item) override {
      auto discard = dispatcher_.discard_symbol(item.symbol);
      // note! it's too much -- always discard
      if (discard)
        return;
      Shared::Security security{
          .exchange = item.exchange,
          .symbol = item.symbol,
          .display_factor = item.display_factor,
          .discard = discard,
      };
      securities_.try_emplace(item.security_id, std::move(security));
      double multiplier = item.multiplier == 0 ? NaN : utils::safe_cast<double>(item.multiplier);
      ReferenceData reference_data{
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
          .discard = security.discard,
      };
      TraceInfo trace_info;
      create_trace_and_dispatch(dispatcher_, trace_info, reference_data, true);
    }

   private:
    T &securities_;
    D &dispatcher_;
  } handler{securities, dispatcher};
  secdef::ConfigReader::read(handler, config_file);
}
}  // namespace

// === IMPLEMENTATION ===

Shared::Shared(server::Dispatcher &dispatcher)
    : multicast_config_{flags::Multicast::multicast_config_file()}, fills(server::Flags::cache_fills_max_depth()),
      bids(server::Flags::cache_mbp_max_depth()), asks(server::Flags::cache_mbp_max_depth()),
      trades(server::Flags::cache_trades_max_depth()),
      statistics(magic_enum::enum_count<StatisticsType>()), dispatcher_{dispatcher},
      rate_limiter{flags::Common::request_limit(), flags::Common::request_limit_interval()}, symbols{SLICE} {
  read_secdef(securities, dispatcher);
}

std::pair<std::string, uint16_t> Shared::get_multicast_config(
    std::string_view const &channel_id, multicast::Type type, Priority priority) const {
  std::pair<std::string, uint16_t> result;
  if (multicast_config_.find(channel_id, type, priority, [&](auto &connection) {
        result = {connection.multicast_address, connection.port};
      })) {
  } else {
    throw RuntimeError{
        R"(Unable to find multicast configuration using channel_id="{}", type={}, priority={})"sv,
        channel_id,
        magic_enum::enum_name(type),
        priority};
  }
  return result;
}

std::string_view Shared::next_request_id() {
  auto request_id = ++request_id_;
  stack_buffer_.clear();
  fmt::format_to(std::back_inserter(stack_buffer_), "roq-{}"sv, request_id);
  return {std::data(stack_buffer_), std::size(stack_buffer_)};
}

}  // namespace cme
}  // namespace roq
