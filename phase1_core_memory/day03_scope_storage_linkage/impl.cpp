// Day 03: Namespaces, scope, storage duration, linkage, static, extern. Driver + tests.
// Build:  g++ -std=c++20 -O2 -Wall -Wextra -pedantic impl.cpp oms.cpp risk.cpp -o impl
// Run:    ./impl
//
// Tests FAIL until you implement the TODOs in oms.h, oms.cpp and risk.cpp.

#include "oms.h"
#include "risk.h"

#include <cassert>
#include <cstdint>
#include <iostream>
#include <thread>

// Function-local static: lazy, once, same object every time.
static void test_venue() {
    assert(oms::venue_init_count() == 0);          // NOT constructed at program start
    const oms::Venue& a = oms::primary_venue();
    const oms::Venue& b = oms::primary_venue();
    assert(&a == &b);
    assert(a.name == "NSE" && a.id == 1);
    assert(oms::venue_init_count() == 1);          // constructed exactly once
}

// Internal-linkage state behind an external-linkage API + an `extern` global.
static void test_order_ids() {
    assert(oms::session_id == 0);
    assert(oms::next_order_id() == 1);
    assert(oms::next_order_id() == 2);
    assert(oms::next_order_id() == 3);

    oms::reset_order_ids(1000);
    assert(oms::next_order_id() == 1000);
    assert(oms::next_order_id() == 1001);
    assert(oms::ids_issued() == 5);                // reset does not clear the total

    oms::session_id = 7;                           // we can touch it: external linkage
    oms::reset_order_ids();                        // default argument -> 1
    assert(oms::next_order_id() == ((std::uint64_t{7} << 32) | 1));
    assert(oms::next_order_id() == ((std::uint64_t{7} << 32) | 2));
    assert(oms::ids_issued() == 7);
    oms::session_id = 0;
}

// Thread storage duration.
static void test_thread_counter() {
    assert(oms::bump_thread_counter() == 1);
    assert(oms::bump_thread_counter() == 2);
    assert(oms::bump_thread_counter() == 3);

    int seen_by_other_thread = -1;
    std::thread t([&] { seen_by_other_thread = oms::bump_thread_counter(); });
    t.join();
    assert(seen_by_other_thread == 1);             // the new thread started from its own 0
    assert(oms::bump_thread_counter() == 4);       // ours was untouched
}

// Static data member + automatic storage duration + block scope.
static void test_live_orders() {
    assert(oms::Order::live() == 0);
    {
        oms::Order a;
        oms::Order b;
        assert(oms::Order::live() == 2);
        {
            oms::Order c = a;                      // copy construction
            assert(oms::Order::live() == 3);
        }                                          // c destroyed at the end of its block
        assert(oms::Order::live() == 2);
    }
    assert(oms::Order::live() == 0);
}

// Same private name (`g_count`) in two TUs must not clash.
static void test_risk() {
    using risk::check_qty;                         // using-declaration: brings in ONE name
    assert(check_qty(100));
    assert(!check_qty(0));
    assert(!check_qty(-5));
    assert(check_qty(risk::kMaxOrderQty));
    assert(!check_qty(risk::kMaxOrderQty + 1));
    assert(risk::checks_done() == 5);
    assert(oms::ids_issued() == 7);                // oms's g_count is a different object
}

int main() {
    test_venue();                                  // keep first: checks lazy initialization
    test_order_ids();
    test_thread_counter();
    test_live_orders();
    test_risk();
    std::cout << "All Day 03 tests passed\n";
}

/* ── Part B observations (fill in) ────────────────────────────────────────────
 B.1 both g_count made external (no static / no anonymous namespace):
 B.2 `extern std::uint64_t g_seq;` used from impl.cpp while g_seq is internal:
 B.3 nm -C oms.o: which names are missing or lowercase, and why:
 B.4 local static moved to namespace scope: when is the Venue constructed now?
 B.5 thread_local removed from bump_thread_counter: which assert fires?
 B.6 Order copy constructor deleted (let the compiler generate it): which assert fires, and why?
*/
