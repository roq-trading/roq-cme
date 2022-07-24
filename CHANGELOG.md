# Change Log

All notable changes will be documented in this file.

## Head

## 0.8.6 &ndash; 2022-07-18

### Fixed

* Auto-reconnect after exchange maintenance (#246)
* MbP snapshot did not contain `exchange_time_utc` (#234)

## 0.8.5 &ndash; 2022-06-06

### Added

* Flag to disconnect market data connection after some time (#224)

## 0.8.4 &ndash; 2022-05-14

### Added

* Optional multicast connection (#202)

### Fixed

* Undocumented field for OrderCancelReject (<35> = ' 9') (#204)
* Ticker fields may contain non-numeric values (#203)

## 0.8.3 &ndash; 2022-03-22

### Changed

* Support order download (#39)

## 0.8.2 &ndash; 2022-02-18

### Changed

* Make termination opt-in for MarketDataRequestReject (#182)
* Drop `exec_type=CANCELED`/`ord_status=CANCELED` (#143)

## 0.8.1 &ndash; 2022-01-16

## 0.8.0 &ndash; 2022-01-12

### Fixed

* WS TopOfBook not working if FIX seeing a symbol first (#157)

### Changed

* WebSocket now using a rate-limited queue for subscriptions
* WebSocket subscribing ticker.{instrument}.raw will break after 2022-01-15 (#155)

## 0.7.9 &ndash; 2021-12-08

### Changed

* Price inversion is possible (#115)

### Fixed

* Incorrect hashing for mutliple signatures within 1ms (#133)

## 0.7.8 &ndash; 2021-11-02

### Added

* Add exchange sequence number to `MarketByPrice` and `MarketByOrder` (#101)
* Add `max_trade_vol` and `trade_vol_step_size` to ReferenceData (#100)

### Changed

* The jsonrpc parser now expects `server::TraceInfo`
* Move cache utilities to API (#111)
* Remove custom literals (#110)
* Interface to support binary data from web::socket
* Round quantity and price for CreateOrder/ModifyOrder (#102)
* ReferenceData currencies should follow FX conventions (#99)
* Replace `snapshot` (bool) with `update_type` (UpdateType) (#97)
* Moved signature handling to tools library (chore)
* Parse `Portfolio.security_keys_enabled` (JSON, undoc) (chore)
* Resubscribe market data when observing "bad state" (#86)
* Report last traded as the aggregate of all fills (#84)
* DropCopy doesn't need to subscribe positions (#79)
* Now subscribing to position updates (FIX) (#79)

## 0.7.7 &ndash; 2021-09-20

### Changed

* Reject message with `rate_limit_exceeded` (#73)
* Reduced logging (#72)
* Logon timeout (#70)
* Subscribe to reference data changes (FIX) (#65)
* Added HTTP `request_id` (#55)
* Base64 clean-up to support web-safe (#43)

## 0.7.6 &ndash; 2021-09-02

### Changed

* Download orders (#39)
* FIX message now logged for parse errors (#36)
* OrderCancelReject (sometimes) has undoc tag 151 and 6 (#36)
* New order management interface (#25)

### Fixed

* `OrderCancelReject` use undocumented tags 151 and 6 (#36)
* `TopOfBook` was only published from the primary WS connection due to
  non-shared multiplier. (This bug was introduced with release 0.7.3.)

## 0.7.5 &ndash; 2021-08-08

## 0.7.4 &ndash; 2021-07-20

### Added

* Subsribe `user.orders.any.any.raw` and `user.trades.any.any.raw`

## 0.7.3 &ndash; 2021-07-06

### Changed

* `TopOfBook` now convert `Quote.best_bid_amount` and `Quote.best_ask_amount`
  to number of contracts

## 0.7.2 &ndash; 2021-06-20

## 0.7.1 &ndash; 2021-05-30

## 0.7.0 &ndash; 2021-04-15

### Added

* Multi-account support

### Changed

* Streams to support load-balancing
* Using web-socket for funds and positions

## 0.6.1 &ndash; 2021-02-19

## 0.6.0 &ndash; 2021-02-02

## 0.5.0 &ndash; 2020-12-04

## 0.4.5 &ndash; 2020-11-09

## 0.4.4 &ndash; 2020-09-20

### Changed

* Default config excludes `"USDT-.*"` due to missing market data on testnet


## 0.4.3 &ndash; 2020-09-02

## 0.4.2 &ndash; 2020-07-27

### Removed

* Automake support

## 0.4.1 &ndash; 2020-07-17

## 0.4.0 &ndash; 2020-06-30

### Added

* `fix::ExecutionReport::SecondaryExecID` (tag 527)
* `json::Instrument::block_trade_commission`

## 0.3.9 &ndash; 2020-06-09

## 0.3.8 &ndash; 2020-06-06

## 0.3.7 &ndash; 2020-05-27

### Added

* `SessionStatistics` now used to propagate the index value
  (`index_value`), funding rate (`margin_rate`) and mark price
  (`pre_settlement_price`) from `fix::MarketDataIncrementalRefresh`

## 0.3.6 &ndash; 2020-05-02

## 0.3.5 &ndash; 2020-04-22

### Added

* `fix::SecurityStatus` and `fix::SecurityStatusRequst` (but not yet
   using)

### Changed

* `fix::Reject` "connection too slow" will now cause connection reset

## 0.3.4 &ndash; 2020-04-08

### Added

* `OrderUpdate::execution_instruction` support
* `json::Ticker::delivery_price` parsing
* Web-Socket support (for download)

### Removed

* `OrderUpdate::commissions`

### Changed

* New download state management

## 0.3.3 &ndash; 2020-03-04
