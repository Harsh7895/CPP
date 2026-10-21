# Day 11 — `malloc/calloc/realloc/free` vs `new/delete`, `new[]/delete[]`, placement new
> Phase 1 · Est. time 40–45 min (paired with Day 10) · Builds on: Day 06 (`new[]/delete[]` rule), Day 08 (ownership, invalidation), Day 10 (pages, page faults)

## 🎯 Today in plain words
There are two families of heap allocation. C's `malloc` family hands you **raw bytes**. C++'s `new` hands you a **finished object**: it gets the bytes *and* runs the constructor.
**Placement new** splits those two jobs: "I already have the memory, just build the object here." That's the key to custom allocators.
You build a **bump (arena) allocator**: one big block, and each allocation just moves a pointer forward. On your machine it's about **1.9 ns per order vs about 28 ns** for `new`.

Why a trader cares: the hot path must never call the general-purpose heap (variable latency, locks, page faults). Arenas and pools, built on placement new, are how HFT code allocates orders and messages.

Files: `impl.cpp` (calloc/realloc done right, the arena, placement new, explicit destructors) and `exercises.cpp`.

---

## 📺 Watch first

| # | Video | Length | What to watch for |
|---|-------|--------|-------------------|
| 1 | **The Cherno: "The NEW Keyword in C++"**. Search YouTube for that exact title | short (about 15 min) | What `new` does (it calls `operator new`, then the constructor), `new[]`, and a first look at placement new. Covers §1.1–1.4. |
| 2 | [Back to Basics: Custom Allocators Explained, Kevin Carpenter, CppCon 2025](https://isocpp.org/blog/2026/04/cppcon-2025-back-to-basics-custom-allocators-explained-from-basics-to-advan) (the page links the video) | about 1 hr (watch the first 25 min) | Why custom allocators exist, and arena/bump allocators. Covers §1.6, §2.3 and §6. |

**Not covered well by the videos:** the `new[]` hidden count (§2.2), `realloc`'s traps (§1.3, BUG2), and your measured numbers (§6). Read those carefully.

---

## 0. Warm-up (Days 10 & 08)
1. *(Day 10)* Why did the first pass over fresh memory cost ~450 ns/page but the second only ~13?
2. *(Day 10)* Fix this: `align_up(x, a) = (x + a) & ~(a - 1)`.
3. *(Day 08)* Who should call `delete` on an object: anyone holding a pointer to it, or…?

<details><summary>Answers</summary>

1. The first pass page-faults on every page (the OS must find and zero a frame). The second pass is only a TLB miss plus a cache miss per page.
2. `(x + a - 1) & ~(a - 1)`. Without the `- 1`, an already-aligned `x` gets pushed up a whole step.
3. Only its single **owner**. Everyone else holds non-owning pointers and never deletes.
</details>

---

## 1. Theory

### 1.1 `malloc` vs `new`: bytes vs objects
**In one line:** `malloc` gives raw, uninitialized bytes. `new` gives bytes **and** runs the constructor, so you get a real object.

```cpp
struct NamedOrder { std::string sym; std::int64_t px; };

NamedOrder* a = new NamedOrder{"ESZ6", 295035};   // ✅ memory + constructor: a real object
delete a;                                          // ✅ destructor + free memory

NamedOrder* b = (NamedOrder*)std::malloc(sizeof(NamedOrder));  // bytes only: NOT an object yet
b->sym = "ESZ6";        // ❌ assigning to a std::string that was never constructed → crash (BUG1)
std::free(b);
```

| | `malloc` / `free` | `new` / `delete` |
|---|---|---|
| Returns | `void*` (you must cast) | `T*` (typed) |
| Runs constructor / destructor | ❌ | ✅ |
| On failure | returns `nullptr` | **throws** `std::bad_alloc` (or `nullptr` with `new (std::nothrow)`) |
| Size | you compute the bytes | the compiler knows `sizeof(T)` |
| Can resize | `realloc` | ❌ (use `std::vector`) |
| Pairing | `malloc` ↔ `free` | `new` ↔ `delete`, `new[]` ↔ `delete[]` |

**Never mix them:** `free(new T)` and `delete (T*)malloc(...)` are UB.
**Both are safe with null:** `free(nullptr)` and `delete nullptr` do nothing.

### 1.2 `calloc`: zeroed memory
```cpp
auto* px = (std::int64_t*)std::calloc(5, sizeof(std::int64_t));  // 5 int64, all ZERO
```
- It zeroes the memory for you. For big sizes it gets fresh OS pages, which are already zero, so it doesn't even write the zeros (Day 10 Q4: 1 GB in under 10 ms).
- It takes `(count, size)` separately and checks that `count * size` doesn't overflow.

### 1.3 `realloc`: resize, maybe move
**In one line:** `realloc(p, new_bytes)` makes a block bigger or smaller. It may **move** it to a new address, copying the old bytes.

```cpp
std::int64_t* p = (std::int64_t*)std::malloc(16);
std::int64_t* q = (std::int64_t*)std::realloc(p, 1 << 20);  // on your machine: it moved
// after success: use q. p is now INVALID (Day 08 invalidation, same idea as vector growth)
```
⚠️ **The classic leak:** `p = realloc(p, n);`. If realloc fails it returns `nullptr` and the **old block is still allocated**, but you just overwrote your only pointer to it (BUG2). Always use a temporary:
```cpp
void* t = std::realloc(p, n);
if (!t) { /* p is still valid: handle the error */ }
else    { p = static_cast<std::int64_t*>(t); }
```
⚠️ `realloc` copies **bytes**. It's only correct for trivially copyable types (ints, plain structs), never for `std::string` members.

### 1.4 What `new` really is: two steps
```cpp
Order* o = new Order{1, 295035, 10};
// the compiler turns it into roughly:
void*  mem = operator new(sizeof(Order));   // step 1: get bytes (usually calls malloc inside)
Order* o2  = ::new (mem) Order{1, 295035, 10};  // step 2: construct in those bytes

delete o;
// ≈  o->~Order();          // step 1: destroy
//    operator delete(o);   // step 2: free the bytes
```
`operator new` (the function) only allocates bytes, like `malloc`. The **new expression** (`new T`) does both steps. You can replace `operator new` globally (`exercises.cpp` does that to spy on it).

### 1.5 Placement new: "build the object HERE"
**In one line:** `::new (address) T(args)` constructs a `T` in memory you already have. It allocates nothing.

```cpp
alignas(Order) unsigned char buf[sizeof(Order)];   // raw storage, correctly aligned (Day 12)
Order* o = ::new (buf) Order{1, 295035, 10};       // construct in buf: no heap at all
// ... use o ...
o->~Order();          // explicit destructor call: destroy, but DON'T free (buf isn't from new)
```

| Rule | Why |
|---|---|
| The memory must be big enough **and aligned** for `T` | otherwise UB (misaligned objects) |
| Never `delete` a placement-new object | `delete` would try to free memory that wasn't from `new` |
| Call the destructor yourself (`p->~T()`) if `T` has one that matters | nobody else will (BUG3) |
| Write `::new` (with the `::`) | it always picks the global placement form, even if `T` has its own `operator new` |

**Analogy:** normal `new` is buying a house: the land *and* building it. Placement new is building a house on land you already own.

### 1.6 Bump (arena) allocator: the simplest fast allocator
**In one line:** grab one big block at startup, keep an offset, and each allocation rounds the offset up for alignment, then moves it forward. You "free" everything at once with `reset()`.

```
buf ─► [ order1 | order2 | pad | msg | order3 |            free            ]
                                               ▲ used_   (allocate = align + add + compare)
reset(): used_ = 0  → the whole block is reusable again
```
| | `new`/`malloc` | Arena |
|---|---|---|
| Allocate | search free lists, maybe lock, maybe call the OS | add + mask + compare |
| Free one object | yes | ❌ (only everything at once) |
| Fragmentation | can happen | none |
| Locality | objects scattered | objects back to back (cache-friendly) |
| Best for | general use | per-tick / per-message / per-day scratch data |

---

## 2. Internals: what actually happens

### 2.1 What `malloc` does inside (roughly)
- Small sizes: the allocator keeps **free lists** ("bins") of blocks per size class. `malloc` pops one, and `free` pushes it back. Each block carries a small header (its size and flags) just before your pointer.
- Big sizes (hundreds of KB or more): it asks the OS directly for fresh pages (`mmap` / `VirtualAlloc`), and `free` returns them. That's Day 10 territory: page faults on first touch.
- Thread safety: allocators use locks or per-thread caches, so latency varies.
- `malloc` always returns memory aligned for any normal type: `alignof(std::max_align_t)` = **16** on your machine.

### 2.2 The hidden count before `new[]` arrays
`delete[] p` must know **how many** destructors to run. Where does it find that? For types with a non-trivial destructor, `new[]` asks for **extra bytes** and stores the count just before the array:
```
new WithDtor[3]  → operator new[] asked for 20 bytes (3 × 4 + 8)   ← your g++ (Q1)
                   ┌──────────┬─────┬─────┬─────┐
                   │ count=3  │ [0] │ [1] │ [2] │
                   └──────────┴─────┴─────┴─────┘
                    8 bytes    ▲ the pointer you get back
new Plain[3]     → asked for 12 bytes: no destructor, so no count needed
```
This is why `delete` vs `delete[]` matters. `delete` doesn't look for the count, and it frees from the wrong address.

### 2.3 How `allocate(bytes, align)` works
```
base = address of buf_               (16-aligned, from malloc)
cur  = base + used_
aligned = align_up(cur, align)        ← align the ADDRESS, not just the offset
if (aligned - base + bytes > cap_) → nullptr
used_ = aligned - base + bytes
return aligned
```
Aligning the *offset* only works if `base` is already aligned to `align`. `malloc` gives you 16, so a request for 64 (a cache line) needs the real address.

> 🔍 **Going deeper (optional):** since C++14 the compiler may **delete** a `new`/`delete` pair completely if the pointer never "escapes". That's why `exercises.cpp` stores pointers into a `volatile` sink: without it, `-O2` printed `0 0` for Q1. g++ also assumes your replacement `operator new` doesn't touch other globals, so the spy counter had to be `volatile` too. Benchmarks of `new` must defeat the same optimization.

---

## 3. Implementation (`impl.cpp`)

Build: `g++ -std=c++20 -O2 -Wall -Wextra -pedantic impl.cpp -o impl` then `./impl`.

**Step 1: `alloc_prices(n)` and `grow_prices(&arr, old_n, new_n)`.** `calloc` for zeroed memory. `realloc` through a **temporary** pointer. Zero the new tail yourself (`realloc` doesn't). The test asks for an impossible size and checks that your old buffer is still intact.

**Step 2: `Arena`.** The constructor does one `std::malloc(capacity)` (store it as `std::byte*`), and the destructor does one `std::free`. `allocate(bytes, align)` follows §2.3. `reset()` sets `used_ = 0`.
<details><summary>Hint</summary>Work in `std::uintptr_t` for the address math: `auto base = reinterpret_cast<std::uintptr_t>(buf_);`. Return `buf_ + (aligned - base)`. That keeps the pointer derived from `buf_` (no integer-to-pointer casts needed).</details>

**Step 3: `make_order(a, id, px, qty)`.** `void* m = a.allocate(sizeof(Order), alignof(Order));`. If it's null, return null. Otherwise `return ::new (m) Order{id, px, qty};`.

**Step 4: `make_tracked` / `destroy_tracked`.** Same as Step 3 for a type with a destructor. Destroy with an **explicit destructor call** `t->~Tracked();`. The test checks that the alive count goes 0 → 2 → 0 while the arena's memory stays used.

### Part B (write it at the bottom of `impl.cpp`)
1. Replace placement new with a plain cast plus field writes. Fine for `Order`? For a struct with a `std::string`?
2. `std::free` a pointer from `new`. What happens?
3. Remove the capacity check and allocate past the end. Did anything stop you?

---

## 4. Output Questions

**Q1: what does `new[]` really ask for?** (with a spying `operator new[]`)
```cpp
struct WithDtor { int x; ~WithDtor() {} };
struct Plain    { int x; };
new WithDtor[3];   // request = ?
new Plain[3];      // request = ?
```
<details><summary>Answer</summary>`20 12`. The type with a destructor gets 8 extra bytes for the hidden element count (§2.2). The trivial type doesn't need one.</details>

**Q2**
```cpp
alignas(Loud) unsigned char buf[sizeof(Loud)];
std::printf("Q2: ");
Loud* p = ::new (buf) Loud;   // Loud prints "ctor " and "dtor "
std::printf("| ");
p->~Loud();
std::printf("| end\n");
```
<details><summary>Answer</summary>`Q2: ctor | dtor | end`. Placement new runs only the constructor. The explicit call runs only the destructor. No heap is involved.</details>

**Q3: bump offsets** (base 16-aligned; `bump` aligns, then adds)
```cpp
bump(used, 1, 1); bump(used, 8, 8); bump(used, 4, 4); bump(used, 16, 16);
```
<details><summary>Answer</summary>`0 8 16 32 used=48`. 1 byte at 0. Then align to 8 → 8 (7 bytes of padding). 16 is already 4-aligned → 16. Then 20 → align to 16 → 32. 48 bytes used for 29 bytes of data: order your allocations by alignment to waste less (Day 12).</details>

**Q4**
```cpp
volatile std::size_t huge = SIZE_MAX / 4;
char* p = new (std::nothrow) char[huge];        // ?
try { char* q = new char[huge]; } catch (const std::bad_alloc&) { /* ? */ }
```
<details><summary>Answer</summary>`nothrow gave nullptr, plain new threw bad_alloc`. Plain `new` never returns null: it throws. Checking `if (!p)` after a plain `new` is dead code.</details>

---

## 5. Debugging (`-DBUG1` … `-DBUG3`)

**BUG1: malloc then use as an object**
```cpp
NamedOrder* o = (NamedOrder*)std::malloc(sizeof(NamedOrder));
o->sym = "ESZ6-quarterly-future";
```
<details><summary>Answer</summary>The `std::string` member was never constructed, so `operator=` reads garbage internal pointers. On your machine: a crash (exit 139), and g++ warns "used uninitialized". Fix: `new NamedOrder{...}`, or `malloc` + placement new + an explicit destructor before `free`.</details>

**BUG2: `p = realloc(p, n)`**
```cpp
prices = (std::int64_t*)std::realloc(prices, huge);   // fails
```
<details><summary>Answer</summary>On failure `realloc` returns `nullptr` and leaves the old block allocated. You've lost the only pointer to it, so that's a leak (your run printed "LEAKED"). Fix: assign to a temporary first, as in impl Step 1.</details>

**BUG3: `reset()` without destructors**
```cpp
for (int i = 0; i < 3; ++i) ::new (arena.allocate(sizeof(Tracked))) Tracked;   // each owns a heap string
arena.reset();
```
<details><summary>Answer</summary>`reset()` only rewinds the offset. The 3 objects were never destroyed (alive = 3), so their strings' heap memory is leaked. Fix: call each destructor before `reset()`, or only put <b>trivially destructible</b> types in an arena (a common HFT rule: arena objects hold plain data, like ints, fixed char arrays and handles, never `std::string`).</details>

---

## 6. Performance (the HFT angle)
Measured on your machine, 1,000,000 × `Order` (24 B), `-O2`:
| Method | ns per object |
|---|---|
| `new Order{...}` | **~27–30** |
| `delete` | ~15–16 |
| Arena `make_order` (fresh memory) | ~3.5 |
| Arena `make_order` (pre-faulted, Day 10) | **~1.9** |

- **Arena allocation is ~15× faster** and, just as important, **predictable**: there's no lock, no search and no system call.
- **Pre-fault the arena** at startup: almost half of the "fresh" arena cost was page faults.
- **Objects back to back** means sequential memory, so iterating orders in an arena is cache- and prefetcher-friendly. `new`'d objects can be scattered.
- **Typical HFT pattern:** one arena per message / tick / trading day, `reset()` at the boundary; or a fixed-size **pool** (a free list of same-size slots, later in the plan) when objects die one at a time.
- **`realloc` on a hot path** can copy megabytes and invalidate every pointer. Size buffers up front.
- **Exceptions:** plain `new` can throw on the hot path. Many trading codebases build with no exceptions, use `nothrow` or pools, and treat allocation failure at startup as fatal.

---

## 7. Interview Questions
<details><summary>1. Difference between `malloc` and `new`?</summary>`malloc` returns raw, uninitialized bytes as `void*` and returns `nullptr` on failure. `new` allocates (via `operator new`), runs the constructor, returns `T*`, and throws `bad_alloc` on failure.</details>
<details><summary>2. Difference between `operator new` and the `new` expression?</summary>`operator new` is a function that only allocates bytes. The `new` expression calls it and then constructs the object. You can replace or overload `operator new`, but not the expression.</details>
<details><summary>3. What is placement new, and when do you use it?</summary>It constructs an object in memory you already have: `::new (p) T(args)`. It's used in allocators, pools, arenas, `std::vector` (which constructs elements in its raw buffer), `std::optional` and shared memory.</details>
<details><summary>4. How do you destroy an object made with placement new?</summary>Call the destructor explicitly with `p->~T()`, then release the memory however it was obtained. Never call `delete` on it.</details>
<details><summary>5. Why must `new[]` be paired with `delete[]`?</summary>`delete[]` runs every element's destructor, using a hidden count stored before the array (for non-trivial types), and frees from the right address. `delete` does neither: UB.</details>
<details><summary>6. What does `calloc` do that `malloc` doesn't?</summary>It zeroes the memory and checks the `count * size` multiplication for overflow.</details>
<details><summary>7. What are the dangers of `realloc`?</summary>It may move the block (old pointers become invalid). `p = realloc(p, n)` leaks on failure. It copies bytes, so it's wrong for non-trivially-copyable types.</details>
<details><summary>8. Does `new` ever return `nullptr`?</summary>Plain `new` never does (it throws `bad_alloc`). `new (std::nothrow)` returns `nullptr` on failure.</details>
<details><summary>9. What's a bump/arena allocator, and its trade-off?</summary>A pointer bump inside one big block: O(1), with no per-object metadata and great locality. But you can't free individual objects, only reset everything, and destructors are your job.</details>
<details><summary>10. What alignment does `malloc` guarantee?</summary>Enough for any fundamental type: `alignof(std::max_align_t)`, 16 on x86-64. For more (64 for cache lines) use `std::aligned_alloc`, `operator new(n, std::align_val_t{64})`, or align inside an arena.</details>
<details><summary>11. Can the compiler remove a `new`/`delete` pair?</summary>Yes, since C++14, if the allocation isn't observable (the pointer doesn't escape). Benchmarks must account for that.</details>

---

## 8. Hard Questions (optional for now: come back after Day 15)
<details><summary>1. Design an allocator for orders that are created and cancelled one at a time (not all reset together), with O(1) allocate/free and no system calls.</summary>A fixed-size <b>pool</b>: one pre-faulted array of N slots of `sizeof(Order)` (aligned), with free slots linked through their own memory (a free list: each free slot stores the index or pointer of the next free one). Allocate pops the head, and free pushes it back. Both are O(1) with no fragmentation. Add a generation counter to handles to catch use-after-free (Day 08). Construct with placement new and destroy with an explicit destructor call.</details>
<details><summary>2. `std::vector<T>` grows with `new T[n]`, right?</summary>No. It allocates <b>raw</b> memory (through its allocator, `operator new` under the hood), then constructs only `size()` elements with placement new (via `std::construct_at` / allocator traits). Capacity slots past `size()` are raw memory, not objects. `new T[n]` would construct all `n`, and would require `T` to be default-constructible. You'll build exactly this in MyVector (Day 33).</details>
<details><summary>3. Why might an arena be faster than `malloc` even if `malloc` were "free"?</summary>Locality: arena objects are contiguous in allocation order, so a pass over them streams through cache lines with the prefetcher. Heap objects of mixed lifetimes are scattered across pages, which means more cache and TLB misses. Also, there's no per-object header, so more objects fit per cache line.</details>

---

## 9. Mini Project: arena-backed event buffer for your backtester
1. Give your backtester a per-bar (or per-day) `Arena`. Pre-fault it at startup with your Day 10 `touch_pages` idea.
2. Allocate every `Order` / fill event created while processing that bar with `make_order` (placement new), and call `reset()` when the bar ends. Keep these types **trivially destructible** (check with `static_assert(std::is_trivially_destructible_v<Order>);`).
3. Benchmark one backtest run: `new`/`delete` vs the arena. Write down the total time and ns per event.
4. Write 2 lines: what would break if a strategy kept an `Order*` from the arena past `reset()`? (Day 08: a dangling pointer. Use ids instead.)

---

## Checklist
- [ ] Watched the video(s) and read the theory
- [ ] `impl.cpp` passes all tests (`All Day 11 tests passed`)
- [ ] Part B written down
- [ ] `exercises.cpp`: Q1–Q4, BUG1–BUG3 done
- [ ] Mini project numbers written down
