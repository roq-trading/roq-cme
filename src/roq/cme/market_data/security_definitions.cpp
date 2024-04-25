/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/cme/market_data/security_definitions.hpp"

#include "roq/logging.hpp"

#include "roq/utils/safe_cast.hpp"

#include "roq/cme/secdef/config_reader.hpp"

using namespace std::literals;

namespace roq {
namespace cme {
namespace market_data {

// === HELPERS ===

namespace {
template <typename T, typename SG, typename MS, typename D>
struct Handler final : public secdef::ConfigReader::Handler {
  Handler(T &securities, SG &security_groups, MS &market_segments, D &dispatcher)
      : securities_{securities}, security_groups_{security_groups}, market_segments_{market_segments},
        dispatcher_{dispatcher} {}

  void operator()(secdef::ConfigReader::SecDef const &item) override {
    auto discard = dispatcher_.discard_symbol(item.symbol);
    // note! it's too much -- always discard
    if (discard)
      return;  // note!
    auto security = tools::Security{
        .exchange = item.exchange,
        .symbol = item.symbol,
        .display_factor = item.display_factor,
        .discard = discard,
    };
    securities_.try_emplace(item.security_id, std::move(security));
    security_groups_[item.asset].emplace(item.security_id);
    market_segments_[item.market_segment_id].try_emplace(item.symbol, item.security_id);
  }

 private:
  T &securities_;
  SG &security_groups_;
  MS &market_segments_;
  D &dispatcher_;
};
}  // namespace

// === IMPLEMENTATION ===

SecurityDefinitions::SecurityDefinitions(Dispatcher &dispatcher, std::string_view const &secdef_config_file) {
  if (std::empty(secdef_config_file))
    return;
  log::info(R"(Reading instrument definitions from "{}"...)"sv, secdef_config_file);
  Handler handler{securities_, security_groups_, market_segments_, dispatcher};
  secdef::ConfigReader::read(handler, secdef_config_file);
}

}  // namespace market_data
}  // namespace cme
}  // namespace roq
