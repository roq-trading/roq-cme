/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/deribit/sbe/parser.hpp"

#include "roq/logging.hpp"

#include "roq/debug/hex/message.hpp"

#include "roq/deribit/sbe/frame.hpp"
#include "roq/deribit/sbe/utils.hpp"

#include <iostream>

using namespace std::literals;

namespace roq {
namespace deribit {
namespace sbe {

bool Parser::dispatch(Handler &handler, std::span<std::byte const> const &buffer, TraceInfo const &trace_info) {
  auto result = true;
  if (Frame::parse(buffer, [&](auto &frame) {
        // log::debug("skip frame"sv);
        auto tmp = buffer.subspan(Frame::size());
        // sbe headers are not const-safe
        std::span message{reinterpret_cast<char *>(const_cast<std::byte *>(std::data(tmp))), std::size(tmp)};
        while (result) {
          // log::debug("message: size={}"sv, std::size(message));
          deribit_multicast::MessageHeader header{std::data(message), std::size(message)};
          auto template_id = header.templateId();
          // log::debug("template_id={}"sv, template_id);
          switch (header.templateId()) {
            case 1000: {
              deribit_multicast::Instrument instrument{std::data(message), std::size(message)};
              auto length = compute_length(instrument);
              // log::debug("--> instrument: length={}"sv, length);
              // log::info<5>("instrument={}"sv, instrument);
              instrument.sbeRewind();  // note! important
              create_trace_and_dispatch(handler, trace_info, instrument, frame);
              message = message.subspan(length);
              break;
            }
            case 1001: {
              deribit_multicast::Book book{std::data(message), std::size(message)};
              auto length = compute_length(book);
              // log::debug("--> book: length={}"sv, length);
              // log::info<5>("book={}"sv, book);
              book.sbeRewind();  // note! important
              create_trace_and_dispatch(handler, trace_info, book, frame);
              message = message.subspan(length);
              break;
            }
            case 1002: {
              deribit_multicast::Trades trades{std::data(message), std::size(message)};
              auto length = compute_length(trades);
              // log::debug("--> trades: length={}"sv, length);
              // log::info<5>("trades={}"sv, trades);
              trades.sbeRewind();  // note! important
              create_trace_and_dispatch(handler, trace_info, trades, frame);
              message = message.subspan(length);
              break;
            }
            case 1003: {
              deribit_multicast::Ticker ticker{std::data(message), std::size(message)};
              auto length = compute_length(ticker);
              // log::debug("--> ticker: length={}"sv, length);
              // log::info<5>("ticker={}"sv, ticker);
              ticker.sbeRewind();  // note! important
              create_trace_and_dispatch(handler, trace_info, ticker, frame);
              message = message.subspan(length);
              break;
            }
            case 1004: {
              deribit_multicast::Snapshot snapshot{std::data(message), std::size(message)};
              auto length = compute_length(snapshot);
              // log::debug("--> snapshot: length={}"sv, length);
              // std::cerr << snapshot << std::endl;
              // log::info<5>("snapshot={}"sv, snapshot);
              snapshot.sbeRewind();  // note! important
              create_trace_and_dispatch(handler, trace_info, snapshot, frame);
              message = message.subspan(length);
              break;
            }
            default: {
              log::warn("Unknown template_id={}"sv, template_id);
              result = false;
              debug::hex::Message message{buffer};
              log::info<5>("DEBUG: {}"sv, message);
              return;
            }
          }
          if (std::empty(message)) {
            // log::debug("done!"sv);
            break;
          }
          // XXX something wrong with Snapshot...
          if (std::size(message) < 12) {  // size of header
            log::warn("remaining data: length={}"sv, std::size(message));
            break;
          }
        }
      })) {
  } else {
    return false;
  }
  return result;
}

}  // namespace sbe
}  // namespace deribit
}  // namespace roq
