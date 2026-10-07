// pricing.h: tiny pricing library (Day 01)
#pragma once

// Declarations only: definitions live in pricing.cpp (a different translation unit).
double mid_price(double bid, double ask);
double spread_bps(double bid, double ask);
long long to_ticks(double price, double tick_size);

// Defined IN the header, so it must be `inline` (included by both impl.cpp and pricing.cpp).
// Part D task: remove `inline`, rebuild, read the error, put it back.
inline bool is_crossed(double bid, double ask) {
    // TODO: return true if the book is crossed/locked (bid >= ask)
    (void)bid; (void)ask;
    return false;
}
