// Day 07: Exercises. PREDICT the answer BEFORE running!
//
// Output questions:     g++ -std=c++20 -Wall -Wextra exercises.cpp -o ex && ./ex
// "Will it compile?":   add -DC1, -DC2, -DC3 or -DC4 (one at a time)
// Bugs:                 add -DBUG1, -DBUG2 or -DBUG3 (one at a time)
//                       BUG1/BUG2 are UB (dangling): output may look right, or not.

#include <cstdint>
#include <iostream>
#include <utility>

using Price = std::int64_t;

// ── Q1: a reference can't be re-pointed ─────────────────────────────────────
void q1() {
    int a = 1;
    int& r = a;
    r = 5;
    int b = 9;
    r = b;
    b = 10;
    std::cout << "Q1: " << a << ' ' << r << ' ' << b << '\n';
}

// ── Q2: which overload? ─────────────────────────────────────────────────────
void f(int&)       { std::cout << 'L'; }
void f(const int&) { std::cout << 'C'; }
void f(int&&)      { std::cout << 'R'; }

void q2() {
    int x = 1;
    const int y = 2;
    int&& z = 3;
    std::cout << "Q2: ";
    f(x); f(y); f(4); f(z); f(std::move(x));
    std::cout << '\n';
}

// ── Q3: a table of function pointers ────────────────────────────────────────
int add(int a, int b) { return a + b; }
int sub(int a, int b) { return a - b; }
int mul(int a, int b) { return a * b; }

void q3() {
    using Op = int (*)(int, int);
    Op ops[] = {add, sub, mul};
    int acc = 10;
    for (Op op : ops) acc = op(acc, 2);
    std::cout << "Q3: " << acc << '\n';
}

// ── Q4: sizeof a reference vs a struct holding one ──────────────────────────
struct Holder { char& r; };

void q4() {
    char c = 'x';
    char& rc = c;
    std::cout << "Q4: " << sizeof(rc) << ' ' << sizeof(Holder) << '\n';
}

// ── Bonus: sizes of the pointer family (not a quiz, just look) ──────────────
struct Strategy {
    int ticks = 0;
    void on_tick(int) { ++ticks; }
};
struct Fill { std::int64_t px; std::int32_t qty; std::int32_t fee; };

void sizes() {
    std::cout << "sizes: int* " << sizeof(int*)
              << ", fn ptr " << sizeof(int (*)(int, int))
              << ", data member ptr " << sizeof(std::int32_t Fill::*)
              << ", member fn ptr " << sizeof(void (Strategy::*)(int)) << '\n';
}

// ── Will it compile? ────────────────────────────────────────────────────────
void compile_checks() {
    int x = 1;
    (void)x;
#ifdef C1
    int& r1;
#endif
#ifdef C2
    int& r2 = 5;
#endif
#ifdef C3
    const int& r3 = 5;
    std::cout << "C3 compiled, r3 = " << r3 << '\n';
#endif
#ifdef C4
    int&& r4 = x;
#endif
}

// ── Bugs ────────────────────────────────────────────────────────────────────
#ifdef BUG1
const Price& mid(const Price& bid, const Price& ask) {
    Price m = (bid + ask) / 2;
    return m;
}
#endif

#ifdef BUG2
const Price& better_bid(const Price& a, const Price& b) { return a >= b ? a : b; }
#endif

#ifdef BUG3
struct Strat2 { void on_tick(int px) { std::cout << "tick " << px << '\n'; } };
#endif

void bugs() {
#ifdef BUG1
    Price bid = 295000, ask = 295040;
    const Price& m = mid(bid, ask);
    std::cout << "BUG1 mid = " << m << " (expected 295020)\n";
#endif
#ifdef BUG2
    const Price& best = better_bid(295000, 295035);
    std::cout << "BUG2 best = " << best << " (expected 295035)\n";
#endif
#ifdef BUG3
    using Fn = void (Strat2::*)(int);
    Fn fn = &Strat2::on_tick;
    Strat2 s;
    s.*fn(295035);
#endif
}

int main() {
    q1();
    q2();
    q3();
    q4();
    sizes();
    compile_checks();
    bugs();
}
