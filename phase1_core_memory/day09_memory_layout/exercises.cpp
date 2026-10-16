// Day 09: Exercises. PREDICT the answer BEFORE running!
//
// Output questions:   g++ -std=c++20 -O2 -Wall -Wextra exercises.cpp -o ex && ./ex
// Bugs:               add -DBUG1, -DBUG2 or -DBUG3 (one at a time)
//                     BUG1/BUG2 crash on purpose. Note the exit code: echo $?

#include <cstdint>
#include <iostream>

// ── Q1: static local vs plain local ─────────────────────────────────────────
int next_id()  { static int id = 100; return ++id; }
int next_id2() { int id = 100;        return ++id; }

void q1() {
    std::cout << "Q1: " << next_id() << next_id() << next_id() << ' '
              << next_id2() << next_id2() << '\n';
}

// ── Q2: array copy vs pointer to a literal ──────────────────────────────────
void q2() {
    char        a[] = "ESZ6";
    const char* p   = "ESZ6";
    std::cout << "Q2: " << sizeof(a) << ' ' << sizeof(p) << '\n';
}

// ── Q3: what starts at zero? ────────────────────────────────────────────────
int g;
static int s;

void q3() {
    static int ls;
    std::cout << "Q3: " << g << s << ls << '\n';
}

// ── Q4: which way does the stack grow? ──────────────────────────────────────
[[gnu::noinline]] void inner(std::uintptr_t outer_addr) {
    volatile int b = 0;
    std::cout << "Q4: " << (reinterpret_cast<std::uintptr_t>(&b) < outer_addr) << '\n';
}

void q4() {
    volatile int a = 0;
    inner(reinterpret_cast<std::uintptr_t>(&a));
}

// ── Bugs ────────────────────────────────────────────────────────────────────
#ifdef BUG2
[[gnu::noinline]] void load_day(int n) {
    std::int64_t ticks[2'000'000];   // 16 MB local
    for (std::int64_t& t : ticks) t = n;           // really use all 16 MB
    std::cout << "BUG2 survived?! " << ticks[n] + ticks[1'999'999] << '\n';
}
#endif

#ifdef BUG3
[[gnu::noinline]] int count_fills(const int* qty, int n) {
    int count;                        // not zero!
    for (int i = 0; i < n; ++i) if (qty[i] > 0) ++count;
    return count;
}
#endif

void bugs() {
#ifdef BUG1
    char* sym = const_cast<char*>("ESZ6");
    sym[0] = 'N';
    std::cout << "BUG1 survived?! " << sym << '\n';
#endif
#ifdef BUG2
    load_day(0);
#endif
#ifdef BUG3
    const int qty[] = {5, 0, 3, 0, 1};
    std::cout << "BUG3 fills = " << count_fills(qty, 5) << " (expected 3)\n";
#endif
}

int main() {
    q1();
    q2();
    q3();
    q4();
    bugs();
}
