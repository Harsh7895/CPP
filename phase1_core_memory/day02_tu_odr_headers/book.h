// book.h: #pragma once style
#pragma once

// FORWARD DECLARATION instead of #include "instrument.h".
// We only hold a pointer, so the compiler doesn't need Instrument's size or members here.
// Every file that includes book.h no longer pays for parsing <string> etc. through this header.
struct Instrument;

class OrderBook {
public:
    explicit OrderBook(const Instrument* instrument) : instrument_(instrument) {}

    // Needs Instrument's members (tick_size), so it CAN'T be defined here
    // with only a forward declaration. Defined in book.cpp.
    long long price_to_ticks(double price) const;

    const Instrument* instrument() const { return instrument_; }   // fine: only copies a pointer

private:
    const Instrument* instrument_;
};
