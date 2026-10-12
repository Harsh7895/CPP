// Day 04: Exercises. PREDICT the answer BEFORE running!
//
// Output questions:     g++ -std=c++20 -Wall -Wextra exercises.cpp -o ex && ./ex
// "Will it compile?":   add -DC1, -DC2, -DC3, -DC4 or -DC5 (one at a time)
// Bugs:                 add -DBUG1, -DBUG2 or -DBUG3 (one at a time)

#include <iostream>
#include <vector>

// ── Q1: const reference ──────────────────────────────────────────────────────
void q1() {
    int price = 100;
    const int& ref = price;   // ref is another name for price, but "read-only"
    price = 105;
    std::cout << "Q1: " << ref << '\n';
}

// ── Q2: one constexpr function, used two ways ────────────────────────────────
constexpr int square(int x) { return x * x; }

void q2() {
    constexpr int a = square(5);   // computed by the compiler
    int n = 7;                     // a normal variable
    int b = square(n);             // computed when the program runs
    std::cout << "Q2: " << a << ' ' << b << '\n';
}

// ── Q3: mutable ──────────────────────────────────────────────────────────────
struct Quote {
    int bid;
    mutable int reads = 0;
    int get_bid() const { ++reads; return bid; }
};

void q3() {
    const Quote q{100};
    q.get_bid();
    q.get_bid();
    std::cout << "Q3: " << q.reads << '\n';
}

// ── Q4: constinit ────────────────────────────────────────────────────────────
constinit int limit = 10;

void q4() {
    limit += 5;
    std::cout << "Q4: " << limit << '\n';
}

// ── Q5: const and non-const versions of the same function ───────────────────
struct Book {
    int best_bid = 99;
    int& get() { std::cout << "non-const "; return best_bid; }
    const int& get() const { std::cout << "const "; return best_bid; }
};

void q5() {
    Book b;
    const Book cb;
    std::cout << "Q5: ";
    b.get();
    cb.get();
    std::cout << '\n';
}

// ── "Will it compile?" ───────────────────────────────────────────────────────
#ifdef C1
const int max_qty;                       // C1
#endif

#ifdef C2
int lot = 5;
constexpr int two_lots = lot * 2;        // C2
#endif

#ifdef C3
consteval int twice(int x) { return 2 * x; }
int c3() {
    int n = 4;
    return twice(n);                     // C3
}
#endif

#ifdef C4
int read_limit_from_config() { return 500; }
constinit int max_orders = read_limit_from_config();   // C4
#endif

#ifdef C5
struct Account {
    int cash = 0;
    void deposit(int x) { cash += x; }
};
void c5(const Account& acc) {
    acc.deposit(100);                    // C5
}
#endif

// ── Bugs ─────────────────────────────────────────────────────────────────────
#ifdef BUG1
// Not a crash: a SPEED bug. How many copies are made? Fix it with one small change.
struct Order {
    static inline int copies = 0;
    Order() = default;
    Order(const Order&) { ++copies; }
};

int count_orders(std::vector<Order> orders) { return static_cast<int>(orders.size()); }

void bug1() {
    std::vector<Order> book(1000);
    for (int i = 0; i < 10; ++i) count_orders(book);
    std::cout << "B1: copies made = " << Order::copies << "  (should be 0)\n";
}
#endif

#ifdef BUG2
// Does not compile. Fix it by adding ONE word.
class PriceCache {
public:
    int get() const {
        if (!ready_) { value_ = 42; ready_ = true; }   // expensive calculation, done once
        return value_;
    }
private:
    int value_ = 0;
    bool ready_ = false;
};
void bug2() {
    const PriceCache c;
    std::cout << "B2: " << c.get() << '\n';
}
#endif

#ifdef BUG3
// Undefined behaviour! What do you expect it to print, and what does it actually print with -O2?
void bug3() {
    const int risk_limit = 100;
    int* p = const_cast<int*>(&risk_limit);   // p points at risk_limit, with const removed
    *p = 200;                                 // write 200 through p
    std::cout << "B3: " << risk_limit << ' ' << *p << '\n';
}
#endif

int main() {
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
#ifdef BUG3
    bug3();
#endif
}
