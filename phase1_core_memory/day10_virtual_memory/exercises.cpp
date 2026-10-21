// Day 10: Exercises. PREDICT the answer BEFORE running!
//
// Output questions:   g++ -std=c++20 -O2 -Wall -Wextra exercises.cpp -o ex && ./ex
// Bugs:               add -DBUG1, -DBUG2 or -DBUG3 (one at a time)
//                     BUG1 crashes on purpose. Note the exit code: echo $?

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
[[maybe_unused]] static std::uint64_t fault_count() {
    PROCESS_MEMORY_COUNTERS c{};
    c.cb = sizeof c;
    GetProcessMemoryInfo(GetCurrentProcess(), &c, sizeof c);
    return c.PageFaultCount;
}
[[maybe_unused]] static char* map_fresh(std::size_t bytes) {
    return static_cast<char*>(VirtualAlloc(nullptr, bytes, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE));
}
[[maybe_unused]] static char* reserve_only(std::size_t bytes) {
    return static_cast<char*>(VirtualAlloc(nullptr, bytes, MEM_RESERVE, PAGE_READWRITE));
}
#else
#include <sys/mman.h>
#include <sys/resource.h>
[[maybe_unused]] static std::uint64_t fault_count() {
    rusage u{};
    getrusage(RUSAGE_SELF, &u);
    return static_cast<std::uint64_t>(u.ru_minflt + u.ru_majflt);
}
[[maybe_unused]] static char* map_fresh(std::size_t bytes) {
    return static_cast<char*>(mmap(nullptr, bytes, PROT_READ | PROT_WRITE,
                                   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
}
[[maybe_unused]] static char* reserve_only(std::size_t bytes) {   // address range only, no access allowed
    return static_cast<char*>(mmap(nullptr, bytes, PROT_NONE,
                                   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
}
#endif

// ── Q1: split an address into page number + offset (4 KB pages) ────────────
void q1() {
    std::uintptr_t a = 0x7ff7a8a79004;
    std::printf("Q1: page 0x%llx offset 0x%llx\n",
                static_cast<unsigned long long>(a >> 12),
                static_cast<unsigned long long>(a & 0xFFF));
}

// ── Q2: how many pages does this 16-byte object touch? ──────────────────────
void q2() {
    std::uintptr_t start = 0x10000 + 4090;   // 6 bytes before a page boundary
    std::uintptr_t last  = start + 16 - 1;   // last byte of the object
    std::printf("Q2: %llu\n", static_cast<unsigned long long>((last >> 12) - (start >> 12) + 1));
}

// ── Q3: 64-bit pointers, but how many bits are really used? ─────────────────
void q3() {
    int local = 0;
    auto a = reinterpret_cast<std::uintptr_t>(&local);
    std::printf("Q3: %llu\n", static_cast<unsigned long long>(a >> 47));
}

// ── Q4: a huge calloc that you never touch ──────────────────────────────────
void q4() {
    auto t0 = std::chrono::steady_clock::now();
    char* p = static_cast<char*>(std::calloc(1u << 30, 1));    // 1 GB of zeros
    auto t1 = std::chrono::steady_clock::now();
    if (!p) { std::printf("Q4: calloc failed\n"); return; }
    double us = std::chrono::duration<double, std::micro>(t1 - t0).count();
    std::printf("Q4: p[123456789] = %d, calloc took %s\n", p[123456789],
                us < 10'000 ? "under 10 ms" : "over 10 ms");
    std::free(p);
}

// ── Bugs ────────────────────────────────────────────────────────────────────
#ifdef BUG2
constexpr std::uintptr_t align_up_buggy(std::uintptr_t x, std::uintptr_t a) {
    return (x + a) & ~(a - 1);
}
#endif

void bugs() {
#ifdef BUG1
    char* p = reserve_only(1u << 20);
    std::printf("BUG1 got address %p, writing...\n", static_cast<void*>(p));
    std::fflush(stdout);
    p[0] = 1;
    std::printf("BUG1 survived?!\n");
#endif
#ifdef BUG2
    std::printf("BUG2 align_up(4096, 4096) = %llu (expected 4096)\n",
                static_cast<unsigned long long>(align_up_buggy(4096, 4096)));
    std::printf("BUG2 align_up(5000, 4096) = %llu (expected 8192)\n",
                static_cast<unsigned long long>(align_up_buggy(5000, 4096)));
#endif
#ifdef BUG3
    const std::size_t bytes = 64u << 20, pg = 4096;
    char* p = map_fresh(bytes);
    // "pre-fault" at startup by reading every page... or does it?
    for (std::size_t i = 0; i < bytes; i += pg) {
        char c = p[i];
        (void)c;
    }
    std::uint64_t f0 = fault_count();
    for (std::size_t i = 0; i < bytes; i += pg) p[i] = 1;   // the "hot path"
    std::uint64_t f1 = fault_count();
    std::printf("BUG3 faults in the hot path: %llu (hoped for ~0, pages = %zu)\n",
                static_cast<unsigned long long>(f1 - f0), bytes / pg);
#endif
}

int main() {
    q1();
    q2();
    q3();
    q4();
    bugs();
}
