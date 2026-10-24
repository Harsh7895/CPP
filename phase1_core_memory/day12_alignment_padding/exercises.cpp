// Day 12: Exercises. PREDICT the answer BEFORE running!
//
// Output questions:   g++ -std=c++20 -O2 -Wall -Wextra exercises.cpp -o ex && ./ex
// Bugs:               add -DBUG1, -DBUG2 or -DBUG3 (one at a time)

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <memory>
#include <new>

// ── Q1: sizes ───────────────────────────────────────────────────────────────
struct Tiny  { char a; short b; char c; };
struct DC    { double d; char c; };
struct Empty {};
struct WithE { int x; Empty e; };

void q1() {
    std::printf("Q1: %zu %zu %zu %zu\n", sizeof(Tiny), sizeof(DC), sizeof(Empty), sizeof(WithE));
}

// ── Q2: offsets ─────────────────────────────────────────────────────────────
struct Msg {
    char          type;
    std::int64_t  px;
    std::int32_t  qty;
};

void q2() {
    std::printf("Q2: type@%zu px@%zu qty@%zu size %zu\n",
                offsetof(Msg, type), offsetof(Msg, px), offsetof(Msg, qty), sizeof(Msg));
}

// ── Q3: alignment of alignas types and arrays ───────────────────────────────
struct alignas(64) Line { std::int64_t v; };

void q3() {
    Line arr[3];
    std::printf("Q3: %zu %zu %td\n", sizeof(Line), sizeof(arr),
                reinterpret_cast<char*>(&arr[2]) - reinterpret_cast<char*>(&arr[0]));
}

// ── Q4: what does std::align do? ────────────────────────────────────────────
void q4() {
    alignas(16) unsigned char buf[100];
    void* p = buf + 3;
    std::size_t space = 97;
    void* r = std::align(16, 24, p, space);
    std::printf("Q4: moved %td, space left %zu, p==r %d\n",
                static_cast<unsigned char*>(r) - (buf + 3), space, p == r);
}

// ── Bugs ────────────────────────────────────────────────────────────────────
#ifdef BUG1
struct Quote { std::int64_t px; std::int32_t qty; char side; };   // 3 padding bytes at the end
#endif

#ifdef BUG2
struct __attribute__((packed)) Wire { char type; std::int64_t px; };   // px at offset 1!
void bump(std::int64_t* p) { *p += 1; }
#endif

void bugs() {
#ifdef BUG1
    Quote a, b;
    std::memset(&a, 0xAA, sizeof a);    // pretend: a came from one buffer...
    std::memset(&b, 0x55, sizeof b);    // ...and b from another
    a.px = b.px = 295035; a.qty = b.qty = 10; a.side = b.side = 'B';
    std::printf("BUG1 same quote? memcmp says %s\n",
                std::memcmp(&a, &b, sizeof a) == 0 ? "yes" : "NO");
#endif
#ifdef BUG2
    Wire w{'Q', 295035};
    bump(&w.px);                        // a normal int64_t* to a misaligned int64
    std::printf("BUG2 px = %lld (did g++ warn?)\n", static_cast<long long>(w.px));
#endif
#ifdef BUG3
    unsigned char raw[256];
    unsigned char* spot = raw + 1;      // "some free space in my buffer"
    Line* l = ::new (spot) Line{42};    // Line needs 64-byte alignment
    std::printf("BUG3 address %% 64 = %zu (must be 0)\n",
                static_cast<std::size_t>(reinterpret_cast<std::uintptr_t>(l) % 64));
#endif
}

int main() {
    q1();
    q2();
    q3();
    q4();
    bugs();
}
