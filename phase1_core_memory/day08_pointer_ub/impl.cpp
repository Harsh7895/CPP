// Day 08: safe tools against pointer UB: bit casts, wire parsing, handles, safe erase
// Build:  g++ -std=c++20 -O2 -Wall -Wextra -pedantic impl.cpp -o impl
// Run:    ./impl
//
// Do the steps in order (README section 3 explains each one).
// The tests in main() FAIL until you fill in every TODO.
// Rule for today: NO reinterpret_cast anywhere in your code.

#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <vector>

static_assert(std::endian::native == std::endian::little,
              "this lesson assumes a little-endian CPU (x86-64 / ARM64)");

using Price = std::int64_t;
using Qty   = std::int32_t;

// ─────────────────────────────────────────────────────────────────────────────
// STEP 1: look at a float's bits safely.
//   float_bits:      use std::memcpy
//   float_from_bits: use std::bit_cast (from <bit>)
// ─────────────────────────────────────────────────────────────────────────────
std::uint32_t float_bits(float f) {
    // TODO
    (void)f;
    return 0;
}

float float_from_bits(std::uint32_t u) {
    // TODO
    (void)u;
    return 0.0f;
}

// ─────────────────────────────────────────────────────────────────────────────
// STEP 2: read little-endian integers from ANY byte address (aligned or not).
// ─────────────────────────────────────────────────────────────────────────────
std::int64_t load_i64(const unsigned char* p) {
    // TODO
    (void)p;
    return 0;
}

std::int32_t load_i32(const unsigned char* p) {
    // TODO
    (void)p;
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// STEP 3: parse a 16-byte quote message.
//   [0..4) symbol (4 chars, no '\0')   [4..12) price int64   [12..16) qty int32
//   len < 16 -> return false, don't touch *out.
// ─────────────────────────────────────────────────────────────────────────────
inline constexpr std::size_t kQuoteWireSize = 16;

struct Quote {
    char  sym[5];   // 4 chars + '\0' (you add it)
    Price px;
    Qty   qty;
};

bool parse_quote(const unsigned char* buf, std::size_t len, Quote* out) {
    // TODO
    (void)buf;
    (void)len;
    (void)out;
    return false;
}

// ─────────────────────────────────────────────────────────────────────────────
// STEP 4: handles (indices) instead of pointers, so reallocation can't hurt you.
//   add():  append, return the new level's index as a LevelId
//   at():   reference to the level (safe only until the next add()!)
// ─────────────────────────────────────────────────────────────────────────────
struct Level {
    Price px;
    Qty   qty;
};
using LevelId = std::uint32_t;

class LevelStore {
public:
    LevelId add(Price px, Qty qty) {
        // TODO
        (void)px;
        (void)qty;
        return 999;
    }

    Level& at(LevelId id) {
        // TODO
        (void)id;
        static Level dummy{};
        return dummy;
    }

    std::size_t size() const {
        // TODO
        return 0;
    }

private:
    std::vector<Level> levels_;
};

// ─────────────────────────────────────────────────────────────────────────────
// STEP 5: remove orders with qty == 0, keep the rest IN ORDER, return how many removed.
// ─────────────────────────────────────────────────────────────────────────────
struct Order {
    std::uint64_t id;
    Qty           qty;
};

std::size_t remove_filled(std::vector<Order>& v) {
    // TODO
    (void)v;
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// Test helper (given): write the low n bytes of v, little-endian, using shifts only.
// ─────────────────────────────────────────────────────────────────────────────
void encode_le(unsigned char* dst, std::uint64_t v, int n) {
    for (int i = 0; i < n; ++i) dst[i] = static_cast<unsigned char>(v >> (8 * i));
}

int main() {
    // Step 1
    assert(float_bits(1.0f) == 0x3F800000u);
    assert(float_bits(-2.0f) == 0xC0000000u);
    assert(float_bits(0.0f) == 0u);
    assert(float_from_bits(0x3F800000u) == 1.0f);
    assert(float_from_bits(float_bits(2950.35f)) == 2950.35f);   // exact round trip

    // Step 2: values placed at ODD (misaligned) offsets on purpose
    unsigned char raw[32]{};
    encode_le(raw + 3, 0x0102030405060708ull, 8);
    assert(load_i64(raw + 3) == 0x0102030405060708ll);
    encode_le(raw + 13, static_cast<std::uint32_t>(-5), 4);
    assert(load_i32(raw + 13) == -5);

    // Step 3
    unsigned char msg[kQuoteWireSize + 1]{};
    std::memcpy(msg, "ESZ6", 4);                      // symbol, no '\0' on the wire
    encode_le(msg + 4, 295035, 8);                    // 2950.35
    encode_le(msg + 12, 25, 4);
    Quote q{};
    assert(parse_quote(msg, kQuoteWireSize, &q));
    assert(std::strcmp(q.sym, "ESZ6") == 0);
    assert(q.px == 295035 && q.qty == 25);
    Quote untouched{{'X', 'X', 'X', 'X', '\0'}, 7, 7};
    assert(!parse_quote(msg, kQuoteWireSize - 1, &untouched));   // too short
    assert(untouched.px == 7 && untouched.sym[0] == 'X');

    // Step 4
    LevelStore store;
    LevelId a = store.add(295035, 10);
    LevelId b = store.add(295040, 5);
    assert(a == 0 && b == 1 && store.size() == 2);
    for (int i = 0; i < 1000; ++i) store.add(300000 + i, i);    // several reallocations
    assert(store.size() == 1002);
    assert(store.at(a).px == 295035 && store.at(a).qty == 10);  // the handle survived
    assert(store.at(b).px == 295040);
    store.at(b).qty = 0;
    assert(store.at(b).qty == 0);
    assert(store.at(1001).px == 300999);

    // Step 5
    std::vector<Order> orders{{1, 5}, {2, 0}, {3, 0}, {4, 7}, {5, 0}};
    assert(remove_filled(orders) == 3);
    assert(orders.size() == 2 && orders[0].id == 1 && orders[1].id == 4);
    std::vector<Order> all_zero{{1, 0}, {2, 0}};
    assert(remove_filled(all_zero) == 2 && all_zero.empty());
    std::vector<Order> none;
    assert(remove_filled(none) == 0);
    std::vector<Order> adjacent{{1, 0}, {2, 0}, {3, 9}};          // two zeros in a row
    assert(remove_filled(adjacent) == 2 && adjacent.size() == 1 && adjacent[0].id == 3);

    std::cout << "All Day 08 tests passed\n";
}

/* ── Part B: ownership (fill in: OWNING or NON-OWNING, and why) ───────────────
 make_book() return value:
 print_book(const Book* b) parameter:
 Strategy::book:
 b (in the caller):
 What's dangerous about s after `delete b;`?
*/
