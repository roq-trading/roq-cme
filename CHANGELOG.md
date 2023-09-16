# Change Log

All notable changes will be documented in this file.

## Head

### Fixed

* TCP fragmentation could cause freeze (#392)
* `CancelAllOrders` could cause unhandled exception (#387)

### Changed

* `OrderRequestID` has no space for version after `order_id` increased to 48 bits

## 0.9.6 &ndash; 2023-07-22

## 0.9.5 &ndash; 2023-06-12

## 0.9.4 &ndash; 2023-05-04

### Fixed

* `OrderUpdate` incorrect for `MBOUpdate` with changed price and quantity (#338)

## 0.9.3 &ndash; 2023-03-20

### Changed

* [BREAKING CHANGE] `MarketByOrderUpdate` and `MBOUpdate` has been changed to support CME's TradeSummary (#322)

## 0.9.2 &ndash; 2023-02-22

## 0.9.1 &ndash; 2023-01-12

## 0.9.0 &ndash; 2022-12-22

## 0.8.9 &ndash; 2022-11-14

## 0.8.8 &ndash; 2022-10-04

## 0.8.7 &ndash; 2022-08-22
