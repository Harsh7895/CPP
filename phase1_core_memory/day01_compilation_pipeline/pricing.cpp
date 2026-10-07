// pricing.cpp: implement these from scratch (Day 01)
#include "pricing.h"

#include <cmath>

double mid_price(double bid, double ask) {
    // TODO
    (void)bid; (void)ask;
    return 0.0;
}

double spread_bps(double bid, double ask) {
    // TODO: (ask - bid) / mid * 10'000
    (void)bid; (void)ask;
    return 0.0;
}

long long to_ticks(double price, double tick_size) {
    // TODO: integer tick count, rounded to NEAREST.
    // Careful: 100.07 / 0.01 is not exactly 10007.0 in binary floating point.
    (void)price; (void)tick_size;
    return 0;
}
