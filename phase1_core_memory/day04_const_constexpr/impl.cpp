// Day 04: const, constexpr, consteval, constinit, mutable
// Build:  g++ -std=c++20 -O2 -Wall -Wextra -pedantic impl.cpp -o impl
// Run:    ./impl
//
// Do the steps in order (README section 3 explains each one).
// The tests in main() FAIL until you fill in every TODO.

#include <array>
#include <cassert>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

// ─────────────────────────────────────────────────────────────────────────────
// STEP 1: a constexpr function
// Return 10 to the power n (n is between 0 and 18). Use a simple loop.
//   pow10(0) == 1,  pow10(3) == 1000
// ─────────────────────────────────────────────────────────────────────────────
constexpr long long pow10(int n) {
    // TODO
    (void)n;
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// STEP 2: a lookup table that the COMPILER builds
// Fill t[i] = pow10(i) for every i. kPow10 below is computed while compiling,
// so at run time it's just ready-made numbers in memory.
// ─────────────────────────────────────────────────────────────────────────────
constexpr std::array<long long, 19> make_pow10_table() {
    std::array<long long, 19> t{};   // 19 zeros
    // TODO: fill the table
    return t;
}

constexpr std::array<long long, 19> kPow10 = make_pow10_table();

// ─────────────────────────────────────────────────────────────────────────────
// STEP 3: use the table
// Prices are stored as whole numbers with a fixed number of decimals.
//   2950.35 with 2 decimals  -> 295035
//   2950.35 with 4 decimals  -> 29503500
// rescale() converts from `from_dec` decimals to `to_dec` decimals.
// Going to FEWER decimals just cuts off the extra digits.
//   rescale(295035, 2, 4)   == 29503500
//   rescale(29503567, 4, 2) == 295035
// Use kPow10. Do NOT use std::pow (it works with doubles and isn't constexpr).
// ─────────────────────────────────────────────────────────────────────────────
constexpr long long rescale(long long value, int from_dec, int to_dec) {
    // TODO
    (void)value; (void)from_dec; (void)to_dec;
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// STEP 4: consteval: this function is ONLY allowed to run at compile time
// Return how many "units" make 1.0 for the given number of decimals.
//   units_per_one(2) == 100,  units_per_one(4) == 10000
// ─────────────────────────────────────────────────────────────────────────────
consteval long long units_per_one(int decimals) {
    // TODO
    (void)decimals;
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// STEP 5: constinit (already written: read it, then do Part B.4 in the README)
// The value is set at compile time, but the variable is NOT const: you can change it later.
// ─────────────────────────────────────────────────────────────────────────────
constexpr int default_max_orders_per_sec() { return 500; }
constinit int g_max_orders_per_sec = default_max_orders_per_sec();

// ─────────────────────────────────────────────────────────────────────────────
// STEP 6: const member functions + mutable
// tick_size() is a CONST function (it promises not to change the object),
// but we still want to count how many times it is called.
// TODO: add a counter member. The compiler will refuse until you use the right keyword.
// ─────────────────────────────────────────────────────────────────────────────
class Instrument {
public:
    Instrument(std::string symbol, long long tick_size)
        : symbol_(std::move(symbol)), tick_size_(tick_size) {}

    const std::string& symbol() const { return symbol_; }

    long long tick_size() const {
        // TODO: count this call, then return the tick size
        return 0;
    }

    int lookup_count() const {
        // TODO
        return -1;
    }

private:
    std::string symbol_;
    long long tick_size_;
    // TODO: add the counter here
};

// ─────────────────────────────────────────────────────────────────────────────
// STEP 7: pass by const reference
// Add up all the prices. The parameter is `const std::vector<long long>&`:
//   &      -> no copy is made (fast)
//   const  -> this function promises not to change the caller's vector
// ─────────────────────────────────────────────────────────────────────────────
long long sum_prices(const std::vector<long long>& prices) {
    // TODO
    (void)prices;
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// Tests
// ─────────────────────────────────────────────────────────────────────────────
int main() {
    // Step 1
    assert(pow10(0) == 1);
    assert(pow10(1) == 10);
    assert(pow10(6) == 1'000'000);
    assert(pow10(18) == 1'000'000'000'000'000'000);

    // Step 2
    assert(kPow10[0] == 1);
    assert(kPow10[5] == 100'000);
    assert(kPow10[18] == pow10(18));

    // Step 3
    assert(rescale(295035, 2, 4) == 29503500);
    assert(rescale(29503500, 4, 2) == 295035);
    assert(rescale(29503567, 4, 2) == 295035);   // extra digits are cut off
    assert(rescale(7, 2, 2) == 7);                // same decimals: unchanged

    // Step 4
    assert(units_per_one(2) == 100);
    assert(units_per_one(4) == 10'000);

    // Step 5
    assert(g_max_orders_per_sec == 500);
    g_max_orders_per_sec = 1000;                  // allowed: constinit is NOT const
    assert(g_max_orders_per_sec == 1000);

    // Step 6
    const Instrument es("ES", 25);                // a CONST object: only const functions can be called
    assert(es.symbol() == "ES");
    assert(es.tick_size() == 25);
    assert(es.tick_size() == 25);
    assert(es.tick_size() == 25);
    assert(es.lookup_count() == 3);

    // Step 7
    std::vector<long long> prices{100, 250, 650};
    assert(sum_prices(prices) == 1000);
    assert(sum_prices({}) == 0);

    std::cout << "All Day 04 tests passed\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// STEP 8 (final check): once all the tests pass, UNCOMMENT these lines.
// static_assert is checked BY THE COMPILER. If the file still compiles,
// your functions really do run at compile time.
// ─────────────────────────────────────────────────────────────────────────────
// static_assert(pow10(3) == 1000);
// static_assert(kPow10[18] == 1'000'000'000'000'000'000);
// static_assert(rescale(295035, 2, 4) == 29503500);
// static_assert(units_per_one(4) == 10'000);

/* ── Part B observations (fill in) ────────────────────────────────────────────
 B.1 remove `const` from tick_size(): what error, and on which line?
 B.2 remove `mutable` from your counter:
 B.3 call units_per_one(n) where `int n = 2;` is a normal variable:
 B.4 make default_max_orders_per_sec() a normal (non-constexpr) function:
 B.5 change g_max_orders_per_sec from constinit to constexpr: which line breaks now?
*/
