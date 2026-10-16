// Day 09: process memory layout: text, read-only data, data, BSS, heap, stack
// Build:  g++ -std=c++20 -O2 -Wall -Wextra -pedantic impl.cpp -o impl
// Run:    ./impl
//
// Do the steps in order (README section 3 explains each one).
// The tests in main() FAIL until you fill in every TODO.
// Works on Windows (MinGW g++) and Linux (g++). Tested on Windows.

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string_view>

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

enum class Region { Text, ReadOnly, Data, Bss, Stack, Heap };

struct Range {
    std::uintptr_t lo = 0;
    std::uintptr_t hi = 0;   // one past the end, like [first, last)
    bool contains(std::uintptr_t a) const { return a >= lo && a < hi; }
};

struct Layout {
    Range text;      // machine code
    Range rodata;    // const globals, string literals
    Range data;      // globals/statics with a NON-zero start value
    Range bss;       // globals/statics that start at zero
    Range stack;     // this thread's stack
};

template <class T>
std::uintptr_t addr(T* p) { return reinterpret_cast<std::uintptr_t>(p); }

// ═════════════════════════════════════════════════════════════════════════════
// GIVEN: ask the executable and the OS where each region is.
// You don't need to change this. Read it AFTER you finish (README §2.4).
// ═════════════════════════════════════════════════════════════════════════════
#ifdef _WIN32
extern "C" IMAGE_DOS_HEADER __ImageBase;   // the linker puts the .exe's first byte here

Layout read_layout() {
    Layout L;
    const auto* base = reinterpret_cast<const unsigned char*>(&__ImageBase);
    const auto* nt   = reinterpret_cast<const IMAGE_NT_HEADERS*>(base + __ImageBase.e_lfanew);
    const IMAGE_SECTION_HEADER* sec = IMAGE_FIRST_SECTION(nt);
    for (unsigned i = 0; i < nt->FileHeader.NumberOfSections; ++i, ++sec) {
        const char* name = reinterpret_cast<const char*>(sec->Name);   // 8 bytes, maybe no '\0'
        Range r{addr(base + sec->VirtualAddress),
                addr(base + sec->VirtualAddress + sec->Misc.VirtualSize)};
        if      (std::strncmp(name, ".text",  8) == 0) L.text   = r;
        else if (std::strncmp(name, ".rdata", 8) == 0) L.rodata = r;
        else if (std::strncmp(name, ".data",  8) == 0) L.data   = r;
        else if (std::strncmp(name, ".bss",   8) == 0) L.bss    = r;
    }
    ULONG_PTR lo = 0, hi = 0;
    GetCurrentThreadStackLimits(&lo, &hi);
    L.stack = Range{lo, hi};
    return L;
}
#else
// Symbols that the GNU linker / glibc define on Linux.
extern "C" char __executable_start, etext, __data_start, edata, __bss_start, end;

Layout read_layout() {
    Layout L;
    L.text   = Range{addr(&__executable_start), addr(&etext)};
    L.rodata = Range{addr(&etext), addr(&__data_start)};   // roughly: everything between
    L.data   = Range{addr(&__data_start), addr(&edata)};
    L.bss    = Range{addr(&__bss_start), addr(&end)};
    pthread_attr_t attr;
    void* stack_lo = nullptr;
    std::size_t size = 0;
    pthread_getattr_np(pthread_self(), &attr);
    pthread_attr_getstack(&attr, &stack_lo, &size);
    pthread_attr_destroy(&attr);
    L.stack = Range{addr(stack_lo), addr(stack_lo) + size};
    return L;
}
#endif

// ─────────────────────────────────────────────────────────────────────────────
// STEP 1: make each variable land in the region its name says.
// Change ONLY the declaration lines (keep the names).
// ─────────────────────────────────────────────────────────────────────────────
int g_data;              // TODO: must be in DATA  (hint: what decides data vs BSS?)
int g_bss = 5;           // TODO: must be in BSS
int g_rodata = 7;        // TODO: must be READ-ONLY

// TODO: this has a Day 08 bug (g++ warns you) AND it's in the wrong region.
// Make it return the address of a counter that lives for the whole program,
// starts at 1, and sits in DATA.
int* session_counter() {
    int counter = 1;
    return &counter;
}

// TODO: return the address of a function-local static that starts at ZERO (so: BSS).
int* zero_counter() {
    return nullptr;
}

// ─────────────────────────────────────────────────────────────────────────────
// STEP 2: which region holds address a? Check the ranges in L in this order:
// text, rodata, data, bss, stack. If none matches, say Heap.
// (We can't cheaply ask for the heap's bounds, so "anything else" = Heap here.)
// ─────────────────────────────────────────────────────────────────────────────
Region classify(std::uintptr_t a, const Layout& L) {
    // TODO
    (void)a;
    (void)L;
    return Region::Text;
}

std::string_view region_name(Region r) {
    // TODO: "text", "rodata", "data", "bss", "stack", "heap"
    (void)r;
    return "?";
}

// ─────────────────────────────────────────────────────────────────────────────
// STEP 3: watch the stack grow. Recurse `depth` times; at each level store the
// address of a local in out[level] (out[0] = the first call).
// Careful: the optimizer may turn the recursion into a loop and reuse ONE frame.
// Do some work AFTER the recursive call so it can't (see the README hint).
// ─────────────────────────────────────────────────────────────────────────────
[[gnu::noinline]] int record_frames(int depth, int level, std::uintptr_t* out) {
    // TODO
    (void)depth;
    (void)level;
    (void)out;
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// STEP 4: print a memory map. Sort entries by address (ascending) with your own
// insertion sort, then print one line each: address, region, name.
// ─────────────────────────────────────────────────────────────────────────────
struct Entry {
    const char*    name;
    std::uintptr_t a;
};

void sort_by_address(Entry* e, std::size_t n) {
    // TODO: insertion sort
    (void)e;
    (void)n;
}

void print_map(Entry* e, std::size_t n, const Layout& L) {
    sort_by_address(e, n);
    std::printf("\n%-20s %-8s %s\n", "address", "region", "name");
    for (std::size_t i = 0; i < n; ++i)
        std::printf("0x%016llx   %-8.*s %s\n", static_cast<unsigned long long>(e[i].a),
                    static_cast<int>(region_name(classify(e[i].a, L)).size()),
                    region_name(classify(e[i].a, L)).data(), e[i].name);
}

void tick_handler() {}   // just a function, to find the TEXT region

int main() {
    const Layout L = read_layout();

    // Step 1 + 2
    int local = 0;
    int* heap_small = new int(1);
    void* heap_big = std::malloc(1 << 20);   // 1 MB
    const char* literal = "ESZ6";

    assert(classify(reinterpret_cast<std::uintptr_t>(&tick_handler), L) == Region::Text);
    assert(classify(addr(&g_data), L) == Region::Data);
    assert(classify(addr(&g_bss), L) == Region::Bss);
    assert(classify(addr(&g_rodata), L) == Region::ReadOnly);
    assert(classify(addr(literal), L) == Region::ReadOnly);
    assert(session_counter() != nullptr);
    assert(*session_counter() == 1);
    assert(classify(addr(session_counter()), L) == Region::Data);
    assert(session_counter() == session_counter());           // same object every call
    assert(zero_counter() != nullptr && *zero_counter() == 0);
    assert(classify(addr(zero_counter()), L) == Region::Bss);
    assert(classify(addr(&local), L) == Region::Stack);
    assert(classify(addr(heap_small), L) == Region::Heap);
    assert(classify(addr(heap_big), L) == Region::Heap);
    assert(region_name(Region::Bss) == "bss" && region_name(Region::Heap) == "heap");

    // Step 3
    std::uintptr_t frames[5]{};
    record_frames(5, 0, frames);
    for (int i = 0; i < 5; ++i) assert(classify(frames[i], L) == Region::Stack);
    for (int i = 0; i + 1 < 5; ++i) assert(frames[i + 1] < frames[i]);   // grows DOWN
    std::printf("one stack frame of record_frames = %llu bytes\n",
                static_cast<unsigned long long>(frames[0] - frames[1]));

    // Step 4
    Entry map[] = {
        {"local (main)", addr(&local)},
        {"tick_handler()", reinterpret_cast<std::uintptr_t>(&tick_handler)},
        {"g_data", addr(&g_data)},
        {"heap_big (1 MB)", addr(heap_big)},
        {"g_bss", addr(&g_bss)},
        {"\"ESZ6\" literal", addr(literal)},
        {"heap_small", addr(heap_small)},
        {"g_rodata", addr(&g_rodata)},
        {"session_counter", addr(session_counter())},
        {"zero_counter", addr(zero_counter())},
    };
    constexpr std::size_t n = sizeof(map) / sizeof(map[0]);
    print_map(map, n, L);
    for (std::size_t i = 0; i + 1 < n; ++i) assert(map[i].a <= map[i + 1].a);

    std::printf("\nstack size of this thread = %llu KB\n",
                static_cast<unsigned long long>((L.stack.hi - L.stack.lo) / 1024));
    delete heap_small;
    std::free(heap_big);
    std::printf("All Day 09 tests passed\n");
}

/* ── Part B observations (fill in) ────────────────────────────────────────────
 B.1 exe size with  static char big[50'000'000];        :
     exe size with  static char big[50'000'000] = {1};  :   why?
 B.2 `objdump -h impl.exe` (or `size impl.exe`): sizes of .text / .data / .bss:
 B.3 local array of 2'000'000 int64 inside main: what happened? why?
 B.4 run ./impl twice: which addresses changed? (ASLR)
*/
