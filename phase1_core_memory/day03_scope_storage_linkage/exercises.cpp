// Day 03: Output questions + debugging. PREDICT before running!
// Build:  g++ -std=c++20 -Wall -Wextra exercises.cpp -o exercises && ./exercises
// Bugs:   add -DBUG1, -DBUG2, -DBUG3 or -DBUG4 (one at a time). Also try adding -Wshadow.

#include <algorithm>
#include <iostream>
#include <string>

// ── Q1: scope & shadowing ────────────────────────────────────────────────────
int x = 1;

void q1() {
    int x = 2;
    {
        int x = 3;
        std::cout << "Q1: " << x << ' ' << ::x << ' ';
    }
    std::cout << x << '\n';
}

// ── Q2: local static ─────────────────────────────────────────────────────────
int next_seq() {
    static int n = 0;
    return ++n;
}

void q2() {
    std::cout << "Q2: " << next_seq() << ' ' << next_seq() << ' ' << next_seq() << '\n';
}

// ── Q3: storage duration & lifetime. Predict EVERY "Q3" line of the whole program, in order ──
struct Tracer {
    const char* name;
    explicit Tracer(const char* n) : name(n) { std::cout << "Q3: ctor " << name << '\n'; }
    ~Tracer() { std::cout << "Q3: dtor " << name << '\n'; }
};

Tracer g_tracer("global");

void f() {
    static Tracer s("local-static");
    Tracer a("auto");
}

void q3() {
    std::cout << "Q3: calling f twice\n";
    f();
    f();
}

// ── Q4: argument-dependent lookup ────────────────────────────────────────────
namespace mkt {
struct Quote {};
void print(const Quote&) { std::cout << "Q4: mkt::print\n"; }
}  // namespace mkt

void q4() {
    mkt::Quote q;
    print(q);            // no `mkt::`, no `using`. Does this even compile?
}

// ── Q5: inline namespace ─────────────────────────────────────────────────────
namespace api {
namespace v1 { int version() { return 1; } }
inline namespace v2 { int version() { return 2; } }
}  // namespace api

void q5() {
    std::cout << "Q5: " << api::version() << ' ' << api::v1::version() << '\n';
}

// ── Q6: static data member ───────────────────────────────────────────────────
struct T {
    static inline int n = 0;
    T() { ++n; }
    ~T() { --n; }
};

void q6() {
    T a;
    {
        T b;
        T c = b;
    }
    std::cout << "Q6: " << T::n << '\n';
}

// ── Debugging ────────────────────────────────────────────────────────────────
#ifdef BUG1
// Crashes or prints garbage. Why? Two different fixes?
const std::string& venue_name() {
    std::string s = "NSE";
    return s;
}
void bug1() { std::cout << "B1: " << venue_name() << '\n'; }
#endif

#ifdef BUG2
// Does not compile. Read the error carefully: who is fighting over the name?
using namespace std;
int count = 0;
void bug2() {
    ++count;
    cout << "B2: " << count << '\n';
}
#endif

#ifdef BUG3
// VWAP per symbol. RELIANCE's answer is wrong. Why? (And what happens with two threads?)
double vwap_update(double px, double qty) {
    static double pv = 0, vol = 0;
    pv += px * qty;
    vol += qty;
    return pv / vol;
}
void bug3() {
    vwap_update(100.0, 10);                       // TCS
    double tcs = vwap_update(102.0, 10);          // TCS  -> expected 101
    double rel = vwap_update(2500.0, 5);          // RELIANCE -> expected 2500
    std::cout << "B3: tcs=" << tcs << " rel=" << rel << '\n';
}
#endif

#ifdef BUG4
// Position never changes. Why? Which warning flag catches it?
class Position {
public:
    void set(int qty) { int qty_ = qty; (void)qty_; }
    int qty() const { return qty_; }
private:
    int qty_ = 0;
};
void bug4() {
    Position p;
    p.set(500);
    std::cout << "B4: " << p.qty() << "  (expected 500)\n";
}
#endif

int main() {
    std::cout << "main start\n";
    q1();
    q2();
    q3();
    q4();
    q5();
    q6();
#ifdef BUG1
    bug1();
#endif
#ifdef BUG2
    bug2();
#endif
#ifdef BUG3
    bug3();
#endif
#ifdef BUG4
    bug4();
#endif
    std::cout << "main end\n";
}
