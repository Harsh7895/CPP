// Day 11: malloc/calloc/realloc/free vs new/delete, placement new, a bump (arena) allocator
// Build:  g++ -std=c++20 -O2 -Wall -Wextra -pedantic impl.cpp -o impl
// Run:    ./impl
//
// Do the steps in order (README section 3 explains each one).
// The tests in main() FAIL until you fill in every TODO.

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <new>

using Price = std::int64_t;
using Qty   = std::int32_t;

struct Order {
    std::uint64_t id;
    Price         px;
    Qty           qty;
};   // 24 bytes, alignment 8

// ─────────────────────────────────────────────────────────────────────────────
// STEP 1: C-style allocation, done right.
//   alloc_prices(n):  n int64 prices, ALL ZERO (which C function zeroes?). nullptr on failure.
//   grow_prices(&arr, old_n, new_n):  resize with realloc. Keep the old values, zero the new
//       slots. If realloc fails: leave *arr UNCHANGED (still valid!) and return false.
// ─────────────────────────────────────────────────────────────────────────────
std::int64_t* alloc_prices(std::size_t n) {
    // TODO
    (void)n;
    return nullptr;
}

bool grow_prices(std::int64_t** arr, std::size_t old_n, std::size_t new_n) {
    // TODO
    (void)arr;
    (void)old_n;
    (void)new_n;
    return false;
}

// ─────────────────────────────────────────────────────────────────────────────
// STEP 2: a bump (arena) allocator. ONE malloc in the ctor, ONE free in the dtor.
//   allocate(bytes, align):  round the CURRENT ADDRESS up to `align` (a power of two),
//       hand out `bytes` from there, bump used_. Not enough room -> nullptr, used_ unchanged.
//   reset():  used_ = 0. Everything is "freed" at once. It does NOT run destructors!
// ─────────────────────────────────────────────────────────────────────────────
class Arena {
public:
    explicit Arena(std::size_t capacity) {
        // TODO: buf_ = ..., cap_ = ...
        (void)capacity;
    }
    ~Arena() {
        // TODO
    }
    Arena(const Arena&) = delete;              // a raw owner: no copies (Rule of 3/5 on Day 22)
    Arena& operator=(const Arena&) = delete;

    void* allocate(std::size_t bytes, std::size_t align) {
        // TODO
        (void)bytes;
        (void)align;
        return nullptr;
    }

    void reset() {
        // TODO
    }

    std::size_t used() const { return used_; }
    std::size_t capacity() const { return cap_; }

private:
    std::byte*  buf_  = nullptr;
    std::size_t cap_  = 0;
    std::size_t used_ = 0;
};

// ─────────────────────────────────────────────────────────────────────────────
// STEP 3: placement new. Get raw memory from the arena, then CONSTRUCT an Order in it
// with  ::new (mem) Order{...}.  Arena full -> nullptr.
// ─────────────────────────────────────────────────────────────────────────────
Order* make_order(Arena& a, std::uint64_t id, Price px, Qty qty) {
    // TODO
    (void)a;
    (void)id;
    (void)px;
    (void)qty;
    return nullptr;
}

// ─────────────────────────────────────────────────────────────────────────────
// STEP 4: objects WITH destructors. Placement new to create, an explicit destructor
// call to destroy. The memory stays in the arena (until reset()).
// ─────────────────────────────────────────────────────────────────────────────
struct Tracked {
    static inline int alive = 0;   // how many Tracked objects exist right now
    int v;
    explicit Tracked(int x) : v(x) { ++alive; }
    ~Tracked() { --alive; }
};

Tracked* make_tracked(Arena& a, int v) {
    // TODO
    (void)a;
    (void)v;
    return nullptr;
}

void destroy_tracked(Tracked* t) {
    // TODO: run the destructor WITHOUT freeing memory
    (void)t;
}

template <class T>
std::uintptr_t addr(T* p) { return reinterpret_cast<std::uintptr_t>(p); }

int main() {
    // Step 1
    std::int64_t* px = alloc_prices(5);
    assert(px != nullptr);
    for (int i = 0; i < 5; ++i) assert(px[i] == 0);
    px[0] = 295035;
    px[4] = 295040;
    assert(grow_prices(&px, 5, 10));
    assert(px[0] == 295035 && px[4] == 295040);         // old values kept
    for (int i = 5; i < 10; ++i) assert(px[i] == 0);    // new slots zeroed
    std::int64_t* before = px;
    assert(!grow_prices(&px, 10, SIZE_MAX / 16));       // impossible size: must fail...
    assert(px == before && px[0] == 295035);            // ...and leave the old buffer intact
    std::free(px);

    // Step 2
    Arena a(1024);
    assert(a.capacity() == 1024 && a.used() == 0);
    void* p1 = a.allocate(1, 1);
    void* p2 = a.allocate(8, 8);
    assert(p1 != nullptr && p2 != nullptr);
    assert(addr(p2) % 8 == 0);                          // aligned
    assert(addr(p2) >= addr(p1) + 1);                   // no overlap
    assert(a.used() == 16);                             // 1 byte + 7 padding + 8
    void* p3 = a.allocate(64, 64);
    assert(p3 != nullptr && addr(p3) % 64 == 0);        // cache-line aligned
    std::size_t used_before = a.used();
    assert(a.allocate(2000, 8) == nullptr);             // too big
    assert(a.used() == used_before);                    // unchanged after a failure
    a.reset();
    assert(a.used() == 0);
    assert(a.allocate(1, 1) == p1);                     // memory is reused from the start
    a.reset();

    // Step 3
    Order* o1 = make_order(a, 1, 295035, 10);
    Order* o2 = make_order(a, 2, 295040, 5);
    assert(o1 && o2);
    assert(o1->id == 1 && o1->px == 295035 && o1->qty == 10);
    assert(o2->id == 2 && o2->qty == 5);
    assert(o2 - o1 == 1);                               // back to back: 24 bytes apart
    assert(addr(o1) % alignof(Order) == 0);

    Arena small(10 * sizeof(Order));
    int made = 0;
    while (make_order(small, static_cast<std::uint64_t>(made), 1, 1)) ++made;
    assert(made == 10);                                 // exactly fills, then nullptr

    // Step 4
    a.reset();
    assert(Tracked::alive == 0);
    Tracked* t1 = make_tracked(a, 7);
    Tracked* t2 = make_tracked(a, 8);
    assert(t1 && t2 && t1->v == 7 && t2->v == 8);
    assert(Tracked::alive == 2);
    destroy_tracked(t2);
    destroy_tracked(t1);
    assert(Tracked::alive == 0);                        // destructors ran
    assert(a.used() >= 2 * sizeof(Tracked));            // memory still "used" until reset()

    std::printf("All Day 11 tests passed\n");
}

/* ── Part B observations (fill in) ────────────────────────────────────────────
 B.1 In make_order, replace placement new with a plain cast:
       Order* o = static_cast<Order*>(mem); o->id = id; ...
     Does it compile? Is it OK for Order? Would it be OK for a struct with a std::string?
 B.2 Call std::free on a pointer you got from new. What happened?
 B.3 Remove the `if (... > cap_) return nullptr;` check and allocate past the end. Did anything stop you?
*/
