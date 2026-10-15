// Day 06: Exercises. PREDICT the answer BEFORE running!
//
// Output questions:     g++ -std=c++20 -Wall -Wextra exercises.cpp -o ex && ./ex
// "Will it compile?":   add -DC1, -DC2, -DC3, -DC4 or -DC5 (one at a time)
// Bugs:                 add -DBUG1, -DBUG2 or -DBUG3 (one at a time)
//                       Bugs are UB: the program may crash, print garbage, or look fine.

#include <cstdint>
#include <iostream>

// ── Q1: pointer arithmetic and negative index ───────────────────────────────
void q1() {
    std::int64_t a[] = {10, 20, 30, 40};
    std::int64_t* p = a;
    p += 2;
    std::cout << "Q1: " << *p << ' ' << p[-1] << ' ' << (p - a) << '\n';
}

// ── Q2: array decay (g++ warns here on purpose: read the warning!) ──────────
void show(std::int64_t arr[]) { std::cout << sizeof(arr) << ' '; }

void q2() {
    std::int64_t a[4]{};
    std::cout << "Q2: " << sizeof(a) << ' ';
    show(a);
    std::cout << '\n';
}

// ── Q3: pointer to pointer ──────────────────────────────────────────────────
void q3() {
    int x = 1, y = 2;
    int* p = &x;
    int** pp = &p;
    *pp = &y;
    **pp = 5;
    std::cout << "Q3: " << x << ' ' << y << ' ' << *p << '\n';
}

// ── Q4: how many BYTES does p + 1 move? ─────────────────────────────────────
void q4() {
    std::int64_t px[3]{};
    std::int64_t* p = px;
    auto bytes = reinterpret_cast<char*>(p + 1) - reinterpret_cast<char*>(p);
    std::cout << "Q4: " << bytes << '\n';
}

// ── Will it compile? ────────────────────────────────────────────────────────
void compile_checks() {
    int x = 1, y = 2;
    const int* a = &x;
    int* const b = &x;
    (void)a; (void)b; (void)y;
#ifdef C1
    *a = 5;
#endif
#ifdef C2
    b = &y;
#endif
#ifdef C3
    a = &y;
    std::cout << "C3 compiled, *a = " << *a << '\n';
#endif
#ifdef C4
    int* c = a;
    (void)c;
#endif
#ifdef C5
    double g[3][4]{};
    double** pg = g;
    (void)pg;
#endif
}

// ── Bugs ────────────────────────────────────────────────────────────────────
void bugs() {
#ifdef BUG1
    std::int64_t* last_price;     // never initialized
    *last_price = 295035;
    std::cout << "BUG1 survived (pure luck)\n";
#endif
#ifdef BUG2
    double* prices = new double[100]{};
    prices[0] = 1.0;
    delete prices;
    std::cout << "BUG2 ran: did the compiler warn?\n";
#endif
#ifdef BUG3
    std::int64_t qty[4] = {1, 2, 3, 4};
    std::int64_t sum = 0;
    for (std::int64_t* p = qty; p <= qty + 4; ++p) sum += *p;
    std::cout << "BUG3 sum = " << sum << " (expected 10)\n";
#endif
}

int main() {
    q1();
    q2();
    q3();
    q4();
    compile_checks();
    bugs();
}
