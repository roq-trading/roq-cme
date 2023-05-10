/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/cme/shared.hpp"

#include "roq/logging.hpp"

#include "roq/utils/safe_cast.hpp"

#include "roq/cme/secdef/config_reader.hpp"

using namespace std::literals;

namespace roq {
namespace cme {

// === CONSTANTS ===

namespace {
auto const BUFFER_SIZE = size_t{4096};
}

// === HELPERS ===

namespace {
template <typename R>
R read_ilink_config(auto const &filename) {
  R result;
  if (!std::empty(filename)) {
    struct Handler final : public ilink::ConfigReader::Handler {
      explicit Handler(R &result) : result_{result} {}

     protected:
      void operator()(uint32_t market_segment_id, ilink::ConfigReader::MarketSegment const &market_segment) override {
        result_.try_emplace(market_segment_id, market_segment);
      }

     private:
      R &result_;
    } handler{result};
    ilink::ConfigReader::read(handler, filename);
  }
  return result;
}

template <typename T, typename D>
void read_secdef(T &securities, D &dispatcher, auto &settings) {
  auto config_file = settings.common.secdef_config_file;
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
      auto security = tools::Security{
          .exchange = item.exchange,
          .symbol = item.symbol,
          .display_factor = item.display_factor,
          .discard = discard,
      };
      securities_.try_emplace(item.security_id, std::move(security));
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

Shared::Shared(server::Dispatcher &dispatcher, Settings const &settings)
    : dispatcher_{dispatcher}, settings{settings}, mdp_config_{settings.multicast.config_file},
      ilink_config_{read_ilink_config<decltype(ilink_config_)>(settings.ilink.config_file)}, buffer(BUFFER_SIZE) {
  read_secdef(securities, dispatcher, settings);
}

std::pair<std::string, uint16_t> Shared::get_multicast_config(
    std::string_view const &channel_id, mdp::ConnectionType type, Priority priority) const {
  std::pair<std::string, uint16_t> result;
  if (mdp_config_.find(channel_id, type, priority, [&](auto &connection) {
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

}  // namespace cme
}  // namespace roq
