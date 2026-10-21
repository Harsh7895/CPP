// Day 10: virtual memory: pages, page faults, reserve/commit/mmap, pre-faulting
// Build:  g++ -std=c++20 -O2 -Wall -Wextra -pedantic impl.cpp -o impl
// Run:    ./impl
//
// Do the steps in order (README section 3 explains each one).
// The tests in main() FAIL until you fill in every TODO.
// Works on Windows (MinGW g++) and Linux (g++). Tested on Windows.

#include <cassert>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdio>

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#else
#include <sys/mman.h>
#include <sys/resource.h>
#include <unistd.h>
#endif

// ═════════════════════════════════════════════════════════════════════════════
// GIVEN: a tiny OS layer. You don't need to change it (README §2.4 explains it).
// ═════════════════════════════════════════════════════════════════════════════
namespace os {

#ifdef _WIN32
std::size_t page_size() {
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    return si.dwPageSize;
}
// Reserve + commit: you get an address range. Physical pages come later (first touch).
void* map(std::size_t bytes) {
    return VirtualAlloc(nullptr, bytes, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
}
void unmap(void* p, std::size_t) { VirtualFree(p, 0, MEM_RELEASE); }
// How many page faults this process has taken so far (all kinds).
std::uint64_t fault_count() {
    PROCESS_MEMORY_COUNTERS c{};
    c.cb = sizeof c;
    GetProcessMemoryInfo(GetCurrentProcess(), &c, sizeof c);
    return c.PageFaultCount;
}
#else
std::size_t page_size() { return static_cast<std::size_t>(sysconf(_SC_PAGESIZE)); }
void* map(std::size_t bytes) {
    void* p = mmap(nullptr, bytes, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    return p == MAP_FAILED ? nullptr : p;
}
void unmap(void* p, std::size_t bytes) { munmap(p, bytes); }
std::uint64_t fault_count() {
    rusage u{};
    getrusage(RUSAGE_SELF, &u);
    return static_cast<std::uint64_t>(u.ru_minflt + u.ru_majflt);
}
#endif

}  // namespace os

// ─────────────────────────────────────────────────────────────────────────────
// STEP 1: page math. `page` is always a power of two (4096 on x86).
//   is_pow2(x):            true for 1, 2, 4, 8, ...  (false for 0)
//   align_down(x, a):      largest multiple of a that is <= x
//   align_up(x, a):        smallest multiple of a that is >= x
//   page_number(addr, pg): which page the address is in
//   page_offset(addr, pg): where inside that page
//   pages_spanned(addr, len, pg): how many pages [addr, addr+len) touches (len 0 -> 0)
// Use bit tricks (& and ~) for is_pow2 / align / offset. page_number may use /.
// (Hints in README.)
// ─────────────────────────────────────────────────────────────────────────────
constexpr bool is_pow2(std::uintptr_t x) {
    // TODO
    (void)x;
    return false;
}

constexpr std::uintptr_t align_down(std::uintptr_t x, std::uintptr_t a) {
    // TODO
    (void)a;
    return x + 1;
}

constexpr std::uintptr_t align_up(std::uintptr_t x, std::uintptr_t a) {
    // TODO
    (void)a;
    return x + 1;
}

constexpr std::uintptr_t page_number(std::uintptr_t addr, std::uintptr_t pg) {
    // TODO
    (void)addr;
    (void)pg;
    return 0;
}

constexpr std::uintptr_t page_offset(std::uintptr_t addr, std::uintptr_t pg) {
    // TODO
    (void)addr;
    (void)pg;
    return 1;
}

constexpr std::size_t pages_spanned(std::uintptr_t addr, std::size_t len, std::uintptr_t pg) {
    // TODO: use align_down / align_up
    (void)addr;
    (void)len;
    (void)pg;
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// STEP 2: write ONE byte into every page of [p, p + bytes). Return pages touched.
// ─────────────────────────────────────────────────────────────────────────────
std::size_t touch_pages(char* p, std::size_t bytes, std::size_t page) {
    // TODO
    (void)p;
    (void)bytes;
    (void)page;
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// STEP 3: measure one pass of touch_pages(): how many page faults, how many ns per page.
//   faults = os::fault_count() after - before
//   ns_per_page = elapsed ns / pages touched   (std::chrono::steady_clock)
// ─────────────────────────────────────────────────────────────────────────────
struct TouchResult {
    std::uint64_t faults      = 0;
    double        ns_per_page = 0.0;
};

TouchResult measure_touch(char* p, std::size_t bytes, std::size_t page) {
    // TODO
    (void)p;
    (void)bytes;
    (void)page;
    return TouchResult{999'999'999, 0.0};
}

// ─────────────────────────────────────────────────────────────────────────────
// STEP 4: a buffer that is pre-faulted at construction, so the hot path never faults.
//   ctor:  os::map(bytes), then touch every page (Step 2)
//   dtor:  os::unmap
// Copying a raw owner needs the Rule of 3/5 (Day 22), so it's forbidden here.
// ─────────────────────────────────────────────────────────────────────────────
class PrefaultedBuffer {
public:
    explicit PrefaultedBuffer(std::size_t bytes) {
        // TODO
        (void)bytes;
    }
    ~PrefaultedBuffer() {
        // TODO
    }
    PrefaultedBuffer(const PrefaultedBuffer&) = delete;
    PrefaultedBuffer& operator=(const PrefaultedBuffer&) = delete;

    char*       data() { return data_; }
    std::size_t size() const { return size_; }

private:
    char*       data_ = nullptr;
    std::size_t size_ = 0;
};

int main() {
    const std::size_t pg = os::page_size();
    std::printf("page size = %zu bytes\n", pg);

    // Step 1 (compile-time checks become possible once these work: see Part B)
    assert(is_pow2(1) && is_pow2(4096) && !is_pow2(0) && !is_pow2(4095) && !is_pow2(6));
    assert(align_down(5000, 4096) == 4096);
    assert(align_down(4096, 4096) == 4096);
    assert(align_up(5000, 4096) == 8192);
    assert(align_up(4096, 4096) == 4096);       // already aligned: unchanged
    assert(align_up(0, 64) == 0);
    assert(align_up(1, 64) == 64);
    assert(page_number(0x7ff7a8a79004, 4096) == 0x7ff7a8a79);
    assert(page_offset(0x7ff7a8a79004, 4096) == 0x004);
    assert(pages_spanned(0x1000, 4096, 4096) == 1);     // exactly one page
    assert(pages_spanned(0x1000 + 4092, 8, 4096) == 2); // 8 bytes across a boundary
    assert(pages_spanned(0x1000, 1, 4096) == 1);
    assert(pages_spanned(0x1000, 0, 4096) == 0);
    assert(pages_spanned(0x1001, 3 * 4096, 4096) == 4);

    // Step 2 + 3: fresh memory faults on first touch, not on the second
    const std::size_t bytes = 64u << 20;                // 64 MB
    const std::size_t pages = bytes / pg;
    char* raw = static_cast<char*>(os::map(bytes));
    assert(raw != nullptr);
    assert(touch_pages(raw, 3 * pg + 1, pg) == 4);      // a partial last page counts
    TouchResult first  = measure_touch(raw, bytes, pg);
    TouchResult second = measure_touch(raw, bytes, pg);
    std::printf("first touch : %llu faults for %zu pages, %.1f ns/page\n",
                static_cast<unsigned long long>(first.faults), pages, first.ns_per_page);
    std::printf("second touch: %llu faults, %.1f ns/page\n",
                static_cast<unsigned long long>(second.faults), second.ns_per_page);
    assert(first.faults >= pages / 2);                  // about one fault per page
    assert(second.faults < pages / 20);                 // already mapped: ~0
    assert(first.ns_per_page > second.ns_per_page);
    os::unmap(raw, bytes);

    // Step 4
    {
        PrefaultedBuffer buf(bytes);
        assert(buf.data() != nullptr && buf.size() == bytes);
        TouchResult hot = measure_touch(buf.data(), buf.size(), pg);
        std::printf("pre-faulted : %llu faults, %.1f ns/page\n",
                    static_cast<unsigned long long>(hot.faults), hot.ns_per_page);
        assert(hot.faults < pages / 20);
    }

    std::printf("All Day 10 tests passed\n");
}

/* ── Part B observations (fill in) ────────────────────────────────────────────
 B.1 Add  static_assert(align_up(5000, 4096) == 8192);  Does it compile? Why can it?
 B.2 Your numbers: first-touch ns/page = ____ , second = ____ , ratio = ____
 B.3 Put std::getchar(); right after an os::map(1 GB) and look at Task Manager
     (Details tab, add the "Commit size" and "Page faults" columns). Commit size vs
     working set (memory) BEFORE and AFTER you touch the pages?
 B.4 Change 64 MB to 1 GB. Does first-touch time grow linearly? (about 262,144 pages)
*/
