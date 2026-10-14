// Day 05: Exercises. PREDICT the answer BEFORE running!
//
// Output questions:     g++ -std=c++20 -Wall -Wextra exercises.cpp -o ex && ./ex
// "Will it compile?":   add -DC1, -DC2, -DC3, -DC4 or -DC5 (one at a time)
// Bugs:                 add -DBUG1, -DBUG2, -DBUG3 or -DBUG4 (one at a time)

#include <cstdint>
#include <iostream>
#include <type_traits>
#include <vector>

// ── Q1: plain enum values ────────────────────────────────────────────────────
enum Level { Low, Mid = 5, High };

void q1() {
    std::cout << "Q1: " << Low << ' ' << Mid << ' ' << High << '\n';
}

// ── Q2: braces vs parentheses ────────────────────────────────────────────────
void q2() {
    std::vector<int> a(3, 7);
    std::vector<int> b{3, 7};
    std::cout << "Q2: " << a.size() << ' ' << b.size() << ' ' << a[0] << ' ' << b[0] << '\n';
}

// ── Q3: enum sizes ───────────────────────────────────────────────────────────
enum class A { X };
enum class B : std::uint8_t { X };
enum C { CX };

void q3() {
    std::cout << "Q3: " << sizeof(A) << ' ' << sizeof(B) << ' ' << sizeof(C) << '\n';
}

// ── Q4: designated initializers + default member initializers ───────────────
struct Quote {
    int bid = 100;
    int ask = 101;
    int size = 1;
};

void q4() {
    Quote q{.ask = 105};
    std::cout << "Q4: " << q.bid << ' ' << q.ask << ' ' << q.size << '\n';
}

// ── Q5: the most vexing parse ────────────────────────────────────────────────
struct Book {
    Book() { std::cout << "built "; }
};

void q5() {
    std::cout << "Q5: ";
    Book b1;
    Book b2();      // what is this?
    Book b3{};
    (void)b1;
    (void)b3;
    std::cout << '\n';
}

// ── Q6: auto with braces ─────────────────────────────────────────────────────
void q6() {
    auto x{5};
    auto y = {5};
    std::cout << "Q6: " << std::is_same_v<decltype(x), int> << ' '
              << std::is_same_v<decltype(y), int> << '\n';
}

// ── Will it compile? ─────────────────────────────────────────────────────────
#ifdef C1
int c1() { int a{3.7}; return a; }
#endif

#ifdef C2
enum class S { Bv };
int c2() { int x = S::Bv; return x; }
#endif

#ifdef C3
char c3() { char c{65}; return c; }
#endif

#ifdef C4
struct P { int x; int y; };
P c4() { P p{.y = 1, .x = 2}; return p; }
#endif

#ifdef C5
class Q { int bid; };
Q c5() { Q q{1}; return q; }
#endif

// ── Bugs ─────────────────────────────────────────────────────────────────────
#ifdef BUG1
int total_qty(const std::vector<int>& fills) {
    int total;
    for (int f : fills) total += f;
    return total;
}
#endif

#ifdef BUG2
enum class OrdType : std::uint8_t { Market = 1, Limit = 2, Stop = 3 };
int priority(OrdType t) {
    switch (t) {
        case OrdType::Market: return 0;
        case OrdType::Limit:  return 1;
    }
}
#endif

#ifdef BUG3
using Price = std::int64_t;
using Qty   = std::int64_t;
void send_order(Price px, Qty qty) {
    std::cout << "BUG3: sending " << qty << " lots @ " << px << '\n';
}
#endif

#ifdef BUG4
enum class OrdType4 : std::uint8_t { Market = 1, Limit = 2, Stop = 3 };
OrdType4 parse(std::uint8_t byte) { return static_cast<OrdType4>(byte); }
const char* name(OrdType4 t) {
    switch (t) {
        case OrdType4::Market: return "MKT";
        case OrdType4::Limit:  return "LMT";
        case OrdType4::Stop:   return "STP";
    }
    return "???";
}
#endif

int main() {
    q1();
    q2();
    q3();
    q4();
    q5();
    q6();

#ifdef BUG1
    std::cout << "BUG1: " << total_qty({10, 20, 30}) << " (expected 60)\n";
#endif
#ifdef BUG2
    std::cout << "BUG2: " << priority(OrdType::Stop) << '\n';
#endif
#ifdef BUG3
    Qty   q  = 10;
    Price px = 295035;
    send_order(q, px);   // looks fine?
#endif
#ifdef BUG4
    std::cout << "BUG4: " << name(parse(2)) << ' ' << name(parse(200)) << '\n';
#endif
}
