# Day 06 — Raw pointers, pointer arithmetic, pointer-to-pointer, const pointers, `nullptr` vs `NULL`
> Phase 1 · Est. time 40–45 min (paired with Day 07) · Builds on: Day 03 (storage duration), Day 04 (`const`), Day 05 (`using Price = std::int64_t`)

## 🎯 Today in plain words
A **pointer** is a variable that holds a **memory address**: "where something lives", not the thing itself.
Today you learn to make pointers, follow them, move them along an array, and lock them with `const`.
You also build a **2D price grid** (instruments × minutes) by hand, in two ways, and see why one way is much faster.

Why a trader cares: every order book, ring buffer and market-data parser in HFT is, under the hood, pointers walking over memory.
If you can "see" the addresses, you can see why one layout hits the cache and another doesn't.

Files: `impl.cpp` (you implement it, step by step) and `exercises.cpp` (predict the output, will it compile, bugs).

---

## 📺 Watch first

| # | Video | Length | What to watch for |
|---|-------|--------|-------------------|
| 1 | **The Cherno: "POINTERS in C++"** (episode 16 of his C++ series). Search YouTube for that exact title | short (about 15–20 min) | The friendliest start: memory is one long street of numbered houses, and a pointer is a house number. Covers §1.1–1.2. |
| 2 | **Back to Basics: Pointers, Mike Shah, CppCon 2021.** Search YouTube for "Back to Basics: Pointers Mike Shah CppCon 2021" | about 1 hr (watch the first 25–30 min) | Pointer arithmetic, arrays vs pointers, pointer-to-pointer, `const`. Covers §1.3–1.6. |

**Not covered well by the videos:** jagged vs contiguous 2D arrays (§2.3), and the cache and latency angle (§6). Read those carefully.

---

## 0. Warm-up (Day 05 + a Day 02 refresher)
1. `enum class Side : std::uint8_t { Buy, Sell };` then `int x = Side::Buy;`. Does it compile? Why?
2. What's the size and the contents of `std::vector<int>(3, 1)` and of `std::vector<int>{3, 1}`?
3. *(Day 02, in simple words)* What's the difference between a **declaration** and a **definition**?

<details><summary>Answers</summary>

1. No. An `enum class` never turns into an `int` by itself. You must write `static_cast<int>(Side::Buy)`.
2. `(3, 1)` means "3 copies of 1", so `{1, 1, 1}` (size 3). `{3, 1}` means "this exact list", so `{3, 1}` (size 2).
3. A declaration says "this name exists, and here is its type" (like a phone-book entry). A definition actually creates it: it gives a function its body, or gives a variable its memory. You may declare many times but define only once (the ODR).
</details>

---

## 1. Theory

### 1.1 What a pointer is
**In one line:** a pointer stores the **address** of another object. `&x` gets the address, and `*p` follows it ("dereference").

```cpp
std::int64_t px = 295035;          // a price (fixed-point, 2 decimals)
std::int64_t* p = &px;             // p holds the ADDRESS of px
std::cout << p  << '\n';           // prints something like 0x61fe10 (an address)
std::cout << *p << '\n';           // prints 295035 (the value AT that address)
*p = 295100;                       // writes through the pointer
std::cout << px << '\n';           // prints 295100: px itself changed
std::cout << sizeof(p) << '\n';    // prints 8 on a 64-bit machine
```

**Analogy:** memory is a long street of numbered houses (one house = one byte). `px` is a house. `p` is a sticky note with the house number written on it. `*p` means "walk to that house".

**Why care:** a pointer is always 8 bytes (on 64-bit), no matter how big the thing it points to is. Passing an 8-byte address is cheap. Copying a 10 KB order book is not.

### 1.2 `nullptr` vs `NULL`
**In one line:** `nullptr` means "points to nothing" and has its own type (`std::nullptr_t`). `NULL` is an old C macro that is really just an **integer** 0.

```cpp
void send(int qty)     { std::cout << "int\n"; }
void send(int* order)  { std::cout << "pointer\n"; }

send(0);         // prints "int"     (0 is an int)
send(nullptr);   // prints "pointer" (nullptr can only be a pointer)
send(NULL);      // ❌ on your g++: "call of overloaded 'send(NULL)' is ambiguous"
                 //    (on other compilers it may silently pick send(int)!)
```

| | `NULL` | `nullptr` |
|---|---|---|
| What it really is | an integer (`0`, `0L` or `__null`) | its own type, `std::nullptr_t` |
| Can become an `int`? | yes 😬 | no ✅ |
| Overloads | ambiguous, or picks the wrong one | always picks the pointer version |

**Rule:** always write `nullptr`. Always give a pointer a starting value: `int* p = nullptr;`.

**Why care:** an uninitialized pointer holds a random address (a "wild pointer"). Writing through it can corrupt any memory, for example the price you're about to send.

### 1.3 Pointer arithmetic: `p + 1` moves by one *element*, not one byte
**In one line:** adding `n` to a `T*` moves it forward by `n * sizeof(T)` bytes.

```cpp
std::int64_t prices[4] = {100, 101, 99, 105};
std::int64_t* p = prices;     // an array name turns into a pointer to its first element
std::cout << *p       << '\n';  // 100
std::cout << *(p + 2) << '\n';  // 99   (moved 2 * 8 = 16 bytes)
std::cout << p[2]     << '\n';  // 99   (p[i] is just a short way to write *(p + i))
std::int64_t* end = prices + 4; // "one past the end": allowed to MAKE, NOT allowed to read
std::cout << (end - p) << '\n'; // 4    (the difference is in ELEMENTS, type std::ptrdiff_t)
```

**Rules (break them = undefined behaviour, "UB" = the program may do anything):**
| ✅ Allowed | ❌ UB |
|---|---|
| Point anywhere inside the array | Point before the first element (`prices - 1`) |
| Point **one past** the last element | Point two or more past the end (`prices + 5`) |
| Subtract two pointers into the **same** array | Subtract pointers into **different** arrays |
| Read `*p` when p points at a real element | Read `*end` (one past the end) |

**Why care:** the `[first, last)` style, "start pointer + one-past-end pointer", is how every STL algorithm works. You'll use it in `impl.cpp`.

### 1.4 Arrays "decay" into pointers
**In one line:** when you pass an array to a function, it becomes a pointer to its first element, and the size is **lost**.

```cpp
void show(std::int64_t arr[]) {            // really means: std::int64_t* arr
    std::cout << sizeof(arr) << '\n';      // 8: the size of a POINTER (g++ even warns you)
}
std::int64_t prices[4]{};
std::cout << sizeof(prices) << '\n';       // 32: 4 elements * 8 bytes
show(prices);                              // 8
```

**Fix:** always pass the size along (`ptr, count`) or a pair (`first, last`). Day 52: `std::span`, which keeps the pointer and the size together.

### 1.5 `const` and pointers: read it **right to left**
There are **two** things that can be `const`: the pointer itself, or the thing it points to.

```cpp
std::int64_t a = 1, b = 2;

const std::int64_t* p1 = &a;  // "p1 is a pointer to a const int64": can't change *p1, CAN move p1
*p1 = 5;        // ❌
p1  = &b;       // ✅

std::int64_t* const p2 = &a;  // "p2 is a const pointer to an int64": CAN change *p2, can't move p2
*p2 = 5;        // ✅
p2  = &b;       // ❌

const std::int64_t* const p3 = &a;  // both locked
```

| Declaration | Change the value? | Move the pointer? | Read it right to left |
|---|---|---|---|
| `const T* p` (same as `T const* p`) | ❌ | ✅ | pointer to const T |
| `T* const p` | ✅ | ❌ | const pointer to T |
| `const T* const p` | ❌ | ❌ | const pointer to const T |

**Trick:** `const` applies to whatever is on its **left**. If nothing is on its left, it applies to whatever is on its right.

**Why care:** `const T*` in a function signature is a promise: "I only **read** your data". A market-data handler takes `const Tick*` so it can't corrupt the feed buffer.

### 1.6 Pointer to pointer (`T**`)
**In one line:** a pointer that holds the address of **another pointer**, so a function can change *which thing* the caller's pointer points at.

```cpp
void advance(const std::int64_t** cursor) {  // receives the ADDRESS of the caller's pointer
    ++(*cursor);                             // moves the caller's pointer forward
}
std::int64_t ticks[3] = {10, 20, 30};
const std::int64_t* cur = ticks;
advance(&cur);
std::cout << *cur << '\n';                   // 20
```

```
  cursor (T**)          cur (T*)              ticks[]
 ┌──────────┐        ┌──────────┐        ┌────┬────┬────┐
 │ &cur     │ ─────► │ &ticks[1]│ ─────► │ 10 │ 20 │ 30 │
 └──────────┘        └──────────┘        └────┴────┴────┘
```

Other common uses: `char** argv` in `main`, and an **array of pointers** (the "jagged" 2D array in §2.3).

### 1.7 `new[]` and `delete[]`: just enough for today
**In one line:** `new T[n]` asks the heap for room for `n` objects and gives you a pointer to the first one. `delete[] p` gives it back.

```cpp
double* row = new double[390]{};   // 390 doubles, all zero (the {} zeroes them)
row[0] = 1.5;
delete[] row;                      // the [] MUST match the new[]
row = nullptr;                     // good habit: no dangling address left behind
```

| You wrote | You must write | Mixing them up |
|---|---|---|
| `new T` | `delete p` | UB |
| `new T[n]` | `delete[] p` | UB |

The full story (what `new` really calls, `malloc` vs `new`, placement new) is **Day 11**. Today you only need the rule above.

### 1.8 Summary table
| Thing | Syntax | Remember |
|---|---|---|
| address of | `&x` | gives a `T*` |
| dereference | `*p`, `p[i]`, `p->member` | `p->m` is `(*p).m` |
| null | `nullptr` | never `NULL` or `0` in new code |
| arithmetic | `p + n`, `p - q` | moves by `n * sizeof(T)` bytes |
| read-only data | `const T*` | promise "I only read" |
| fixed pointer | `T* const` | the pointer can't move |
| pointer to pointer | `T**` | changes the caller's pointer |

---

## 2. Internals: what actually happens

### 2.1 A pointer is just an 8-byte number
On x86-64 a pointer is a 64-bit integer: a **virtual address** (Day 10 explains "virtual"). The CPU doesn't care about the type. The type only exists while compiling, so the compiler knows **how many bytes to read** and **how far `+1` moves**.

```
address:   0x1000   0x1008   0x1010   0x1018
          ┌────────┬────────┬────────┬────────┐
prices:   │  100   │  101   │   99   │  105   │   each box = 8 bytes (int64)
          └────────┴────────┴────────┴────────┘
             ▲                 ▲
             p                 p + 2  == 0x1000 + 2*8 == 0x1010
```

### 2.2 `p[i]` compiles to one instruction
`p[i]` with `std::int64_t* p` compiles to something like:
```asm
mov rax, QWORD PTR [rdi + rsi*8]    ; rdi = p, rsi = i, *8 = sizeof(int64)
```
x86 can do "base + index × scale" inside one load, so indexing is basically free. What costs time is **where** the memory is (cache or RAM), not the math.

### 2.3 Two ways to build a 2D grid
You want `grid[instrument][minute]`: 3 instruments × 4 minutes.

**A) Jagged: an array of row pointers (`double**`)**: `rows + 1` separate heap allocations.
```
grid ──► ┌──────┐
         │ ptr0 │──► [ . . . . ]   (somewhere in the heap)
         │ ptr1 │──────────────────────► [ . . . . ]   (somewhere else)
         │ ptr2 │──► [ . . . . ]   (somewhere else again)
         └──────┘
```
Reading `grid[r][c]` = load `grid[r]` (a pointer), **then** load `[c]`. Two memory reads, and the rows can be far apart.

**B) Contiguous: one block, you do the math**: 1 allocation.
```
data ──► [ r0c0 r0c1 r0c2 r0c3 | r1c0 r1c1 r1c2 r1c3 | r2c0 r2c1 r2c2 r2c3 ]
                                    ▲
                     index of (r=1, c=2) = r * cols + c = 1*4 + 2 = 6
```
Reading `(r, c)` = one multiply-add and **one** memory read. Rows sit next to each other. This is called **row-major** order (the same as C arrays like `double g[3][4]`).

| | Jagged `double**` | Contiguous `double*` + `r*cols+c` |
|---|---|---|
| Allocations | `rows + 1` | 1 |
| Reads per element | 2 (pointer, then value) | 1 |
| Rows can have different lengths | yes | no |
| Cache-friendly when scanning all of it | no (rows scattered) | yes |
| Freeing | loop, then free the spine | one `delete[]` |

> 🔍 **Going deeper (optional):** `double g[3][4]` (a built-in 2D array) is contiguous, but it is **not** a `double**`. It decays to `double (*)[4]`, "pointer to an array of 4 doubles". That's why you can't pass it to a function that takes `double**`. Try it in `exercises.cpp` C5.

---

## 3. Implementation (`impl.cpp`)

Build: `g++ -std=c++20 -O2 -Wall -Wextra -pedantic impl.cpp -o impl` then `./impl`.
Do the steps in order. The tests at the bottom fail until each TODO is filled in.

**Step 1: `swap_prices(std::int64_t* a, std::int64_t* b)`.** Swap the two values through the pointers.
<details><summary>Hint</summary>Use a temporary: `auto t = *a; *a = *b; *b = t;`. Swapping `a` and `b` themselves (the pointers) changes nothing for the caller. Why not?</details>

**Step 2: `find_first_above(first, last, limit)`.** Walk from `first` up to (not including) `last` with `++`, and return a pointer to the first price `> limit`. If there's none, return `last`. **No indexes**: only pointers.
<details><summary>Hint</summary>`for (; first != last; ++first) if (*first > limit) return first; return last;`. An empty range (`first == last`) must return `last` too.</details>

**Step 3: `next_tick(const std::int64_t** cursor, const std::int64_t* end, std::int64_t* out)`.** If `*cursor == end`, return `false` and touch nothing. Otherwise write the current value into `*out`, move the caller's cursor forward by one and return `true`.
<details><summary>Hint</summary>`*cursor` is the caller's pointer. `**cursor` is the price it points at. Move it with `++(*cursor)`. Watch out: `*cursor++` moves the wrong thing!</details>

**Step 4: jagged grid.** `make_jagged(rows, cols)` returns a `double**`: allocate the spine (`new double*[rows]`), then each row (`new double[cols]{}`). `free_jagged(grid, rows)` frees each row with `delete[]`, then the spine.

**Step 5: `Matrix` (contiguous).** One `new double[rows * cols]{}` in the constructor and one `delete[]` in the destructor. Implement `get`, `set`, `row(r)` (pointer to the start of row `r`), `data()`, `rows()` and `cols()`.
Copying is `= delete`d for now. Copying a raw-pointer owner correctly is the Rule of 3/5 (Day 22).
<details><summary>Hint</summary>`data_[r * cols_ + c]` for get/set, and `data_ + r * cols_` for `row(r)`. The test `m.row(2) - m.row(1) == 4` checks that the rows are back to back.</details>

**Step 6: `row_sum(const double* row, std::size_t n)` and `col_sum(const Matrix* m, std::size_t c)`.** For `row_sum`, walk with a pointer. For `col_sum`, jump by `cols()` each time: that's a **stride**.

### Part B: break it on purpose (write what happened at the bottom of `impl.cpp`)
1. In `row_sum`, try `*row = 0;` (the parameter is `const double*`). What does the error say?
2. Change `delete[] data_;` to `delete data_;`. Does it compile? Does it warn? Why is it still wrong?
3. Call `send(NULL)` from §1.2 with your g++. Copy the error message.

---

## 4. Output Questions
Predict first, then run `exercises.cpp`.

**Q1**
```cpp
std::int64_t a[] = {10, 20, 30, 40};
std::int64_t* p = a;
p += 2;
std::cout << *p << ' ' << p[-1] << ' ' << (p - a) << '\n';
```
<details><summary>Answer</summary>`30 20 2`. `p` points at `a[2]`. `p[-1]` is `*(p - 1)` = `a[1]`, which is legal because it's still inside the array. `p - a` counts elements, not bytes.</details>

**Q2**
```cpp
void show(std::int64_t arr[]) { std::cout << sizeof(arr) << ' '; }
std::int64_t a[4]{};
std::cout << sizeof(a) << ' ';
show(a);
```
<details><summary>Answer</summary>`32 8`. Inside `show`, `arr` is really a `std::int64_t*`, so you get the size of a pointer. The array size is lost (the array "decayed").</details>

**Q3**
```cpp
int x = 1, y = 2;
int* p = &x;
int** pp = &p;
*pp = &y;      // changes p!
**pp = 5;
std::cout << x << ' ' << y << ' ' << *p << '\n';
```
<details><summary>Answer</summary>`1 5 5`. `*pp = &y` makes `p` point at `y`. Then `**pp = 5` writes into `y`. `x` is never touched.</details>

**Q4**
```cpp
std::int64_t px[3]{};
std::int64_t* p = px;
auto bytes = reinterpret_cast<char*>(p + 1) - reinterpret_cast<char*>(p);
std::cout << bytes << '\n';
```
<details><summary>Answer</summary>`8`. `p + 1` moved one element, which is 8 bytes. Looking at the same move through `char*` (1-byte steps) shows the real distance.</details>

### Will it compile? (`-DC1` … `-DC5`)
```cpp
int x = 1, y = 2;
const int* a = &x;   int* const b = &x;
/*C1*/ *a = 5;
/*C2*/ b = &y;
/*C3*/ a = &y;
/*C4*/ int* c = a;
/*C5*/ double g[3][4]{};  double** pg = g;
```
<details><summary>Answers</summary>

- C1 ❌: `a` points to **const** int.
- C2 ❌: `b` is a **const pointer**.
- C3 ✅: the pointer itself isn't const.
- C4 ❌: that would drop the `const` (a promise can't be quietly removed).
- C5 ❌: `g` decays to `double (*)[4]`, not `double**` (§2.3, "Going deeper").
</details>

---

## 5. Debugging
Each snippet is in `exercises.cpp` behind `-DBUG1`…`-DBUG3`. Find the bug, explain it, fix it.

**BUG1: wild pointer**
```cpp
std::int64_t* last_price;     // never initialized
*last_price = 295035;
```
<details><summary>Answer</summary>`last_price` holds a random address, so writing there is UB: a crash if you're lucky, silent memory corruption if not. Fix: `std::int64_t* last_price = nullptr;`, then point it at real storage before writing. `-Wall` warns "is used uninitialized".</details>

**BUG2: wrong delete**
```cpp
double* prices = new double[100]{};
// ...
delete prices;
```
<details><summary>Answer</summary>`new[]` must be paired with `delete[]`. Using plain `delete` is UB. With types that have destructors it typically crashes or leaks. g++ warns with `-Wmismatched-new-delete`. Fix: `delete[] prices;`.</details>

**BUG3: off by one**
```cpp
std::int64_t qty[4] = {1, 2, 3, 4};
std::int64_t sum = 0;
for (std::int64_t* p = qty; p <= qty + 4; ++p) sum += *p;
```
<details><summary>Answer</summary>`<=` also reads `*(qty + 4)`, the one-past-end slot. Making that pointer is fine, but reading it is UB. Fix: `p != qty + 4` (or `<`). The `[first, last)` habit stops this whole class of bug.</details>

---

## 6. Performance (the HFT angle)

| Operation | Rough cost |
|---|---|
| Pointer math (`p + i`) | ~free (folded into the load instruction) |
| Load that hits L1 cache | ~1 ns (about 4 cycles) |
| Load that misses all caches (goes to RAM) | ~60–100 ns |
| `new` / `delete` (a heap call) | tens of ns, and sometimes far more (it can take a lock or ask the OS) |

What this means:
- **Pointer chasing is slow.** In a jagged grid every row is a separate jump to an unknown place. The CPU's **prefetcher** (hardware that guesses the next memory you'll need and loads it early) can't guess a jump, but it *can* guess "the next 64 bytes".
- **Contiguous + walk in order = fast.** A 64-byte **cache line** holds 8 doubles. Walking a row of a contiguous `Matrix` uses all 8. Walking down a **column** jumps `cols * 8` bytes per step, so each load uses 1 of the 8 and wastes the other 7.
- **Allocate once, up front.** One `new` for the whole grid at startup, then zero allocations on the hot path. HFT systems allocate before the market opens and never during trading.
- **Choose the layout from the access pattern.** If your strategy scans one instrument across time, make rows = instruments. If it scans all instruments at one moment (a cross-section), make rows = time.

---

## 7. Interview Questions
<details><summary>1. What is a pointer, and what's its size on x86-64?</summary>A variable that holds a memory address. 8 bytes, whatever type it points to.</details>
<details><summary>2. Why prefer `nullptr` over `NULL`?</summary>`NULL` is an integer constant, so it can pick an `int` overload or be ambiguous. `nullptr` has its own type and only converts to pointers.</details>
<details><summary>3. `const int*` vs `int* const` vs `const int* const`?</summary>Pointer to const (can't change the value), const pointer (can't move the pointer), both. Read them right to left.</details>
<details><summary>4. If `p` is `std::int64_t*`, how many bytes does `p + 1` move?</summary>8, which is `sizeof(std::int64_t)`. Arithmetic is in elements.</details>
<details><summary>5. Is `a[i]` the same as `*(a + i)`?</summary>Yes, by definition. That's even why `i[a]` compiles (it's `*(i + a)`). Never write it, but interviewers love it.</details>
<details><summary>6. What is array decay?</summary>An array used in most expressions (like passing it to a function) turns into a pointer to its first element, so its size is lost. That's why you pass `(ptr, n)`.</details>
<details><summary>7. When is pointer arithmetic UB even without reading?</summary>When the result goes outside `[array start, one past the end]`, or when you subtract pointers into different arrays.</details>
<details><summary>8. When would you use `T**`?</summary>To let a function change the caller's pointer (an out-parameter or a cursor), for arrays of pointers (jagged arrays, `argv`), and for C APIs that hand you back an allocated pointer.</details>
<details><summary>9. `delete` vs `delete[]`?</summary>They must match `new` and `new[]`. `delete[]` runs every element's destructor and frees the array block. Mixing them up is UB.</details>
<details><summary>10. Jagged `double**` or one contiguous block: which is faster to scan, and why?</summary>Contiguous: one allocation, one load per element, and sequential addresses the prefetcher can predict. Jagged needs an extra load per row, and the rows are scattered.</details>
<details><summary>11. Is `int* p = 0;` legal?</summary>Yes, a literal `0` is a null pointer constant. But `nullptr` says what you mean.</details>
<details><summary>12. What type does `p - q` have?</summary>`std::ptrdiff_t`, a signed integer that counts elements. It's only valid for pointers into the same array.</details>

---

## 8. Hard Questions (optional for now: come back after Day 15)
<details><summary>1. Why is `int* p = a + 10;` UB for a 4-element array, even if you never dereference `p`?</summary>The language only defines pointers inside an array or one past its end. Compilers <b>use</b> that rule when they optimize: they may assume `p + n > p` is always true and delete your "overflow" check. So code that looks harmless can be removed or changed. UB isn't only "it crashes"; it's "the optimizer believed something false".</details>
<details><summary>2. You store 1,000 instruments × 390 one-minute bars. One strategy scans per instrument, another scans per minute. How do you lay out memory?</summary>Row-major `[instrument][minute]` makes the per-instrument scan contiguous and the per-minute scan strided (390 × 8 bytes per step, about one cache miss per element). Options: keep two copies (one transposed), pick the layout for the hottest path, or process the strided scan in blocks (tiling). Always measure: at 3 MB the whole thing may fit in L2/L3 anyway.</details>
<details><summary>3. In `void add(int* a, const int* b) { *a += *b; *a += *b; }`, why might the compiler load `*b` twice?</summary>`a` and `b` might point at the same int ("aliasing"). Then the first write changes `*b`, so the compiler can't keep `*b` in a register. `__restrict` (a compiler extension) promises they don't overlap. Day 08 covers aliasing rules properly.</details>

---

## 9. Mini Project: price grid layout benchmark
In your backtester (or a new `grid_bench.cpp`):
1. Use your `Matrix` with rows = 2,000 instruments and cols = 2,000 bars, filled with any numbers.
2. Time (with `std::chrono::steady_clock`) the sum of every element:
   - row by row (`row_sum` on each row),
   - column by column (`col_sum` on each column),
   - the same row-by-row sum over a `make_jagged` grid.
3. Build with `-O2`. Run each 5 times and write down the best time.
4. Write 2 lines: which was fastest, by how much, and why (cache lines, prefetcher, extra loads).

Expected: column-by-column is clearly the slowest. Jagged row-by-row is close to contiguous here, because each row is still contiguous. The jagged cost shows up more with many short rows.

---

## Checklist
- [ ] Watched the video(s) and read the theory
- [ ] `impl.cpp` passes all tests (`All Day 06 tests passed`)
- [ ] Part B notes written
- [ ] `exercises.cpp`: Q1–Q4, C1–C5, BUG1–BUG3 done
- [ ] Mini project numbers written down
