// Day 01: Compilation pipeline. Driver + tests.
// Build:  g++ -std=c++20 -O2 -Wall -Wextra -pedantic impl.cpp pricing.cpp -o impl
// Run:    ./impl
//
// All tests FAIL until you implement pricing.cpp and is_crossed() in pricing.h.

#include "pricing.h"

#include <cassert>
#include <cmath>
#include <iostream>

static bool near(double a, double b, double eps = 1e-9) { return std::fabs(a - b) < eps; }

static void test_mid_price() {
    assert(near(mid_price(100.0, 100.5), 100.25));
    assert(near(mid_price(99.99, 100.01), 100.0));
}

static void test_spread_bps() {
    assert(near(spread_bps(99.95, 100.05), 10.0, 1e-6));   // 0.10 / 100 * 1e4
    assert(near(spread_bps(100.0, 100.0), 0.0));
}

static void test_to_ticks() {
    assert(to_ticks(100.00, 0.01) == 10000);
    assert(to_ticks(100.07, 0.01) == 10007);   // naive truncation gives 10006!
    assert(to_ticks(0.29, 0.01) == 29);        // another classic float trap
    assert(to_ticks(4501.25, 0.25) == 18005);
}

static void test_is_crossed() {
    assert(!is_crossed(100.0, 100.5));
    assert(is_crossed(100.5, 100.0));
    assert(is_crossed(100.0, 100.0));   // locked counts as crossed here
}

// ── Mini project (README §9): write your benchmark below ─────────────────────
// static void bench() { ... }

int main() {
    test_mid_price();
    test_spread_bps();
    test_to_ticks();
    test_is_crossed();
    std::cout << "All Day 01 tests passed\n";
    // bench();
}

/* ── Part B/C observations (fill in) ──────────────────────────────────────────
 1. impl.ii line count:
 2. mid_price asm instruction count:
 4. nm impl.o: U symbols =            T symbols =
 5. mangled name of spread_bps:
 6. relocation type seen for mid_price:
 8. linker program invoked:
 9. why `-lpricing impl.o` fails:
10. ldd app_shared shows:
11. size app_static vs app_shared:
 Part D.1 error + stage:
 Part D.2 error + stage:
*/
