// instrument.h: classic include guards (book.h uses #pragma once, so you see both styles)
#ifndef LEARN_CPP_DAY02_INSTRUMENT_H
#define LEARN_CPP_DAY02_INSTRUMENT_H

#include <string>
#include <string_view>

// A class DEFINITION in a header is fine: the ODR allows one identical definition per TU.
struct Instrument {
    std::string symbol;
    double tick_size;
    int lot_size;
};

// C++17 inline variable: ONE object shared by every TU, defined right here in the header.
inline constexpr int kMaxInstruments = 64;

// DECLARATION only (`extern`, no initializer). The single definition lives in instrument.cpp.
extern long long g_lookup_count;

// Function DECLARATIONS: definitions go in instrument.cpp.
bool register_instrument(const Instrument& inst);    // false if duplicate symbol or table full
const Instrument* find_instrument(std::string_view symbol);   // nullptr if unknown; bumps g_lookup_count
int registered_count();

// Function DEFINED in a header: must be `inline` (Part B.3: remove it and see what happens).
inline int whole_lots(int qty, int lot_size) {
    // TODO: number of complete lots in qty. Return 0 if lot_size <= 0.
    (void)qty; (void)lot_size;
    return -1;
}

#endif  // LEARN_CPP_DAY02_INSTRUMENT_H
