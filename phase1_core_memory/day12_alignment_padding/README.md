# Day 12 — Alignment, padding, `sizeof`/`alignof`, object representation, `std::align`, fragmentation
> Phase 1 · Est. time 40–45 min (paired with Day 13) · Builds on: Day 05 (the 24-byte `Order`), Day 10 (`align_up`, cache lines), Day 11 (arena alignment)

## 🎯 Today in plain words
The CPU likes each value to sit at an address that's a multiple of its size: an 8-byte price at an address divisible by 8. To make that happen, the compiler adds invisible **padding bytes** inside your structs.
Put the fields in a bad order and your struct gets fat: today's order message is **32 bytes** written one way and **16 bytes** written the other, with exactly the same data.

Why a trader cares: half the size means twice as many orders per 64-byte cache line, per MB of L2, and per network packet. And one alignment trick (`alignas(64)`) stops two CPU cores from fighting over the same cache line.

Files: `impl.cpp` (reorder a struct, write the compiler's layout algorithm, cache-line counters, `std::align`, correct comparison) and `exercises.cpp`.

---

## 📺 Watch first

| # | Video | Length | What to watch for |
|---|-------|--------|-------------------|
| 1 | [Data-Oriented Design and C++, Mike Acton, CppCon 2014 keynote](https://isocpp.org/blog/2015/01/cppcon-2014-data-oriented-design-and-c-mike-acton) (the page links the video) | about 1.5 hr (watch the first 30–40 min) | The mindset: "look at your data, count the bytes, count the cache lines". This is why §6 matters. He does it with game structs, and you'll do it with orders. |

There is no short beginner video I could verify for padding rules. §1 and §2 below are the main material. Read them carefully.

---

## 0. Warm-up (Days 10–11)
1. *(Day 11)* `allocate(8, 8)` right after `allocate(1, 1)` in a 16-aligned arena: at which offset does it start? Why?
2. *(Day 10)* `align_up(x, a) = (x + a - 1) & ~(a - 1)`. Why must `a` be a power of two?
3. *(Day 11)* Why did `new WithDtor[3]` ask for 20 bytes instead of 12?

<details><summary>Answers</summary>

1. At offset 8. Offset 1 isn't a multiple of 8, so the arena skips 7 bytes. Those 7 bytes are **padding**, exactly what the compiler does inside structs today.
2. The mask trick needs `a - 1` to be "all low bits set" (`0b0111` for 8). For 6, `6 - 1 = 0b101` isn't a clean mask.
3. 8 extra bytes hold the hidden element count, so `delete[]` knows how many destructors to run.
</details>

---

## 1. Theory

### 1.1 `alignof`: the address rule for each type
**In one line:** `alignof(T)` is the number the address of every `T` must be divisible by.

```cpp
std::printf("%zu %zu %zu %zu %zu\n",
    alignof(char), alignof(short), alignof(int), alignof(double), alignof(void*));
// prints: 1 2 4 8 8      (and alignof(long double) = 16 on your g++)
```
| Type | `sizeof` | `alignof` |
|---|---|---|
| `char` | 1 | 1 |
| `short` | 2 | 2 |
| `int`, `float`, `std::int32_t` | 4 | 4 |
| `double`, `std::int64_t`, pointers | 8 | 8 |
| `long double`, `std::max_align_t` | 16 | 16 |

**Why the CPU cares:** memory moves in aligned chunks (cache lines of 64 bytes). An aligned 8-byte value never crosses a chunk boundary, so the load is a single access. A misaligned one may straddle two cache lines (or two pages, Day 10).

### 1.2 Padding: the invisible bytes inside structs
**In one line:** the compiler places members **in the order you wrote them**, inserting padding before each one so it's aligned, and adding **tail padding** at the end so arrays of the struct stay aligned.

```cpp
struct BadOrder {
    char         side;   // offset 0   then 7 padding bytes (px needs a multiple of 8)
    std::int64_t px;     // offset 8
    char         type;   // offset 16  then 3 padding bytes (qty needs a multiple of 4)
    std::int32_t qty;    // offset 20
    char         flag;   // offset 24  then 7 bytes of TAIL padding (size must be a multiple of 8)
};
static_assert(sizeof(BadOrder) == 32);   // 15 bytes of data, 17 bytes of padding!
```
```
BadOrder (32 B):
|s|.......|pxpxpxpx|t|...|qqqq|f|.......|
 0 1     8        16 17 20   24 25     32      . = padding
```

### 1.3 The fix: order members from biggest alignment to smallest
```cpp
struct GoodOrder {
    std::int64_t px;     // 0
    std::int32_t qty;    // 8
    char side;           // 12
    char type;           // 13
    char flag;           // 14   then 1 byte of tail padding
};
static_assert(sizeof(GoodOrder) == 16);  // same data, half the size
```
```
GoodOrder (16 B):
|pxpxpxpx|qqqq|s|t|f|.|
 0        8    12      16
```
**Rule of thumb:** sort members by `alignof`, largest first. It's free, and it often saves 25–50%.

### 1.4 `alignas`: ask for MORE alignment
**In one line:** `alignas(N)` makes a type or variable start at a multiple of N (a power of two, at least its natural alignment).

```cpp
struct alignas(64) PaddedCounter { std::uint64_t value; };
static_assert(sizeof(PaddedCounter) == 64);   // grows to a full cache line
PaddedCounter per_core[4];                    // each element on its own cache line
```
**Why:** if two cores write two counters that share one 64-byte line, the line bounces between their caches on every write. That's called **false sharing**, and it can make "independent" counters many times slower. You'll measure it in the concurrency phase. `std::hardware_destructive_interference_size` (in `<new>`) is the portable name for that 64.

### 1.5 `#pragma pack` / `__attribute__((packed))`: remove padding (dangerous)
```cpp
struct __attribute__((packed)) Wire { char type; std::int64_t px; };   // sizeof == 9, px at offset 1
```
It's used to describe **exact wire formats**. But `px` is misaligned, and taking `&w.px` as a normal `std::int64_t*` is a trap (BUG2: g++ warns). The better way is to parse wire bytes with `memcpy` into a normal struct (Day 08).

### 1.6 Object representation: the bytes, including padding
**In one line:** an object's **representation** is all its `sizeof` bytes, and the **padding bytes hold unspecified garbage**.

```cpp
GoodOrder a{295035, 10, 'B', 'L', 0};
GoodOrder b{295035, 10, 'B', 'L', 0};
std::memcmp(&a, &b, sizeof a);   // ❌ may say "different": the padding byte can differ (BUG1)
a.px == b.px && a.qty == b.qty && ...;   // ✅ compare fields
bool operator==(const GoodOrder&) const = default;  // ✅ C++20: the compiler writes it for you
```
The same goes for hashing a struct's raw bytes or sending it raw over the network: padding leaks garbage (even old secrets) and breaks equality. `std::has_unique_object_representations_v<T>` tells you whether a type has **no** padding.

### 1.7 `std::align`: find an aligned spot inside a buffer
```cpp
alignas(16) unsigned char buf[100];
void* p = buf + 3;  std::size_t space = 97;
void* r = std::align(16, 24, p, space);   // want 24 bytes at a 16-aligned address
// r == buf + 16 (it moved 13 bytes), space is now 84, and p was updated too
```
It's the standard-library version of the arena math you wrote on Day 11. It returns `nullptr` if there's no room.

### 1.8 Fragmentation: wasted memory, two kinds
| Kind | What | Example |
|---|---|---|
| **Internal** | waste *inside* a block you got | struct padding; `malloc(17)` really using a 32-byte block; arena alignment gaps |
| **External** | free memory exists, but it's in small **holes** that can't fit a big request | after many `new`/`delete` of mixed sizes: 100 MB free in total, but no 1 MB piece |

```
heap after mixed alloc/free:  [used][free 8K][used][free 16K][used][free 8K]
want 32 KB → fails or grows the heap, though 32 KB are free in total
```
Fixes: allocate up front, use same-size pools (no holes), use arenas (reset all at once), and order struct members.

---

## 2. Internals: what actually happens

### 2.1 The layout algorithm (you implement it in Step 2)
```
offset = 0, struct_align = 1
for each member, in declaration order:
    offset = align_up(offset, alignof(member))   ← padding before the member
    member lives at offset
    offset += sizeof(member)
    struct_align = max(struct_align, alignof(member))
sizeof(struct) = align_up(offset, struct_align)   ← tail padding
alignof(struct) = struct_align
```
The compiler is **not allowed to reorder** members (within one access level). C++ promises that members sit in declaration order, so you must do the reordering yourself.

### 2.2 Why tail padding exists
```cpp
struct DC { double d; char c; };    // data = 9 bytes, sizeof = 16
DC arr[2];   // arr[1].d must be 8-aligned → arr[1] starts at 16, not 9
```
`sizeof` already includes the tail padding, so `&arr[i] = base + i * sizeof(T)` is always aligned.

### 2.3 Empty structs are 1 byte
`sizeof(Empty) == 1`, so that two distinct objects always have distinct addresses. Inside another struct it can still cost a full alignment step (`WithE` is 8, not 4). C++20's `[[no_unique_address]]` lets an empty member take 0 bytes.

### 2.4 Misaligned access on x86
x86-64 *can* load a misaligned `int64_t`. It's a little slower, and much slower if it crosses a cache line or a page. But it's still UB in C++ to access through a misaligned `T*`. The compiler may use instructions that **require** alignment (SSE `movaps`, for example), and then you get a crash. ARM CPUs are less forgiving.

---

## 3. Implementation (`impl.cpp`)

Build: `g++ -std=c++20 -O2 -Wall -Wextra -pedantic impl.cpp -o impl` then `./impl`.

**Step 1: reorder `GoodOrder`** (same names and types) so `sizeof == 16`.

**Step 2: `layout_size(members, n)`.** Implement §2.1 exactly. The tests check your function against the real `sizeof` of `BadOrder` (32), a `{char, short, char}` struct (6), `{double, char}` (16) and an empty struct (1).
<details><summary>Hint</summary>Loop: `off = align_up(off, m[i].align) + m[i].size;` and keep the max alignment. At the end, `return align_up(off, max_align);`. Handle `n == 0` first.</details>

**Step 3: `PaddedCounter`.** Add one word so each counter takes exactly one cache line.

**Step 4: `is_aligned` and `place_aligned`.** `is_aligned`: convert to `std::uintptr_t` and use `%`. `place_aligned`: copy `buf` and `len` into local variables, and call `std::align(align, size, ptr, space)`. It updates them and returns the aligned pointer or `nullptr`.

**Step 5: `same_order`.** Compare all five fields. The tests fill the padding with different garbage (`0xAA` vs `0x55`) and expect "equal".
<details><summary>Bonus</summary>Add `bool operator==(const GoodOrder&) const = default;` to `GoodOrder`, and implement `same_order` as `return a == b;`.</details>

### Part B (write it at the bottom of `impl.cpp`)
1. `offsetof` of every `BadOrder` member.
2. How much padding is left in `GoodOrder`, and where?
3. Make `layout_size` checkable with `static_assert`. Why does that work?
4. Orders per cache line and per MB: `BadOrder` vs `GoodOrder`.

---

## 4. Output Questions

**Q1**
```cpp
struct Tiny  { char a; short b; char c; };
struct DC    { double d; char c; };
struct Empty {};
struct WithE { int x; Empty e; };
std::printf("%zu %zu %zu %zu\n", sizeof(Tiny), sizeof(DC), sizeof(Empty), sizeof(WithE));
```
<details><summary>Answer</summary>`6 16 1 8`. Tiny: a@0, pad 1, b@2, c@4, tail pad 1 → 6. DC: 9 bytes of data plus 7 bytes of tail pad. Empty is 1 (distinct addresses). WithE: x@0, e@4, size 5 → rounded up to 8.</details>

**Q2**
```cpp
struct Msg { char type; std::int64_t px; std::int32_t qty; };
// offsetof(type), offsetof(px), offsetof(qty), sizeof
```
<details><summary>Answer</summary>`type@0 px@8 qty@16 size 24`. Reordered `{px, qty, type}`, it would be 16.</details>

**Q3**
```cpp
struct alignas(64) Line { std::int64_t v; };
Line arr[3];
// sizeof(Line), sizeof(arr), distance from &arr[0] to &arr[2] in bytes
```
<details><summary>Answer</summary>`64 192 128`. `alignas(64)` raises both the alignment and the size (sizeof is always a multiple of the alignment).</details>

**Q4**
```cpp
alignas(16) unsigned char buf[100];
void* p = buf + 3;  std::size_t space = 97;
void* r = std::align(16, 24, p, space);
// how far did it move? space left? p == r?
```
<details><summary>Answer</summary>`moved 13, space left 84, p==r 1`. The next 16-aligned address after `buf + 3` is `buf + 16`. `std::align` updates `p` and `space` <b>in place</b>.</details>

---

## 5. Debugging (`-DBUG1` … `-DBUG3`)

**BUG1: `memcmp` on structs**
```cpp
struct Quote { std::int64_t px; std::int32_t qty; char side; };   // 3 bytes of tail padding
// a and b have equal fields, but come from different buffers
std::memcmp(&a, &b, sizeof a) == 0   // ?
```
<details><summary>Answer</summary>It says <b>NO</b> on your run: the 3 padding bytes differ. Fix: compare fields, or use `= default` `operator==`. The same bug hits hashing raw struct bytes and dedup by bytes.</details>

**BUG2: a pointer to a packed member**
```cpp
struct __attribute__((packed)) Wire { char type; std::int64_t px; };
bump(&w.px);   // void bump(std::int64_t* p) { *p += 1; }
```
<details><summary>Answer</summary>`&w.px` is a misaligned `int64_t*` (offset 1). g++ warns (`-Waddress-of-packed-member`). It "works" on x86 here (prints 295036), but it's UB, and on other CPUs or with vectorized code it can crash. Fix: copy out with `memcpy`, modify, and copy back, or don't use packed structs outside the parsing layer.</details>

**BUG3: placement new at a misaligned spot**
```cpp
unsigned char raw[256];
Line* l = ::new (raw + 1) Line{42};   // Line is alignas(64)
```
<details><summary>Answer</summary>The object starts at an address with `% 64 == 1`, which is UB (Day 11 rule: the memory must be aligned). Fix: `std::align(alignof(Line), sizeof(Line), p, space)` first (impl Step 4), or `alignas(Line) unsigned char raw[...]`.</details>

---

## 6. Performance (the HFT angle)
| | `BadOrder` (32 B) | `GoodOrder` (16 B) |
|---|---|---|
| Per 64 B cache line | 2 | **4** |
| Per 1 MB (L2-ish) | 32,768 | **65,536** |
| 1,000,000 orders | 32 MB | **16 MB** |

- **Smaller struct = fewer cache misses.** Scanning a book of 16-byte levels touches half the cache lines. Each RAM miss is about 60–100 ns (Day 06).
- **Hot/cold splitting:** keep the fields the hot path reads (price, qty, side) in one compact struct, and move rarely used ones (client id, text notes, timestamps for audit) to a separate array.
- **`alignas(64)` per-thread data** avoids false sharing between cores (counters, queue head/tail indices).
- **Don't pack for speed.** Packed structs save bytes, but cost misaligned loads and UB traps. Reorder instead.
- **Wire vs memory:** use a packed or exact layout only at the network boundary, parse with `memcpy`, and keep the internal structs naturally aligned and reordered.
- **Arrays of structs vs a struct of arrays:** if a loop reads only `px`, an array of just prices (SoA = "structure of arrays") is the densest possible layout. You'll revisit this in the performance phase.

---

## 7. Interview Questions
<details><summary>1. What is alignment, and why does the CPU care?</summary>The requirement that an object's address be a multiple of `alignof(T)`. Aligned accesses never straddle cache lines or pages, so they're single, fast accesses, and some instructions require them.</details>
<details><summary>2. Why is `sizeof(struct)` bigger than the sum of its members?</summary>Padding: between members to align each one, and at the end so arrays stay aligned.</details>
<details><summary>3. How do you minimize padding?</summary>Order members by decreasing alignment (8-byte, then 4, 2, 1). Group small fields together.</details>
<details><summary>4. Can the compiler reorder members to remove padding?</summary>No. Members are laid out in declaration order (within an access specifier). You must reorder them yourself.</details>
<details><summary>5. What is tail padding for?</summary>So that `sizeof` is a multiple of the alignment, and every element of an array is aligned.</details>
<details><summary>6. What's `sizeof` an empty class? Why?</summary>1, so distinct objects have distinct addresses. As a base class it can take 0 bytes (empty base optimization), and as a member with `[[no_unique_address]]` too.</details>
<details><summary>7. What does `alignas(64)` do, and why use it?</summary>It forces 64-byte alignment (and a size that's a multiple of 64), putting the object on its own cache line, to avoid false sharing between cores.</details>
<details><summary>8. What is false sharing?</summary>Two cores write different variables that share one cache line. The line ping-pongs between their caches, so the "independent" writes become slow.</details>
<details><summary>9. Why is `memcmp` on structs dangerous?</summary>Padding bytes have unspecified values, so equal objects can compare unequal. Raw-byte hashing or serialization has the same problem.</details>
<details><summary>10. What's the danger of packed structs?</summary>Misaligned members: slower access, and taking their address gives misaligned pointers. Using those is UB, and it can crash on some CPUs or with SIMD code.</details>
<details><summary>11. Internal vs external fragmentation?</summary>Internal: waste inside allocated blocks (padding, size-class rounding). External: free memory split into holes too small for requests.</details>

---

## 8. Hard Questions (optional for now: come back after Day 15)
<details><summary>1. Your price-level struct is `{bool active; double px; int qty; bool dirty; int order_count;}` (32 bytes). Shrink it, and say what else you'd change for a hot book scan.</summary>Reorder to `{double px; int qty; int order_count; bool active; bool dirty;}` → 8+4+4+1+1 = 18, rounded up to 24 (from 32). Better: use an `int64` fixed-point price (no float compares), pack the two bools into one flags byte, and maybe store `qty` and `order_count` as `int32`, giving 16 bytes (4 per line). Hot/cold split: move `order_count` out if the scan doesn't need it. And consider SoA if the scan reads only prices.</details>
<details><summary>2. Why must `sizeof(T)` be a multiple of `alignof(T)`?</summary>Because array indexing is `base + i * sizeof(T)`. If the size weren't a multiple of the alignment, element 1 would be misaligned. Tail padding guarantees it.</details>
<details><summary>3. You send `GoodOrder` to another process by `memcpy`-ing it into shared memory. What can go wrong, and what's a safe design?</summary>Padding garbage leaks (and breaks byte-level dedup or checksums). The layout depends on the compiler, ABI and flags (both sides must agree). Endianness, if the machines differ. Safe: define an explicit wire format (fixed offsets, fixed-width little-endian fields), `static_assert` the sizes and offsets on both sides, zero the padding or have no padding (`has_unique_object_representations`), and version the struct.</details>

---

## 9. Mini Project: shrink your backtester's hot structs
1. For each struct your backtester keeps many of (bar/candle, tick, order, fill, position), print `sizeof` and every `offsetof`.
2. Reorder each to remove padding. Add `static_assert(sizeof(Bar) == N);` so a future change can't silently grow it.
3. Benchmark: fill a `std::vector` with 10 M of your bar struct and sum one field. Before vs after reordering, in ns per element.
4. Bonus: try SoA for the hottest loop (a separate `std::vector<std::int64_t> close_px;`) and compare.

---

## Checklist
- [ ] Watched the video and read the theory
- [ ] `impl.cpp` passes all tests (`All Day 12 tests passed`)
- [ ] Part B written down
- [ ] `exercises.cpp`: Q1–Q4, BUG1–BUG3 done
- [ ] Mini project sizes and numbers written down
