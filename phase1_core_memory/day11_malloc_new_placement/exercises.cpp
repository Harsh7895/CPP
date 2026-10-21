// Day 11: Exercises. PREDICT the answer BEFORE running!
//
// Output questions:   g++ -std=c++20 -O2 -Wall -Wextra exercises.cpp -o ex && ./ex
// Bugs:               add -DBUG1, -DBUG2 or -DBUG3 (one at a time)

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <new>
#include <string>

// We REPLACE the global operator new[] / delete[] to spy on how many bytes are requested.
static volatile std::size_t g_last_request = 0;   // volatile: g++ may assume operator new touches no globals
void* volatile g_sink = nullptr;   // storing a pointer here stops -O2 from deleting new/delete pairs
void* operator new[](std::size_t n) {
    g_last_request = n;
    if (void* p = std::malloc(n)) return p;
    throw std::bad_alloc{};
}
void operator delete[](void* p) noexcept { std::free(p); }
void operator delete[](void* p, std::size_t) noexcept { std::free(p); }

// ── Q1: what does new[] really ask for? ─────────────────────────────────────
struct WithDtor { int x; ~WithDtor() {} };   // 4 bytes, has a destructor
struct Plain    { int x; };                  // 4 bytes, trivial

void q1() {
    WithDtor* a = new WithDtor[3];
    g_sink = a;
    std::size_t ra = g_last_request;
    Plain* b = new Plain[3];
    g_sink = b;
    std::size_t rb = g_last_request;
    std::printf("Q1: %zu %zu\n", ra, rb);
    delete[] a;
    delete[] b;
}

// ── Q2: placement new and explicit destructor calls ─────────────────────────
struct Loud {
    Loud()  { std::printf("ctor "); }
    ~Loud() { std::printf("dtor "); }
};

void q2() {
    alignas(Loud) unsigned char buf[sizeof(Loud)];
    std::printf("Q2: ");
    Loud* p = ::new (buf) Loud;   // construct in OUR buffer
    std::printf("| ");
    p->~Loud();                   // destroy, but don't free (buf is on the stack)
    std::printf("| end\n");
}

// ── Q3: bump-allocator offsets (base is 16-aligned) ─────────────────────────
std::size_t bump(std::size_t& used, std::size_t bytes, std::size_t align) {
    std::size_t start = (used + align - 1) & ~(align - 1);
    used = start + bytes;
    return start;
}

void q3() {
    std::size_t used = 0;
    std::size_t a = bump(used, 1, 1);
    std::size_t b = bump(used, 8, 8);
    std::size_t c = bump(used, 4, 4);
    std::size_t d = bump(used, 16, 16);
    std::printf("Q3: %zu %zu %zu %zu used=%zu\n", a, b, c, d, used);
}

// ── Q4: when allocation fails ───────────────────────────────────────────────
void q4() {
    volatile std::size_t huge = SIZE_MAX / 4;            // volatile: decided at run time
    char* p = new (std::nothrow) char[huge];
    g_sink = p;
    std::printf("Q4: nothrow gave %s, ", p ? "memory?!" : "nullptr");
    try {
        char* q = new char[huge];
        g_sink = q;
        std::printf("plain new gave memory?!\n");
        delete[] q;
    } catch (const std::bad_alloc&) {
        std::printf("plain new threw bad_alloc\n");
    }
}

// ── Bugs ────────────────────────────────────────────────────────────────────
struct NamedOrder {
    std::string sym;            // has a constructor: it must be BUILT, not just allocated
    std::int64_t px;
};

#ifdef BUG3
struct MiniArena {
    alignas(16) unsigned char buf[1024];
    std::size_t used = 0;
    void* allocate(std::size_t n) { void* p = buf + used; used += (n + 15) & ~std::size_t{15}; return p; }
    void reset() { used = 0; }
};
struct Tracked {
    static inline int alive = 0;
    std::string note{"a heap-allocated note, long enough to skip SSO"};
    Tracked() { ++alive; }
    ~Tracked() { --alive; }
};
#endif

void bugs() {
#ifdef BUG1
    NamedOrder* o = static_cast<NamedOrder*>(std::malloc(sizeof(NamedOrder)));
    o->sym = "ESZ6-quarterly-future";      // assigning to a string that was never constructed
    o->px  = 295035;
    std::printf("BUG1 %s %lld\n", o->sym.c_str(), static_cast<long long>(o->px));
    std::free(o);
#endif
#ifdef BUG2
    std::int64_t* prices = static_cast<std::int64_t*>(std::malloc(100 * sizeof(std::int64_t)));
    prices[0] = 295035;
    volatile std::size_t huge = SIZE_MAX / 4;
    prices = static_cast<std::int64_t*>(std::realloc(prices, huge));   // fails...
    std::printf("BUG2 prices is now %s: the old 800 bytes are %s\n",
                prices ? "valid" : "nullptr", prices ? "fine" : "LEAKED");
    std::free(prices);
#endif
#ifdef BUG3
    MiniArena arena;
    for (int i = 0; i < 3; ++i) ::new (arena.allocate(sizeof(Tracked))) Tracked;
    arena.reset();                          // "free everything"
    std::printf("BUG3 alive after reset = %d (each one still owns a heap string)\n", Tracked::alive);
#endif
}

int main() {
    q1();
    q2();
    q3();
    q4();
    bugs();
}
