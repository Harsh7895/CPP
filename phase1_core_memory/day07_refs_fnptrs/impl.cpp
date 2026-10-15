// Day 07: references, rvalue references, function pointers, pointer-to-member
// Build:  g++ -std=c++20 -O2 -Wall -Wextra -pedantic impl.cpp -o impl
// Run:    ./impl
//
// Do the steps in order (README section 3 explains each one).
// The tests in main() FAIL until you fill in every TODO.

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <utility>

using Price = std::int64_t;   // fixed-point, 2 decimals: 2950.35 -> 295035
using Qty   = std::int32_t;

// ─────────────────────────────────────────────────────────────────────────────
// STEP 1: references
//   add_fill:   position += fill_qty  (the CALLER's variable must change)
//   better_bid: return a reference to the HIGHER of the two (if equal, return a)
// ─────────────────────────────────────────────────────────────────────────────
void add_fill(Qty& position, Qty fill_qty) {
    // TODO
    (void)position;
    (void)fill_qty;
}

const Price& better_bid(const Price& a, const Price& b) {
    // TODO
    (void)a;
    return b;
}

// ─────────────────────────────────────────────────────────────────────────────
// STEP 2: which overload wins?  int& -> 1,  const int& -> 2,  int&& -> 3
// Then PREDICT each Step 2 test line before running.
// ─────────────────────────────────────────────────────────────────────────────
int kind(int&)       { return 0; }   // TODO
int kind(const int&) { return 0; }   // TODO
int kind(int&&)      { return 0; }   // TODO

// ─────────────────────────────────────────────────────────────────────────────
// Events (used by Steps 3 and 5)
// ─────────────────────────────────────────────────────────────────────────────
enum class EventType : std::uint8_t { Tick, Fill, Cancel };
inline constexpr std::size_t kEventTypes = 3;

struct Event {
    EventType type;
    Price     px;
    Qty       qty;
};

// ─────────────────────────────────────────────────────────────────────────────
// STEP 3: callback dispatcher. NO heap: fixed slots[type][handler].
//   subscribe: false if fn == nullptr or that type is full (kMaxPerType)
//   dispatch:  call every handler of e.type IN SUBSCRIBE ORDER as fn(e, ctx);
//              return how many were called
//   count:     how many handlers that type has
// ─────────────────────────────────────────────────────────────────────────────
using Handler = void (*)(const Event& e, void* ctx);

class Dispatcher {
public:
    static constexpr std::size_t kMaxPerType = 4;

    bool subscribe(EventType t, Handler fn, void* ctx) {
        // TODO
        (void)t;
        (void)fn;
        (void)ctx;
        return false;
    }

    std::size_t dispatch(const Event& e) const {
        // TODO
        (void)e;
        return 0;
    }

    std::size_t count(EventType t) const {
        // TODO
        (void)t;
        return 99;
    }

private:
    struct Slot {
        Handler fn  = nullptr;
        void*   ctx = nullptr;
    };
    Slot        slots_[kEventTypes][kMaxPerType]{};
    std::size_t used_[kEventTypes]{};
};

// ─────────────────────────────────────────────────────────────────────────────
// STEP 4: pointer to data member. Sum ONE chosen field over n fills.
// ─────────────────────────────────────────────────────────────────────────────
struct Fill {
    Price px;
    Qty   qty;
    Qty   fee;
};

std::int64_t sum_field(const Fill* fills, std::size_t n, Qty Fill::* field) {
    // TODO: fills[i].*field
    (void)fills;
    (void)n;
    (void)field;
    return -1;
}

// ─────────────────────────────────────────────────────────────────────────────
// STEP 5: pointer to member function. Fill the table, then implement route().
// ─────────────────────────────────────────────────────────────────────────────
struct Strategy {
    int ticks   = 0;
    Qty pos     = 0;
    int cancels = 0;
    Price last  = 0;

    void on_tick(const Event& e)   { ++ticks; last = e.px; }
    void on_fill(const Event& e)   { pos += e.qty; }
    void on_cancel(const Event&)   { ++cancels; }
};

using StratFn = void (Strategy::*)(const Event&);

// TODO: index Tick -> &Strategy::on_tick, Fill -> on_fill, Cancel -> on_cancel
inline constexpr StratFn kStratTable[kEventTypes] = {nullptr, nullptr, nullptr};

void route(Strategy& s, const Event& e) {
    // TODO: look up kStratTable[...]; if not nullptr, call (s.*fn)(e)
    (void)s;
    (void)e;
}

// ─────────────────────────────────────────────────────────────────────────────
// Test handlers (given: don't change). Each one casts ctx back to its real type.
// ─────────────────────────────────────────────────────────────────────────────
void count_calls(const Event&, void* ctx) { ++*static_cast<int*>(ctx); }
void track_pos(const Event& e, void* ctx) { *static_cast<Qty*>(ctx) += e.qty; }

struct OrderLog { int seq[8]{}; int n = 0; };
void log_a(const Event&, void* ctx) { auto* l = static_cast<OrderLog*>(ctx); l->seq[l->n++] = 1; }
void log_b(const Event&, void* ctx) { auto* l = static_cast<OrderLog*>(ctx); l->seq[l->n++] = 2; }

int main() {
    // Step 1
    Qty position = 10;
    add_fill(position, 5);
    add_fill(position, -20);
    assert(position == -5);
    Price x = 295000, y = 295035;
    assert(&better_bid(x, y) == &y);          // a reference to the ORIGINAL, not a copy
    assert(&better_bid(y, x) == &y);
    assert(&better_bid(x, x) == &x);
    assert(better_bid(x, y) == 295035);

    // Step 2: predict each line first!
    int i = 1;
    const int ci = 2;
    int&& rr = 3;
    assert(kind(i) == 1);
    assert(kind(ci) == 2);
    assert(kind(42) == 3);
    assert(kind(std::move(i)) == 3);
    assert(kind(rr) == 1);                    // ❗ has a name -> lvalue
    assert(kind(i + 1) == 3);

    // Step 3
    Dispatcher d;
    int ticks = 0;
    Qty pos = 0;
    OrderLog log;
    assert(d.count(EventType::Tick) == 0);
    assert(!d.subscribe(EventType::Tick, nullptr, &ticks));          // null handler rejected
    assert(d.subscribe(EventType::Tick, count_calls, &ticks));
    assert(d.subscribe(EventType::Fill, track_pos, &pos));
    assert(d.subscribe(EventType::Cancel, log_a, &log));
    assert(d.subscribe(EventType::Cancel, log_b, &log));
    assert(d.count(EventType::Tick) == 1 && d.count(EventType::Cancel) == 2);

    assert(d.dispatch(Event{EventType::Tick, 295035, 0}) == 1);
    assert(d.dispatch(Event{EventType::Tick, 295040, 0}) == 1);
    assert(ticks == 2);
    assert(d.dispatch(Event{EventType::Fill, 295035, 7}) == 1);
    assert(d.dispatch(Event{EventType::Fill, 295035, -3}) == 1);
    assert(pos == 4);
    assert(d.dispatch(Event{EventType::Cancel, 0, 0}) == 2);
    assert(log.n == 2 && log.seq[0] == 1 && log.seq[1] == 2);         // subscribe order

    int extra = 0;
    assert(d.subscribe(EventType::Tick, count_calls, &extra));
    assert(d.subscribe(EventType::Tick, count_calls, &extra));
    assert(d.subscribe(EventType::Tick, count_calls, &extra));
    assert(!d.subscribe(EventType::Tick, count_calls, &extra));       // full (4 max)
    assert(d.dispatch(Event{EventType::Tick, 1, 0}) == 4);
    assert(ticks == 3 && extra == 3);

    // Step 4
    const Fill fills[] = {{295035, 10, 2}, {295040, 5, 1}, {295030, 20, 4}};
    assert(sum_field(fills, 3, &Fill::qty) == 35);
    assert(sum_field(fills, 3, &Fill::fee) == 7);
    assert(sum_field(fills, 0, &Fill::qty) == 0);

    // Step 5
    Strategy s;
    route(s, Event{EventType::Tick, 295035, 0});
    route(s, Event{EventType::Tick, 295050, 0});
    route(s, Event{EventType::Fill, 295050, 8});
    route(s, Event{EventType::Cancel, 0, 0});
    assert(s.ticks == 2 && s.last == 295050);
    assert(s.pos == 8);
    assert(s.cancels == 1);
    assert(kStratTable[static_cast<std::size_t>(EventType::Fill)] == &Strategy::on_fill);

    std::cout << "All Day 07 tests passed\n";
}

/* ── Part B observations (fill in) ────────────────────────────────────────────
 B.1 s.*fn(e) without parentheses, error:
 B.2 int& r; error:
 B.3 sizeof(StratFn) real value and why:
*/
