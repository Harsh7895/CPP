// Day 01: Output questions + debugging. PREDICT the output before running!
// Build:        g++ -std=c++20 -Wall -Wextra exercises.cpp -o exercises
// Debug bugs:   add -DBUG1 or -DBUG2 to the build command
// See preprocessor output:  g++ -std=c++20 -E exercises.cpp | tail -80

#include <iostream>

// Q6: what prints first in the whole program?
struct Init { Init() { std::cout << "Q6: global ctor\n"; } };
Init g_init;

void q1() {
#define SQUARE(x) x * x
    std::cout << "Q1: " << SQUARE(1 + 2) << '\n';
}

void q2() {
#define MAX(a, b) ((a) > (b) ? (a) : (b))
    int i = 5, j = 3;
    int m = MAX(i++, j);
    std::cout << "Q2: " << m << ' ' << i << '\n';
}

void q3() {
#define STR(x) #x
#define XSTR(x) STR(x)
#define VERSION 42
    std::cout << "Q3: " << STR(VERSION) << ' ' << XSTR(VERSION) << '\n';
}

void q4() {
#if FOO == 0
    std::cout << "Q4: zero\n";
#else
    std::cout << "Q4: nonzero\n";
#endif
}

void q5() {
#define CAT(a, b) a##b
    int xy = 7;
    std::cout << "Q5: " << CAT(x, y) << '\n';
}

// ── Debugging ────────────────────────────────────────────────────────────────
#ifdef BUG1
// Which stage reports the error? Read the message carefully.
double vwap(const double* px, const double* qty, int n);
void bug1() {
    double p[] = {100.0, 100.5}, q[] = {10, 30};
    std::cout << "B1: " << vwap(p, q, 2) << '\n';
}
#endif

#ifdef BUG2
// Expected 2.0. What do you get, and why? Fix the macro (or replace it).
#define SPREAD(ask, bid) ask - bid
void bug2() {
    double s = 2 * SPREAD(101.0, 100.0);
    std::cout << "B2: " << s << '\n';
}
#endif

int main() {
    std::cout << "main starts\n";
    q1();
    q2();
    q3();
    q4();
    q5();
#ifdef BUG1
    bug1();
#endif
#ifdef BUG2
    bug2();
#endif
}
