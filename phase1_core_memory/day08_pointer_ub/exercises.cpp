// Day 08: Exercises. PREDICT the answer BEFORE running!
//
// Output questions:   g++ -std=c++20 -O0 -Wall -Wextra exercises.cpp -o ex && ./ex
//                     then the SAME with -O2 (compare Q1!)
// 10-bug hunt:        g++ -std=c++20 -O2 -Wall -Wextra -DBUG1 exercises.cpp -o bug && ./bug
//                     ... up to -DBUG10, one at a time.
//                     For each: bug type? did g++ warn? what's the fix?
//                     These are UB: "it printed the right thing" proves NOTHING.

#include <bit>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

using Price = std::int64_t;
using Qty   = std::int32_t;

struct Order { std::uint64_t id; Price px; Qty qty; };
struct Level { Price px; Qty qty; };

// ── Q1: strict aliasing: -O0 vs -O2 ─────────────────────────────────────────
[[gnu::noinline]] int foo(int* i, float* f) {
    *i = 1;
    *f = 0.0f;
    return *i;
}

void q1() {
    int x = 5;
    std::cout << "Q1: " << foo(&x, reinterpret_cast<float*>(&x)) << '\n';
}

// ── Q2: when does the address change? ───────────────────────────────────────
void q2() {
    std::vector<int> v;
    v.reserve(4);
    v.push_back(1);
    int* p = &v[0];
    v.push_back(2); v.push_back(3); v.push_back(4);
    std::cout << "Q2: " << (p == &v[0]) << ' ';
    v.push_back(5);
    std::cout << (p == &v[0]) << '\n';
}

// ── Q3: float bits, the safe way ────────────────────────────────────────────
void q3() {
    float f = 1.0f;
    std::cout << "Q3: " << std::hex << std::bit_cast<std::uint32_t>(f) << ' '
              << std::bit_cast<std::uint32_t>(-1.0f) << std::dec << '\n';
}

// ── Q4: the CORRECT erase loop ──────────────────────────────────────────────
void q4() {
    std::vector<int> ids{1, 2, 3, 4, 6, 7};
    for (auto it = ids.begin(); it != ids.end(); ) {
        if (*it % 2 == 0) it = ids.erase(it);
        else ++it;
    }
    std::cout << "Q4: ";
    for (int id : ids) std::cout << id << ' ';
    std::cout << '\n';
}

// ═════════════════════════════════════════════════════════════════════════════
// THE 10-BUG HUNT
// ═════════════════════════════════════════════════════════════════════════════

#ifdef BUG1
void bug() {
    Order* o = new Order{1, 295035, 10};
    delete o;
    std::cout << "BUG1 qty = " << o->qty << '\n';
}
#endif

#ifdef BUG2
Price* best_price() {
    Price p = 295035;
    return &p;
}
void bug() {
    Price* p = best_price();
    std::cout << "BUG2 best = " << (p ? *p : -1) << '\n';
}
#endif

#ifdef BUG3
struct Book { Level levels[4]{}; };
struct Strategy { Book* book; ~Strategy() { delete book; } };
struct Engine   { Book* book; ~Engine()   { delete book; } };
void bug() {
    Book* b = new Book;
    Strategy s{b};
    Engine e{b};
    std::cout << "BUG3 about to leave scope...\n";
}   // both destructors run here
#endif

#ifdef BUG4
void bug() {
    std::vector<Level> book;
    book.reserve(2);
    book.push_back({295035, 10});
    Level* best = &book[0];
    book.push_back({295040, 5});
    book.push_back({295045, 7});
    best->qty = 0;
    std::cout << "BUG4 book[0].qty = " << book[0].qty << " (expected 0)\n";
}
#endif

#ifdef BUG5
void bug() {
    std::vector<Order> ids{{1, 0, 0}, {2, 0, 0}, {3, 0, 5}};   // qty 0 = filled
    for (auto it = ids.begin(); it != ids.end(); ++it) {
        if (it->qty == 0) ids.erase(it);
    }
    std::cout << "BUG5 left " << ids.size() << " (expected 1)\n";
}
#endif

#ifdef BUG6
void bug() {
    const char* sym = std::string("ESZ6-a-long-symbol-name-on-the-heap").c_str();
    std::string other(40, 'Z');                       // new allocation, may reuse memory
    std::cout << "BUG6 sym = " << sym << '\n';
}
#endif

#ifdef BUG7
void bug() {
    std::vector<Level> levels{{295035, 10}, {295040, 5}};
    Level& top = levels.front();
    levels.insert(levels.begin(), Level{295030, 99});
    std::cout << "BUG7 top.px = " << top.px << " (expected 295035)\n";
}
#endif

#ifdef BUG8
void bug() {
    float f = 1.0f;
    std::uint32_t bits = *reinterpret_cast<std::uint32_t*>(&f);
    std::cout << "BUG8 bits = " << std::hex << bits << std::dec << '\n';
}
#endif

#ifdef BUG9
void bug() {
    alignas(8) unsigned char buf[16]{};
    std::int64_t px_in = 295035;
    std::memcpy(buf + 3, &px_in, 8);
    std::int64_t px = *reinterpret_cast<const std::int64_t*>(buf + 3);
    std::cout << "BUG9 px = " << px << '\n';
}
#endif

#ifdef BUG10
using Handler = void (*)(void* ctx);
struct Callback { Handler fn = nullptr; void* ctx = nullptr; };
Callback g_cb;                                        // lives forever

void count_tick(void* ctx) { ++*static_cast<int*>(ctx); }

void setup() {
    int tick_count = 0;                               // local!
    g_cb = Callback{count_tick, &tick_count};
}

void scribble() {                                     // some other function using the stack
    volatile int junk[8] = {7, 7, 7, 7, 7, 7, 7, 7};
    (void)junk;
}

void bug() {
    setup();
    scribble();
    g_cb.fn(g_cb.ctx);                                // writes into a dead stack slot
    std::cout << "BUG10 callback ran (into what?)\n";
}
#endif

int main() {
    q1();
    q2();
    q3();
    q4();
#if defined(BUG1) || defined(BUG2) || defined(BUG3) || defined(BUG4) || defined(BUG5) || \
    defined(BUG6) || defined(BUG7) || defined(BUG8) || defined(BUG9) || defined(BUG10)
    bug();
#endif
}
