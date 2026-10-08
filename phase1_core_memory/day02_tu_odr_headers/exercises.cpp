// Day 02: Output questions + debugging. PREDICT before running!
// This program has TWO translation units on purpose:
//   g++ -std=c++20 -O0 exercises.cpp odr_other.cpp -o ex0 && ./ex0
//   g++ -std=c++20 -O2 exercises.cpp odr_other.cpp -o ex2 && ./ex2     <- compare Q3!
// Debugging: add -DBUG3 to both commands.

#include <iostream>

// ── Q1 ──────────────────────────────────────────────────────────────────────
int never_defined();          // declared, NO definition anywhere in the program

void q1() {
    std::cout << "Q1: " << sizeof(never_defined()) << '\n';
}

// ── Q2 ──────────────────────────────────────────────────────────────────────
// Imagine both lines came from the same header, pasted into both TUs.
const int kLimit = 100;
inline const int kInlineLimit = 100;
const int* other_klimit_addr();        // defined in odr_other.cpp
const int* other_kinline_addr();

void q2() {
    std::cout << "Q2: " << (&kLimit == other_klimit_addr()) << ' '
              << (&kInlineLimit == other_kinline_addr()) << '\n';
}

// ── Q3 ──────────────────────────────────────────────────────────────────────
// odr_other.cpp ALSO defines inline int fee_bps(), but returns 2. (ODR violation!)
inline int fee_bps() { return 1; }
int other_fee();                        // defined in odr_other.cpp, returns fee_bps()

void q3() {
    std::cout << "Q3: " << fee_bps() << ' ' << other_fee() << '\n';
}

// ── Q4 ──────────────────────────────────────────────────────────────────────
int twice(int);
int twice(int);
int twice(int x) { return 2 * x; }

void q4() {
    std::cout << "Q4: " << twice(21) << '\n';
}

// ── Q5 ──────────────────────────────────────────────────────────────────────
extern int counter;                    // defined in odr_other.cpp as `int counter = 7;`
int bump();                            // ++counter

void q5() {
    std::cout << "Q5: " << counter << ' ' << bump() << ' ' << counter << '\n';
}

// ── Debugging B3: why does the "shared" counter disagree? ────────────────────
#ifdef BUG3
static int g_orders = 0;               // "from a header", so odr_other.cpp has the same line
void record_order_other();
int orders_seen_by_other();

void bug3() {
    ++g_orders;            // record an order here
    record_order_other();  // ...and one there
    std::cout << "B3: here=" << g_orders << " other=" << orders_seen_by_other()
              << "  (expected 2 and 2)\n";
}
#endif

int main() {
    q1();
    q2();
    q3();
    q4();
    q5();
#ifdef BUG3
    bug3();
#endif
}
