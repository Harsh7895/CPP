// Day 12: alignment, padding, sizeof/alignof, object representation, std::align
// Build:  g++ -std=c++20 -O2 -Wall -Wextra -pedantic impl.cpp -o impl
// Run:    ./impl
//
// Do the steps in order (README section 3 explains each one).
// The tests in main() FAIL until you fill in every TODO.

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <memory>

using Price = std::int64_t;
using Qty   = std::int32_t;

// The order message as someone first wrote it: 32 bytes. Don't change this one.
struct BadOrder {
    char  side;    // 'B' / 'S'
    Price px;
    char  type;    // 'L' / 'M'
    Qty   qty;
    char  flag;
};

// ─────────────────────────────────────────────────────────────────────────────
// STEP 1: same five members, better order. Target: sizeof == 16.
// Keep the member NAMES (the tests use them). Only change the ORDER.
// ─────────────────────────────────────────────────────────────────────────────
struct GoodOrder {
    // TODO: reorder these
    char  side;
    Price px;
    char  type;
    Qty   qty;
    char  flag;
};

// ─────────────────────────────────────────────────────────────────────────────
// STEP 2: be the compiler. Given each member's size and alignment IN ORDER, compute
// sizeof the struct the way g++ does:
//   offset = align_up(offset, member_align);  offset += member_size;
//   struct_align = biggest member_align (at least 1)
//   result = align_up(offset, struct_align)   (tail padding, so arrays stay aligned)
// An empty struct (n == 0) has size 1.
// ─────────────────────────────────────────────────────────────────────────────
constexpr std::size_t align_up(std::size_t x, std::size_t a) { return (x + a - 1) & ~(a - 1); }

struct Member {
    std::size_t size;
    std::size_t align;
};

constexpr std::size_t layout_size(const Member* m, std::size_t n) {
    // TODO
    (void)m;
    (void)n;
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// STEP 3: one counter per CPU core, each on its OWN 64-byte cache line.
// TODO: add the right alignas so sizeof == 64 and neighbours are 64 bytes apart.
// ─────────────────────────────────────────────────────────────────────────────
struct PaddedCounter {
    std::uint64_t value = 0;
};

// ─────────────────────────────────────────────────────────────────────────────
// STEP 4: alignment of pointers.
//   is_aligned(p, a):  is the address a multiple of a?
//   place_aligned(buf, len, size, align):  find the first address inside [buf, buf+len)
//       aligned to `align` with room for `size` bytes. Use std::align. No room -> nullptr.
// ─────────────────────────────────────────────────────────────────────────────
bool is_aligned(const void* p, std::size_t a) {
    // TODO
    (void)p;
    (void)a;
    return false;
}

void* place_aligned(void* buf, std::size_t len, std::size_t size, std::size_t align) {
    // TODO: std::align(align, size, ptr, space) changes ptr and space for you
    (void)buf;
    (void)len;
    (void)size;
    (void)align;
    return nullptr;
}

// ─────────────────────────────────────────────────────────────────────────────
// STEP 5: compare two orders correctly. Padding bytes can hold ANY garbage,
// so memcmp is wrong (BUG1). Compare field by field (or "= default" operator==).
// ─────────────────────────────────────────────────────────────────────────────
bool same_order(const GoodOrder& a, const GoodOrder& b) {
    // TODO
    (void)a;
    (void)b;
    return false;
}

int main() {
    // Step 1
    static_assert(sizeof(BadOrder) == 32);
    assert(sizeof(GoodOrder) == 16);
    GoodOrder g{};
    g.side = 'B'; g.px = 295035; g.type = 'L'; g.qty = 10; g.flag = 0;
    assert(g.px == 295035 && g.qty == 10 && g.side == 'B');

    // Step 2: check your "compiler" against the real one
    constexpr Member bad[]  = {{1, 1}, {8, 8}, {1, 1}, {4, 4}, {1, 1}};
    constexpr Member good[] = {{8, 8}, {4, 4}, {1, 1}, {1, 1}, {1, 1}};
    constexpr Member tiny[] = {{1, 1}, {2, 2}, {1, 1}};        // char, short, char
    constexpr Member dc[]   = {{8, 8}, {1, 1}};                // double, char
    assert(layout_size(bad, 5) == sizeof(BadOrder));           // 32
    assert(layout_size(good, 5) == 16);
    assert(layout_size(tiny, 3) == 6);
    assert(layout_size(dc, 2) == 16);                          // tail padding!
    assert(layout_size(nullptr, 0) == 1);                      // empty struct

    // Step 3
    assert(sizeof(PaddedCounter) == 64);
    assert(alignof(PaddedCounter) == 64);
    PaddedCounter per_core[4];
    assert(reinterpret_cast<char*>(&per_core[1]) - reinterpret_cast<char*>(&per_core[0]) == 64);
    assert(is_aligned(&per_core[2], 64));

    // Step 4
    alignas(64) unsigned char buf[256];
    assert(is_aligned(buf, 64) && is_aligned(buf + 8, 8) && !is_aligned(buf + 3, 4));
    assert(is_aligned(buf + 1, 1));
    void* p = place_aligned(buf + 3, 200, 24, 16);
    assert(p == buf + 16);                                      // first 16-aligned spot
    void* q = place_aligned(buf + 1, 200, 64, 64);
    assert(q == buf + 64);
    assert(place_aligned(buf + 1, 100, 64, 64) == nullptr);    // 63 skipped + 64 > 100: no room

    // Step 5: same fields, different garbage in the padding
    GoodOrder a, b;
    std::memset(&a, 0xAA, sizeof a);
    std::memset(&b, 0x55, sizeof b);
    a.side = b.side = 'S'; a.px = b.px = 295040; a.type = b.type = 'M';
    a.qty = b.qty = 7; a.flag = b.flag = 1;
    assert(same_order(a, b));
    b.qty = 8;
    assert(!same_order(a, b));

    std::printf("All Day 12 tests passed\n");
}

/* ── Part B observations (fill in) ────────────────────────────────────────────
 B.1 Offsets of every BadOrder member (use offsetof from <cstddef>):
 B.2 After Step 1, how many padding bytes are left in GoodOrder? Where?
 B.3 Add  static_assert(layout_size(bad, 5) == 32);  Compiles? Why can it?
 B.4 How many BadOrder vs GoodOrder fit in one 64-byte cache line? In 1 MB of L2?
*/
