/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/cme/config.hpp"

#include <utility>

#include "roq/logging.hpp"

#include "roq/cme/flags/config.hpp"

using namespace std::literals;

namespace roq {
namespace cme {

// === CONSTANTS ===

namespace {
auto const SUPPORTS = Mask{
    SupportType::REFERENCE_DATA,
    SupportType::MARKET_STATUS,
    SupportType::TOP_OF_BOOK,
    SupportType::MARKET_BY_PRICE,
    SupportType::TRADE_SUMMARY,
    SupportType::STATISTICS,
};
auto const MBP_MAX_DEPTH = 10;
auto const MBP_ALLOW_REMOVE_NON_EXISTING = true;
auto const OMS_REQUEST_ID_TYPE = RequestIdType::BASE64;
}  // namespace

// === IMPLEMENTATION ===

Config::Config() {
  server::ConfigReader::parse_file(*this);
  log::info<1>("config={}"sv, *this);
}

Account const &Config::get_master_account() const {
  return master_account_;
}

std::string const &Config::get_access_key(Account const &account) const {
  auto iter = accounts.find(account);
  if (iter == std::end(accounts))
    log::fatal(R"(Unknown account="{}")"sv, account);
  return (*iter).second.login;
}

std::string const &Config::get_access_secret(Account const &account) const {
  auto iter = accounts.find(account);
  if (iter == std::end(accounts))
    log::fatal(R"(Unknown account="{}")"sv, account);
  return (*iter).second.secret;
}

void Config::dispatch(server::Config::Handler &handler) const {
  handler(flags::Config::exchange());
  handler(symbols);
  for (auto &iter : accounts)
    handler(iter.second);
  for (auto &user : users)
    handler(user);
  GatewaySettings gateway_settings{
      .supports = SUPPORTS,
      .mbp_max_depth = MBP_MAX_DEPTH,
      .mbp_tick_size_multiplier = NaN,
      .mbp_min_trade_vol_multiplier = NaN,
      .mbp_allow_remove_non_existing = MBP_ALLOW_REMOVE_NON_EXISTING,
      .mbp_allow_price_inversion = flags::Config::mbp_allow_price_inversion(),
      .mbp_checksum = server::Flags::cache_mbp_checksum(),
      .oms_download_has_state = {},
      .oms_download_has_routing_id = {},
      .oms_request_id_type = OMS_REQUEST_ID_TYPE,
  };
  handler(gateway_settings);
  for (auto &iter : rate_limits)
    handler(iter.second);
}

void Config::operator()(server::Symbols &&symbols) {
  (*this).symbols = std::move(symbols);
}

void Config::operator()(server::Account &&account) {
  if (account.master)
    master_account_ = account.name;
  accounts.emplace(account.name, std::move(account));
}

void Config::operator()(server::User &&user) {
  users.emplace_back(std::move(user));
}

void Config::operator()(server::RateLimit &&rate_limit) {
  rate_limits.emplace(rate_limit.name, std::move(rate_limit));
}

void Config::operator()(std::string_view const &key, toml::node &) {
  log::warn(R"(Unexpected: key="{}")"sv, key);
}

}  // namespace cme
}  // namespace roq
