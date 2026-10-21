# Day 10 — Virtual memory: pages, page tables, TLB, page faults, `mmap` / `VirtualAlloc`
> Phase 1 · Est. time 40–45 min (paired with Day 11) · Builds on: Day 09 (process layout, BSS "zero on demand"), Day 06 (pointer math)

## 🎯 Today in plain words
Every address you printed on Day 09 was **fake**: a *virtual* address. The CPU translates each one into a real location in RAM, on every single load and store.
Memory is handed out in **pages** (4 KB chunks), and a page usually gets real RAM only the **first time you touch it**. That first touch is a **page fault**, and on your machine it costs about **30× more** than touching the page again.

Why a trader cares: a page fault during trading is a ~0.5 µs stall that the market doesn't wait for. HFT systems **pre-fault** and **lock** all their memory before the open. Today you measure the cost yourself and build a buffer that never faults on the hot path.

Files: `impl.cpp` (page math, measure faults, a pre-faulted buffer) and `exercises.cpp`.

---

## 📺 Watch first

| # | Video | Length | What to watch for |
|---|-------|--------|-------------------|
| 1 | **Computerphile: "What's Virtual Memory?"**. Search YouTube for that exact title | short (about 10 min) | The core idea: every program thinks it has the whole memory, and the hardware translates. Covers §1.1–1.2. |
| 2 | **David Black-Schaffer: "Virtual Memory"** lecture series (Uppsala University). Search YouTube for "David Black-Schaffer virtual memory" | short parts (5–10 min each): watch the page-table, TLB and page-fault parts (about 25 min) | Page tables, multi-level tables, the TLB and page faults, with clear diagrams. Covers §1.3–1.5 and §2.1. |

Optional reading (slides, not a video): [CMU 15-213 "Virtual Memory: Details"](https://www.cs.cmu.edu/afs/cs.cmu.edu/academic/class/15213-m25/www/lectures/17-vm-details.pdf). It's the x86-64 page-table walk, step by step.

**Not covered well by the videos:** Windows reserve vs commit (§1.6), real fault costs on **your** machine (§6), and the read-touch trap (BUG3). Read those carefully.

---

## 0. Warm-up (Days 09 & 08)
1. *(Day 09)* A 50 MB zero-filled static array made a 131 KB `.exe`. Where are the 50 MB of zeros when the program runs?
2. *(Day 09)* On your Windows machine, which was at the lowest address: the stack, the heap or the `.exe`?
3. *(Day 08)* Why may the compiler delete a loop that only reads memory and throws the result away?

<details><summary>Answers</summary>

1. Nowhere yet! The OS only notes "these pages are zero". Real RAM is handed out page by page when you touch them, and each first touch is a page fault. That's today's topic.
2. The stack. The `.exe` image was mapped high (`0x7ff…`).
3. A read with no visible effect doesn't change the program's output, so under the "as-if" rule the optimizer may remove it. That breaks naive "pre-fault by reading" code (BUG3).
</details>

---

## 1. Theory

### 1.1 Virtual vs physical addresses
**In one line:** your program only sees **virtual** addresses. A piece of hardware in the CPU, the **MMU** (Memory Management Unit), translates each one to a **physical** address (a real spot in a RAM chip).

```cpp
int x = 42;
std::printf("%p\n", (void*)&x);   // e.g. 000000e0229ff66c: a VIRTUAL address
// Another program can print the SAME number for its own x: different RAM, no clash.
```

**Analogy:** a hotel. Your room key says "Room 305" (the virtual address). The front desk's book says Room 305 is "Building B, floor 3, door 12" (the physical address). Every guest can have a "Room 305" in a different hotel.

**Why care:** this is how the OS isolates processes, gives every program the same clean layout (Day 09), and gives out RAM lazily.

### 1.2 Pages: memory is managed in 4 KB blocks
**In one line:** the address space is cut into **pages** (4,096 bytes on x86), and translation happens per page, not per byte.

A virtual address splits into two parts:
```
0x7ff7a8a79004
  └────┬────┘└┬┘
  page number  offset inside the page (low 12 bits, because 2^12 = 4096)
  0x7ff7a8a79  0x004
```
```cpp
std::uintptr_t a = 0x7ff7a8a79004;
auto page   = a >> 12;      // 0x7ff7a8a79
auto offset = a & 0xFFF;    // 0x4
```
Only the **page number** is translated. The offset is copied through unchanged.

### 1.3 Page tables: the translation book
**In one line:** the OS keeps a **page table** for each process: for each virtual page, which physical page (the "frame") it maps to, plus permission bits.

| Bit in a page-table entry | Meaning |
|---|---|
| Present | is there real RAM behind this page right now? |
| Writable | may you write? (text and read-only data: no, Day 09 BUG1) |
| User | may normal programs touch it? (kernel pages: no) |
| No-execute | may code run from it? (the stack and heap: no) |
| Accessed / Dirty | set by the CPU when you read / write (the OS uses them for swapping) |

On x86-64 the table is a **4-level tree** (§2.1). A flat table would be huge.

### 1.4 The TLB: a cache of translations
**In one line:** walking the page table for every load would be far too slow, so the CPU caches recent translations in the **TLB** (Translation Lookaside Buffer).

| | Rough size (modern x86) | Covers (with 4 KB pages) |
|---|---|---|
| L1 data TLB | ~64 entries | 64 × 4 KB = **256 KB** |
| L2 TLB (shared) | ~1,500–2,000 entries | ~6–8 MB |

- **TLB hit:** the translation is free.
- **TLB miss:** the CPU walks the page table itself (a "page walk"): up to 4 memory reads, often tens of cycles.
- **Huge pages** (2 MB instead of 4 KB): one TLB entry covers 512× more memory. That's why HFT and databases love them.

### 1.5 Page faults: the first touch
**In one line:** a **page fault** happens when you touch a page whose entry isn't "present". The CPU stops, jumps into the OS kernel, the OS fixes the mapping, and your instruction runs again.

| Kind | When | Cost |
|---|---|---|
| **Minor (soft)** fault | the page just needs a fresh zeroed frame, or is already in RAM somewhere | ~0.3–1 µs (you'll measure it) |
| **Major (hard)** fault | the data must be read from **disk** (swapped out, or a memory-mapped file) | ~10 µs (SSD) to ms (HDD) |
| **Invalid** access | no mapping allowed at all (null, freed, reserved-only, wrong permission) | crash: segfault / access violation |

```cpp
char* p = (char*)os::map(64 << 20);   // 64 MB: instant. No RAM is used yet!
p[0] = 1;                              // page fault #1: the OS gives page 0 real RAM
p[1] = 1;                              // same page: no fault
p[4096] = 1;                           // page fault #2: the next page
```

### 1.6 Getting pages from the OS: `VirtualAlloc` (Windows) / `mmap` (Linux)
**In one line:** `malloc`/`new` sit **on top of** these OS calls. You can call them directly to get whole pages.

| | Windows | Linux |
|---|---|---|
| Get address space only | `VirtualAlloc(…, MEM_RESERVE, …)` | `mmap(…, PROT_NONE, …)` |
| Make it usable | `MEM_COMMIT` (the OS promises RAM *or* page-file space) | `PROT_READ \| PROT_WRITE` |
| Real RAM given | on first touch (a page fault) | on first touch |
| Pre-fault it now | touch every page (or `VirtualLock`) | `MAP_POPULATE` flag, or touch |
| Give it back | `VirtualFree(p, 0, MEM_RELEASE)` | `munmap(p, len)` |
| Allocation unit | 64 KB address granularity, 4 KB pages | 4 KB pages |

**Reserve vs commit** (Windows) is like booking a table vs ordering the food. Reserving just stops anyone else taking those addresses. Touching reserved-but-not-committed memory crashes (BUG1).

### 1.7 Keeping faults out of the hot path
1. **Pre-fault:** touch (write) every page at startup.
2. **Lock it:** stop the OS from paging it out later (`mlock`/`mlockall` on Linux, `VirtualLock` on Windows).
3. **Huge pages:** fewer pages means fewer faults and fewer TLB misses. (Linux: hugetlbfs or THP. Windows: `MEM_LARGE_PAGES`, which needs a special user privilege.)
4. **Never allocate fresh memory while trading** (Day 11 builds the allocator for that).

---

## 2. Internals: what actually happens

### 2.1 The 4-level page walk (x86-64)
Only 48 of the 64 address bits are used (that's why Q3 prints 0: user addresses fit under 2^47).
```
 47      39 38      30 29      21 20      12 11          0
┌─────────┬──────────┬──────────┬──────────┬─────────────┐
│ PML4 (9)│ PDPT (9) │  PD (9)  │  PT (9)  │ offset (12) │
└────┬────┴────┬─────┴────┬─────┴────┬─────┴──────┬──────┘
     ▼         ▼          ▼          ▼            │
  CR3 ─► [table] ─► [table] ─► [table] ─► [table] ─► frame + offset = physical address
```
Each level is a 4 KB table of 512 entries (9 bits = 512). `CR3` is a CPU register holding the top table for the **current process**. Switching processes means switching `CR3`, which flushes much of the TLB. That's one reason context switches are expensive.

With 2 MB **huge pages**, the walk stops one level early, at PD, and the offset is 21 bits.

### 2.2 A page fault, step by step
1. Your `mov [rax], 1` hits a page marked "not present".
2. The CPU saves where you were and jumps into the kernel's page-fault handler (around 100+ cycles just to switch modes).
3. The kernel checks: is this address part of something you were given (a VAD on Windows, a VMA on Linux)? If not → access violation / `SIGSEGV`.
4. It takes a free physical frame and **fills it with zeros** (4 KB of zeroing, so you never see another process's old data).
5. It writes the page-table entry and returns to your program.
6. Your `mov` runs again and succeeds.

All of that is your ~430–680 ns.

### 2.3 Why the second touch still costs ~13 ns (not ~1 ns)
`touch_pages` jumps 4 KB each time. Each step is a **new page** (a TLB miss) and a **new cache line** (a cache miss to RAM, though the CPU overlaps several of them). Touching bytes **inside** one page would be about 1 ns each. Stride matters, and the mini project measures it.

### 2.4 The given `os::` layer in `impl.cpp`
- `os::map` uses `VirtualAlloc(MEM_RESERVE | MEM_COMMIT)` on Windows, and `mmap(PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS)` on Linux.
- `os::fault_count` uses `GetProcessMemoryInfo(...).PageFaultCount` on Windows, and `getrusage(...).ru_minflt + ru_majflt` on Linux.

---

## 3. Implementation (`impl.cpp`)

Build: `g++ -std=c++20 -O2 -Wall -Wextra -pedantic impl.cpp -o impl` then `./impl`.

**Step 1: page math** (all `constexpr`, so you can check them at compile time in Part B).
`is_pow2`, `align_down`, `align_up`, `page_number`, `page_offset`, `pages_spanned`.
<details><summary>Hints</summary>

- A power of two has exactly one bit set: `x != 0 && (x & (x - 1)) == 0`.
- With `a` a power of two, `a - 1` is a mask of the low bits (4096 − 1 = `0xFFF`).
- `align_down(x, a) = x & ~(a - 1)` (clear the low bits).
- `align_up(x, a) = (x + a - 1) & ~(a - 1)`. Careful with the `- 1`: BUG2 shows what happens without it.
- `pages_spanned = (align_up(addr + len, pg) - align_down(addr, pg)) / pg`, and 0 when `len == 0`.
</details>

**Step 2: `touch_pages(p, bytes, page)`.** Write one byte at `p[0]`, `p[page]`, `p[2*page]`, … while the index is `< bytes`, and return the count. A partial last page counts (the test touches `3 * pg + 1` bytes → 4 pages).

**Step 3: `measure_touch(p, bytes, page)`.** Read `os::fault_count()` and `steady_clock::now()` before and after one `touch_pages`, and return `{faults, ns_per_page}`. The tests check that the first pass faults about once per page and the second pass about never.

**Step 4: `PrefaultedBuffer`.** The constructor calls `os::map` and then touches every page. The destructor calls `os::unmap`. The test proves a pass over it causes ~0 faults: that's your "ready for the open" buffer.

### Part B: experiments (write them at the bottom of `impl.cpp`)
1. Add `static_assert(align_up(5000, 4096) == 8192);`. Why can this be checked at compile time?
2. Write down your three ns/page numbers and the ratio.
3. Task Manager: commit size vs working set, before and after touching 1 GB.
4. 64 MB → 1 GB: does the first-touch time scale linearly?

---

## 4. Output Questions

**Q1**
```cpp
std::uintptr_t a = 0x7ff7a8a79004;
std::printf("page 0x%llx offset 0x%llx\n", (unsigned long long)(a >> 12), (unsigned long long)(a & 0xFFF));
```
<details><summary>Answer</summary>`page 0x7ff7a8a79 offset 0x4`. The low 12 bits are the offset, and the rest is the page number.</details>

**Q2**
```cpp
std::uintptr_t start = 0x10000 + 4090;     // a 16-byte object starting 6 bytes before a boundary
std::uintptr_t last  = start + 16 - 1;
std::printf("%llu\n", (unsigned long long)((last >> 12) - (start >> 12) + 1));
```
<details><summary>Answer</summary>`2`. The object straddles two pages, so touching it can cost two TLB entries, and on first touch two faults. Alignment (Day 12) avoids this.</details>

**Q3**
```cpp
int local = 0;
std::printf("%llu\n", (unsigned long long)(reinterpret_cast<std::uintptr_t>(&local) >> 47));
```
<details><summary>Answer</summary>`0`. User-space addresses on x86-64 live below 2^47 (128 TB). The upper half belongs to the kernel. That's why pointers are 8 bytes but only 48 bits matter.</details>

**Q4**
```cpp
char* p = (char*)std::calloc(1u << 30, 1);   // 1 GB of zeros
std::printf("p[123456789] = %d\n", p[123456789]);
```
<details><summary>Answer</summary>`0`, and the calloc is very fast (under 10 ms on your machine). For huge sizes, `calloc` gets fresh pages from the OS, which are already zero, so it doesn't need to write 1 GB of zeros itself. Reading one byte faults in just one page.</details>

---

## 5. Debugging (`-DBUG1` … `-DBUG3`)

**BUG1: touching reserved-only memory**
```cpp
char* p = (char*)VirtualAlloc(nullptr, 1 << 20, MEM_RESERVE, PAGE_READWRITE);  // Linux: mmap(PROT_NONE)
p[0] = 1;
```
<details><summary>Answer</summary>Reserve gives you addresses but no permission to use them. On your machine it's a crash (exit 139). Fix: also `MEM_COMMIT` that range (`VirtualAlloc(p, size, MEM_COMMIT, PAGE_READWRITE)`), or on Linux `mprotect(p, len, PROT_READ | PROT_WRITE)`.</details>

**BUG2: an off-by-one `align_up`**
```cpp
constexpr std::uintptr_t align_up(std::uintptr_t x, std::uintptr_t a) { return (x + a) & ~(a - 1); }
```
<details><summary>Answer</summary>`align_up(4096, 4096)` gives `8192`: an address that's already aligned gets pushed to the next page. You waste a page per allocation, and code that checks "is it aligned?" breaks. Fix: `(x + a - 1) & ~(a - 1)`.</details>

**BUG3: a "pre-fault" that does nothing**
```cpp
for (std::size_t i = 0; i < bytes; i += pg) { char c = p[i]; (void)c; }   // "warm up"
// ... later, the hot path writes every page ...
```
<details><summary>Answer</summary>The read has no visible effect, so at `-O2` the compiler <b>deletes the whole loop</b>. Your hot path still takes ~16,400 faults (that's what your machine printed). Even with a `volatile` read it's OS-dependent: on Windows a read fault maps a real zeroed page, but on Linux a read maps the shared <b>zero page</b> and the first write faults <b>again</b> (copy-on-write). Fix: pre-fault by <b>writing</b> (as `touch_pages` does), or use `MAP_POPULATE` / `mlock` on Linux.</details>

---

## 6. Performance (the HFT angle)
Your machine (64 MB, 16,384 pages):
| Pass | Faults | ns per page |
|---|---|---|
| First touch (fresh memory) | ~16,400 | **~430–680** |
| Second touch | 0 | **~13** |
| Pre-faulted buffer | 0 | ~13 |

- **One fault ≈ 0.5 µs.** A tick-to-trade budget in HFT is often a few µs, so a single fault can eat a big part of it.
- **Fresh memory faults.** A new `std::vector` buffer, a big `malloc`, a new thread's stack, BSS arrays: the first touch of each page pays. Do it at startup.
- **The TLB is a cache too.** Random access over a big table means TLB misses on top of cache misses. Keep hot data compact, and use huge pages for big tables (order-book arrays, instrument tables).
- **Lock it in.** Pre-faulted pages can still be paged out later under memory pressure. Linux trading boxes use `mlockall(MCL_CURRENT | MCL_FUTURE)`.
- **Context switches flush the TLB.** That's another reason trading threads get pinned to their own cores (later in the plan).

---

## 7. Interview Questions
<details><summary>1. What is virtual memory, and why do we have it?</summary>Each process gets its own private address space, and the MMU translates virtual pages to physical frames. It gives isolation, a uniform layout, lazy allocation, sharing (libraries) and swapping.</details>
<details><summary>2. What's a page, and how big is it on x86-64?</summary>The unit of translation and protection: 4 KB normally, plus 2 MB and 1 GB huge pages.</details>
<details><summary>3. What does a page-table entry contain?</summary>The physical frame number plus flags: present, writable, user, no-execute, accessed, dirty.</details>
<details><summary>4. What is the TLB, and what happens on a TLB miss?</summary>A cache of recent virtual→physical translations. On a miss, the hardware walks the 4-level page table (up to 4 memory reads), then caches the result.</details>
<details><summary>5. Minor vs major page fault?</summary>Minor: the page can be satisfied from RAM (a fresh zero page, or a page already cached), ~sub-µs. Major: it needs disk I/O, ~10 µs to ms.</details>
<details><summary>6. Does `malloc(1 GB)` use 1 GB of RAM immediately?</summary>Usually not. It reserves/commits address space, and physical pages arrive on first touch. (Windows does count it against the commit limit right away.)</details>
<details><summary>7. What is a segfault / access violation in page terms?</summary>A fault on an address with no valid mapping, or with the wrong permission (writing to read-only, executing no-exec). The kernel can't fix it, so it kills the program or raises a signal.</details>
<details><summary>8. Why are huge pages good for performance?</summary>One TLB entry covers 2 MB instead of 4 KB, so there are fewer TLB misses, shorter page walks, and fewer faults to pre-fault.</details>
<details><summary>9. How do you avoid page faults in a latency-critical path?</summary>Pre-allocate, pre-fault by writing every page, lock the memory (`mlock`/`VirtualLock`), use huge pages, and never allocate fresh memory on the hot path.</details>
<details><summary>10. Why are context switches expensive in memory terms?</summary>Switching processes changes `CR3`, which invalidates much of the TLB (unless PCID tags are used), and the caches are cold for the new process.</details>
<details><summary>11. What is `mmap` used for besides getting memory?</summary>Mapping files into memory (reads become page faults that load file pages), shared memory between processes (IPC: very common in trading, e.g. a market-data feed shared with strategies), and guard pages.</details>

---

## 8. Hard Questions (optional for now: come back after Day 15)
<details><summary>1. A strategy process reads market data from shared memory written by a feed handler. After a quiet period, the first tick is slow. Name two VM-related causes and fixes.</summary>(1) Pages of the shared region got paged out or trimmed from the working set, so the first access is a fault. Fix: lock the region (`mlock`/`VirtualLock`) and pre-fault it. (2) Translations were evicted from the TLB, and the caches went cold while the core ran other work or slept. Fix: huge pages, a pinned core that busy-polls (keeps the TLB and caches warm), and isolated cores so nothing else runs there.</details>
<details><summary>2. Why does Linux map the "zero page" on a read fault, and how does that break naive pre-faulting?</summary>To save RAM: all untouched-but-read pages share one physical page of zeros, read-only. The first <b>write</b> then triggers a copy-on-write fault to get a private page. So read-only pre-faulting leaves every page still to fault on the write. Windows behaves differently (you measured 0 extra write faults), so portable code pre-faults by writing.</details>
<details><summary>3. You need 10 GB of address space for a sparse hash table, but you'll only use 1 GB. How do you do it without paying for 10 GB?</summary>Reserve the full 10 GB of address space (Windows `MEM_RESERVE`, Linux `mmap` with `PROT_NONE` or a normal anonymous mapping with lazy pages). Commit or touch only the pages you use: either on demand (in a fault handler, or by checking before access) or in chunks. Physical RAM is only spent on touched pages. The cost is a fault the first time each page is used, so pre-touch the hot part.</details>

---

## 9. Mini Project: stride and TLB experiment
1. Allocate and pre-fault 256 MB (your `PrefaultedBuffer`).
2. Time a pass that reads one byte every **S** bytes, for S = 64, 256, 1 KB, 4 KB, 16 KB and 64 KB. Report **ns per access**.
3. Expected shape: S = 64 is cheapest per access (sequential, prefetcher-friendly). Around S = 4 KB+ every access is a new page, so you get a TLB miss **and** a cache miss.
4. Bonus (Linux only): repeat with `madvise(p, len, MADV_HUGEPAGE)` and compare the 4 KB-stride numbers.
5. Write 3 lines: what you saw, and what it means for laying out an order book (keep hot data in few pages).

---

## Checklist
- [ ] Watched the video(s) and read the theory
- [ ] `impl.cpp` passes all tests and prints your fault numbers
- [ ] Part B written down
- [ ] `exercises.cpp`: Q1–Q4, BUG1–BUG3 done
- [ ] Mini project stride table
