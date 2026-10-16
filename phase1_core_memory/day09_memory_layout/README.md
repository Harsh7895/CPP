# Day 09 — Process memory layout: text, data, BSS, read-only data, heap, stack
> Phase 1 · Est. time 40–45 min (paired with Day 08) · Builds on: Day 03 (storage duration: static/automatic/dynamic), Day 06 (pointers, `new[]`), Day 08 (dangling locals)

## 🎯 Today in plain words
When your program runs, the OS gives it one big range of addresses, and different kinds of things live in different **regions** of it:
the machine code, the constants, the globals, the heap and the stack.
Today you'll **print real addresses** from your own program and sort them into those regions.
You'll also see that your Windows machine arranges them **differently** from the textbook (Linux) picture.

Why a trader cares: *where* a variable lives decides how fast it is to create (stack: about free; heap: tens of ns or worse), how much of it you can have (the stack is small: 2 MB on your g++), and whether it costs anything at startup (BSS: free in the file, but paid for with page faults when you first touch it).

Files: `impl.cpp` (classify addresses, watch the stack grow, print a memory map) and `exercises.cpp`.

---

## 📺 Watch first

| # | Video | Length | What to watch for |
|---|-------|--------|-------------------|
| 1 | **The Cherno: "Stack vs Heap Memory in C++"**. Search YouTube for that exact title | about 20 min | Stack vs heap allocation, what `new` really does, and why the stack is faster. Covers §1.2–1.3 and §6. |
| 2 | **Demystifying Process Address Space: Heap, Stack and Beyond**, NDC TechTown ([talk page](https://ndctechtown.com/agenda/demystifying-process-address-space-heap-stack-and-beyond-03aw/0qw1ri0wmi8)). Search YouTube for that exact title | about 1 hr (watch the first 25 min) | The whole picture: text, data, BSS, heap, stack and the virtual address space. Covers §1 and §2. |

**Not covered well by the videos:** the **Windows/PE** layout you actually have (§2.2) and the BSS file-size trick (§2.3). Read those carefully. Both videos use Linux.

---

## 0. Warm-up (Days 03 & 08)
1. *(Day 03, simply)* What are the three main **storage durations**, and when does each object die?
2. *(Day 08)* Why is `int* f() { int x = 1; return &x; }` a bug? What did your g++ actually return at `-O2`?
3. *(Day 08)* Why is `std::memcpy(&px, buf + 3, 8)` OK when `*reinterpret_cast<const std::int64_t*>(buf + 3)` isn't?

<details><summary>Answers</summary>

1. **Automatic** (locals: they die at the end of their `{ }` scope), **static** (globals and `static` locals: they live for the whole program), and **dynamic** (`new`: they live until `delete`). There's also thread storage (`thread_local`). Today you'll see *where* each of them lives.
2. `x` dies when `f` returns, so the pointer dangles. Your g++ warned and returned `nullptr`.
3. There's no `int64_t` object at `buf + 3`, only bytes. Reading them as an `int64_t` breaks strict aliasing (and alignment). `memcpy` copies bytes, which is always allowed.
</details>

---

## 1. Theory

### 1.1 The six regions
**In one line:** a running program's memory is split into regions, each holding one kind of thing.

```cpp
int        g_data   = 42;     // DATA:      global with a non-zero start value
int        g_bss;             // BSS:       global that starts at zero
const int  g_rodata = 7;      // READ-ONLY: a constant (writing to it = crash)
void tick_handler() {}        // TEXT:      machine code lives here

int main() {
    int  local = 0;           // STACK:     a local (automatic) variable
    int* heap  = new int(1);  // `heap` (the pointer) is on the STACK; the int it points to is on the HEAP
    const char* s = "ESZ6";   // the letters are in READ-ONLY; the pointer s is on the STACK
    static int calls = 1;     // DATA (static local, non-zero)
}
```

| Region | What lives there | Size fixed when? | Writable? |
|---|---|---|---|
| **text** (`.text`) | machine code | at compile/link time | ❌ (read + execute) |
| **read-only data** (`.rdata` on Windows, `.rodata` on Linux) | `const` globals, string literals, jump tables | link time | ❌ |
| **data** (`.data`) | globals/statics with a **non-zero** start value | link time | ✅ |
| **BSS** (`.bss`) | globals/statics that start at **zero** (or have no initializer) | link time | ✅ |
| **heap** | `new`, `malloc`, and whatever `std::vector`/`std::string` allocate | grows at run time | ✅ |
| **stack** | locals, function arguments, return addresses | per thread, fixed maximum | ✅ |

**Analogy:** a restaurant. Text = the recipe book (read-only). Read-only data = the printed menu. Data/BSS = the fixed tables, set up before opening (BSS tables come with empty plates). Heap = the extra tables you rent when a big group arrives. Stack = the tray each waiter carries: fast, small, and cleared after every trip.

**Why "BSS"?** It's an old assembler name ("Block Started by Symbol"). Just remember: **BSS = zeros**.

### 1.2 The stack: fast, automatic, small
**In one line:** each function call pushes a **frame** (its locals plus the return address) onto the stack, and returning pops it.

```cpp
void inner() { int b = 2; std::printf("inner b at %p\n", (void*)&b); }
void outer() { int a = 1; std::printf("outer a at %p\n", (void*)&a); inner(); }
// outer a at 0x...f6ac
// inner b at 0x...f67c   ← a LOWER address: the stack grows DOWN on x86
```

- **Allocating** a frame is one instruction (`sub rsp, N`). That's about free.
- It's **small**: your MinGW program's main thread gets **2 MB** (`impl.cpp` prints it). MSVC programs default to 1 MB. Linux usually gives the main thread 8 MB (`ulimit -s`).
- Too much stack (a huge local array, or very deep recursion) is a **stack overflow**, and the program just dies.

### 1.3 The heap: flexible, slower
**In one line:** the heap is memory you ask for at run time (`new`/`malloc`) and give back yourself (`delete`/`free`).

- **Any size**, any lifetime, but each `new` runs allocator code (it searches free lists, sometimes asks the OS): tens of ns, sometimes µs.
- Big requests (around ≥ 512 KB–1 MB) usually go **straight to the OS** as fresh pages, in a different area. You'll see that your 1 MB `malloc` lands far from the small `new int`.

### 1.4 Data vs BSS: decided by the start value
```cpp
int a = 42;         // .data  (the 42 must be stored in the .exe file)
int b;              // .bss   (globals start at zero, Day 03)
int c = 0;          // .bss   ← explicitly zero is STILL BSS
static int d = 5;   // .data
const int e = 7;    // read-only (if its address is used; otherwise the compiler may just inline the 7)
```
**Rule:** "non-zero start value" → data. "zero" → BSS. "const" → read-only.

### 1.5 ASLR: the addresses change every run
**In one line:** **ASLR** (Address Space Layout Randomization) makes the OS load your program, heap and stack at random addresses each run, so attackers can't guess where things are.

Run `impl` twice: every address changes, but the **order** of the regions and the distances *inside* the `.exe` stay the same.

---

## 2. Internals: what actually happens

### 2.1 The textbook picture (Linux, x86-64)
```
high addresses  0x7fff........  ┌──────────────┐
                                │    stack     │  grows DOWN ↓
                                │      ↓       │
                                │  (big gap)   │  shared libraries, mmap'd big allocations
                                │      ↑       │
                                │    heap      │  grows UP ↑
                                ├──────────────┤
                                │    .bss      │  zeros
                                │    .data     │  initialized globals
                                │   .rodata    │  constants, string literals
                                │    .text     │  code
low addresses   0x55........    └──────────────┘  (with PIE + ASLR)
```

### 2.2 YOUR picture (Windows, MinGW g++): real output from `impl.cpp`
```
address              region   name
0x000000e0229ff66c   stack    local (main)        ← LOWEST: the stack is near the bottom on Windows
0x0000021545042710   heap     heap_small
0x0000021545153040   heap     heap_big (1 MB)
0x00007ff7a8a714c0   text     tick_handler()      ← the .exe image is loaded HIGH
0x00007ff7a8a79000   data     session_counter
0x00007ff7a8a79004   data     g_data
0x00007ff7a8a7a179   rodata   "ESZ6" literal
0x00007ff7a8a7a450   rodata   g_rodata
0x00007ff7a8a7e030   bss      g_bss
0x00007ff7a8a7e034   bss      zero_counter
stack size of this thread = 2048 KB
```
| | Linux (textbook) | Windows (your machine) |
|---|---|---|
| Executable image | low (`0x55…`) | high (`0x7ff…`) |
| Stack | top of the address space | **lowest** of these regions |
| Section order in the image | text, rodata, data, bss | text, **data**, **rdata**, bss (from `objdump -h`) |
| Read-only data called | `.rodata` | `.rdata` |
| Main thread stack | ~8 MB | 2 MB (MinGW), 1 MB (MSVC) |

**The lesson:** "stack at the top, heap below it" is **not a law**. It's one OS's choice. What *is* universal: the stack grows down on x86, the regions have different permissions, and BSS is zero-filled.

### 2.3 Why BSS costs nothing in the file
The `.exe` must store every byte of `.data` (the 42s and 5s). For `.bss` it stores only **"N bytes of zeros, please"**. Real numbers from your g++:
```cpp
static char big[50'000'000];          // b1.exe = 131 KB    (BSS)
static char big[50'000'000] = {1};    // b2.exe = 50 MB     (DATA: all 50 MB stored in the file!)
```
`objdump -h` shows it: `.bss` has file offset 0 and only the `ALLOC` flag (no `CONTENTS`).

> 🔍 **Going deeper (optional):** when the program starts, the OS doesn't really allocate 50 MB of RAM for that BSS array. It maps the pages as "zero on demand". The first time you **touch** each 4 KB page, you take a **page fault** and the OS hands you a fresh zeroed page. That's Day 10, and it's why HFT systems "pre-touch" their memory before the market opens.

### 2.4 How `read_layout()` finds the regions (the given code in `impl.cpp`)
- **Windows:** every `.exe` starts with a header. The linker gives you its address as `__ImageBase`. The header has a **section table**: for each section, a name (`.text`, `.data`, …), where it starts and how big it is. The code walks that table. The stack limits come from the OS call `GetCurrentThreadStackLimits`.
- **Linux:** the GNU linker defines marker symbols, such as `etext` (the end of the code), `edata` (the end of `.data`) and `end` (the end of `.bss`). Their **addresses** are the boundaries. The stack comes from `pthread_getattr_np`.

---

## 3. Implementation (`impl.cpp`)

Build: `g++ -std=c++20 -O2 -Wall -Wextra -pedantic impl.cpp -o impl` then `./impl`.

**Step 1: place the variables.** Change only the declaration lines so `g_data` lands in data, `g_bss` in BSS and `g_rodata` in read-only. Then fix `session_counter()`: it has a Day 08 bug (g++ warns). It must return the address of a counter that lives forever, starts at 1, and sits in data. Then make `zero_counter()` return the address of a function-local static that starts at zero (BSS).
<details><summary>Hint</summary>Data needs a non-zero start value (`= 42`). BSS is "no initializer" or `= 0`. Read-only is `const`. A `static` local lives for the whole program, so `static int counter = 1;` goes in data.</details>

**Step 2: `classify(a, L)` and `region_name(r)`.** Check `L.text`, `L.rodata`, `L.data`, `L.bss` and `L.stack` in that order with `Range::contains`. Anything else is `Region::Heap`.

**Step 3: `record_frames(depth, level, out)`.** Store the address of a local in `out[level]`, recurse with `level + 1` until `depth`, and return something. The tests check that each deeper frame has a **lower** address.
<details><summary>Hint (important)</summary>At `-O2` g++ is clever: if the recursive call is the last thing you do, it can turn the recursion into a loop with <b>one</b> frame, and every address comes out the same. Make the local `volatile int local = level;` (volatile = "the compiler must really keep this in memory"), and use it <b>after</b> the call: `return record_frames(depth, level + 1, out) + local;`.</details>

**Step 4: `sort_by_address(e, n)`.** Write an insertion sort (you know it from DSA) by `Entry::a`. `print_map` then prints your memory map, and it should look like §2.2.

### Part B: experiments (write the results at the bottom of `impl.cpp`)
1. In a tiny separate file, compile `static char big[50'000'000];` and then `= {1};`, and compare the `.exe` sizes (`ls -l`). Why?
2. Run `objdump -h impl.exe` (or `size impl.exe`). Find `.text`, `.data`, `.rdata` and `.bss`: their sizes and flags.
3. Add `std::int64_t ticks[2'000'000];` (16 MB) as a local in `main` and write to every element (if you don't use it, `-O2` simply deletes it). What happens? Why? (§1.2)
4. Run `./impl` twice. Which addresses changed, and which **differences** stayed the same? (§1.5)

---

## 4. Output Questions

**Q1**
```cpp
int next_id()  { static int id = 100; return ++id; }
int next_id2() { int id = 100;        return ++id; }
std::cout << next_id() << next_id() << next_id() << ' '
          << next_id2() << next_id2() << '\n';
```
<details><summary>Answer</summary>`101102103 101101`. The `static` local lives in data and keeps its value between calls. The plain local is a new stack variable each call. (The `<<` chain is evaluated left to right since C++17.)</details>

**Q2**
```cpp
char        a[] = "ESZ6";
const char* p   = "ESZ6";
std::cout << sizeof(a) << ' ' << sizeof(p) << '\n';
```
<details><summary>Answer</summary>`5 8`. `a` is an array of 5 chars (4 + `'\0'`), and it's a <b>copy</b> on the stack that you may modify. `p` is an 8-byte pointer to the literal in read-only data, which you may not modify.</details>

**Q3**
```cpp
int g;
static int s;
int main() {
    static int ls;
    std::cout << g << s << ls << '\n';
}
```
<details><summary>Answer</summary>`000`. All three have static storage duration, so they're in BSS and start at zero. A plain local `int l;` would hold garbage, and reading it is UB (BUG3).</details>

**Q4**
```cpp
void inner(std::uintptr_t outer_addr) {
    int b = 0;
    std::cout << (reinterpret_cast<std::uintptr_t>(&b) < outer_addr) << '\n';
}
int main() { int a = 0; inner(reinterpret_cast<std::uintptr_t>(&a)); }
```
<details><summary>Answer</summary>`1` on x86 (both Windows and Linux): the stack grows <b>down</b>, so the callee's locals are at lower addresses. We compare `uintptr_t` integers because comparing pointers to unrelated objects with `<` isn't guaranteed by the language.</details>

---

## 5. Debugging (`-DBUG1` … `-DBUG3`)

**BUG1: writing into a string literal**
```cpp
char* sym = const_cast<char*>("ESZ6");
sym[0] = 'N';
```
<details><summary>Answer</summary>The literal lives in <b>read-only</b> memory, and the OS marks those pages "no write". On your machine: `Segmentation fault` (exit 139). Fix: copy it into an array you own: `char sym[] = "ESZ6";`.</details>

**BUG2: a stack overflow**
```cpp
void load_day() {
    std::int64_t ticks[2'000'000];   // 16 MB local
    for (std::int64_t& t : ticks) t = 0;   // really use all 16 MB
}
```
<details><summary>Answer</summary>16 MB doesn't fit in a 2 MB stack. On your machine the program dies <b>with no message</b> (Git Bash shows exit 127, and Windows records it as STATUS_STACK_OVERFLOW). Fixes: `std::vector<std::int64_t> ticks(2'000'000);` (heap), or a `static` array (BSS) if one copy is enough. Big buffers never go on the stack.</details>

**BUG3: assuming locals start at zero**
```cpp
int count_fills(const int* qty, int n) {
    int count;                        // not zero!
    for (int i = 0; i < n; ++i) if (qty[i] > 0) ++count;
    return count;
}
```
<details><summary>Answer</summary>Globals and statics (BSS) start at zero. <b>Locals don't</b>: their stack slot holds whatever the last function left there. Reading `count` before setting it is UB. `-Wall` usually warns (`-Wuninitialized` / `-Wmaybe-uninitialized`). Fix: `int count = 0;`.</details>

---

## 6. Performance (the HFT angle)
| Where | Cost to create | Size limit | Notes |
|---|---|---|---|
| Stack | ~0 (one `sub rsp`) | MB-ish per thread | always hot in cache, freed automatically |
| Heap (`new`) | tens of ns, sometimes µs | RAM | may lock, may call the OS, causes fragmentation |
| Data/BSS (static) | 0 at run time | fixed at link time | one fixed address, but it's shared state (be careful with threads) |

- **Hot path rule:** no heap allocation while trading. Allocate pools and buffers at startup (heap or static), then reuse them.
- **Big static buffers** (order pools, ring buffers) in BSS are common in HFT: there's no allocation at all, and the address is fixed. But **pre-touch** them at startup (write one byte per 4 KB page), or the first real orders pay the page-fault cost (Day 10).
- **The stack is the fastest memory you have.** Its top is almost always in L1 cache. Small, fixed-size work buffers belong on the stack, and big ones don't.
- **Deep recursion** in a latency path is a stack-overflow risk and a cache cost. Use loops.
- **Constants in read-only data** can be shared between processes and can never be corrupted. Prefer `constexpr`/`const` tables (Day 04) for lookup tables.

---

## 7. Interview Questions
<details><summary>1. Name the memory regions of a process and what goes in each.</summary>Text (code), read-only data (constants, literals), data (initialized globals/statics), BSS (zero-initialized globals/statics), heap (dynamic), and stack (locals, arguments, return addresses). Also: shared libraries, memory-mapped files, and thread stacks.</details>
<details><summary>2. Difference between `.data` and `.bss`?</summary>`.data` holds non-zero initial values and takes space in the file. `.bss` holds zero-initialized objects: only its size is recorded, and the OS provides zero pages at load.</details>
<details><summary>3. Where does `int x = 0;` at global scope go? And `static int y = 3;` inside a function?</summary>`x` goes in BSS (zero). `y` goes in data (non-zero static).</details>
<details><summary>4. Where does a string literal live? Can you modify it?</summary>In read-only data. Modifying it is UB, and usually a crash.</details>
<details><summary>5. Why is stack allocation faster than heap allocation?</summary>The stack is just a pointer bump (`sub rsp`) that's freed automatically on return, and it's cache-hot. The heap runs allocator code (free lists, maybe locks or OS calls) and can fragment.</details>
<details><summary>6. Which way does the stack grow on x86?</summary>Down, toward lower addresses.</details>
<details><summary>7. What causes a stack overflow?</summary>Using more than the thread's stack limit: huge local arrays, or deep or infinite recursion.</details>
<details><summary>8. `int* p = new int;`: where is `p`, and where is the int?</summary>`p` is on the stack (if it's a local). The int it points to is on the heap.</details>
<details><summary>9. What is ASLR, and why does it exist?</summary>It randomizes the base addresses of the image, heap, stack and libraries each run, so exploits can't hard-code addresses.</details>
<details><summary>10. Is the layout the same on every OS?</summary>No. Linux puts the image low and the stack high. Windows (your machine) puts the stack low and the image high, and its sections are in a different order. What's portable: the kinds of regions, their permissions, zero-filled BSS, and the stack growing down on x86.</details>
<details><summary>11. Do globals and locals start at zero?</summary>Globals and statics do (static storage). Locals don't: they hold garbage until you assign them.</details>

---

## 8. Hard Questions (optional for now: come back after Day 15)
<details><summary>1. You declare a 1 GB zero-filled static array for an order pool. The binary is tiny and startup is instant, but the first minute of trading is slow. Why, and how do you fix it?</summary>BSS pages are mapped "zero on demand". Every first touch of a 4 KB page takes a page fault (a trip into the kernel to get a zeroed page): 262,144 faults for 1 GB, in the hot path. Fix: pre-fault at startup (write to every page, or use OS flags like Linux `MAP_POPULATE`), and consider huge pages and locking the memory (`mlock`) so it's never paged out. Day 10.</details>
<details><summary>2. Why does `static int counter = 1;` inside a function NOT need a "first call" check, when `static Book b(load_config());` does?</summary>A constant initializer is baked into `.data` at link time: there's nothing to do at run time. A dynamic initializer (a function call) must run on the first call, and the compiler adds a hidden thread-safe guard (a flag + a lock on the first call, Day 03 magic statics). That's a small check on every call afterwards. Prefer `constexpr`/`constinit` (Day 04) to keep it in `.data`.</details>
<details><summary>3. Where do the stacks of other threads live, and where does `thread_local` data live?</summary>Each thread's stack is a separate block the OS (or the thread library) allocates when it creates the thread, somewhere in the address space, usually with a guard page below it. `thread_local` variables come from a per-thread copy of the `.tls` template section (you can see `.tls` in `objdump -h`). Each thread gets its own block, reached through a register (`fs`/`gs`) or a TLS index on Windows.</details>

---

## 9. Mini Project: map your backtester
1. Run `objdump -h` (or `size`) on your backtester's `.exe`. Write down the sizes of `.text`, `.data`, `.rdata` and `.bss`. Which is the biggest, and why?
2. Add a debug flag that prints the address and region (use your `classify`) of: the order book object, its level storage (the pointer inside the vector), one strategy object, and one big lookup table. Are any big buffers on the stack? Should any heap buffer be a preallocated static instead?
3. If you have a big lookup table that never changes, make it `constexpr`/`const` and confirm with `objdump -h` that it moved from `.data` to `.rdata`.

---

## Checklist
- [ ] Watched the video(s) and read the theory
- [ ] `impl.cpp` passes all tests (`All Day 09 tests passed`) and prints a memory map
- [ ] Part B experiments written down
- [ ] `exercises.cpp`: Q1–Q4, BUG1–BUG3 done
- [ ] Mini project: backtester map
