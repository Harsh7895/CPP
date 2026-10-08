// Day 02: Translation units, declarations vs definitions, ODR, headers. Driver + tests.
// Build:  g++ -std=c++20 -O2 -Wall -Wextra -pedantic impl.cpp instrument.cpp book.cpp -o impl
// Run:    ./impl
//
// Three TUs: impl.cpp, instrument.cpp, book.cpp. Tests FAIL until you implement the TODOs.

#include "instrument.h"
#include "book.h"
// Part B.1: add a second `#include "instrument.h"` here, then remove its include guard.

#include <cassert>
#include <iostream>
#include <string>

static void test_whole_lots() {
    assert(whole_lots(250, 100) == 2);
    assert(whole_lots(99, 100) == 0);
    assert(whole_lots(300, 100) == 3);
    assert(whole_lots(10, 0) == 0);
    assert(whole_lots(10, -5) == 0);
}

static void test_registry() {
    assert(registered_count() == 0);
    assert(register_instrument({"ES", 0.25, 1}));
    assert(register_instrument({"RELIANCE", 0.05, 1}));
    assert(register_instrument({"NIFTY", 0.05, 75}));
    assert(!register_instrument({"ES", 0.5, 1}));          // duplicate rejected
    assert(registered_count() == 3);

    const long long before = g_lookup_count;
    const Instrument* es = find_instrument("ES");
    assert(es != nullptr);
    assert(es->tick_size == 0.25);
    assert(find_instrument("NOPE") == nullptr);
    assert(g_lookup_count == before + 2);                  // hits AND misses are counted

    assert(es == find_instrument("ES"));                   // stable pointer to the same entry
}

static void test_order_book() {
    const Instrument* es = find_instrument("ES");
    OrderBook book(es);
    assert(book.instrument() == es);
    assert(book.price_to_ticks(4501.25) == 18005);
    assert(book.price_to_ticks(4501.30) == 18005);          // rounds to nearest tick
    assert(book.price_to_ticks(4501.40) == 18006);

    OrderBook rel(find_instrument("RELIANCE"));
    assert(rel.price_to_ticks(2950.35) == 59007);
}

static void test_capacity() {
    for (int i = registered_count(); i < kMaxInstruments; ++i)
        assert(register_instrument({"SYM" + std::to_string(i), 0.01, 1}));
    assert(registered_count() == kMaxInstruments);
    assert(!register_instrument({"ONE_TOO_MANY", 0.01, 1}));   // table full
    assert(registered_count() == kMaxInstruments);
}

int main() {
    test_whole_lots();
    test_registry();
    test_order_book();
    test_capacity();
    std::cout << "All Day 02 tests passed\n";
}

/* ── Part B observations (fill in: error text + which stage: compiler or linker) ──
 B.1 no include guard, included twice:
 B.2 definition `long long g_lookup_count = 0;` moved into the header:
 B.3 `inline` removed from whole_lots:
 B.4 `inline` removed from kMaxInstruments (does it still link? why?):
 B.5 `struct Instrument;` forward decl + calling instrument_->tick_size in book.h:
 B.6 preprocessed line count of impl.cpp with/without <string> reachable via book.h:
*/
