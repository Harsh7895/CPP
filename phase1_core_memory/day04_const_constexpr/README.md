# Day 04 — `const`, `constexpr`, `consteval`, `constinit`, `volatile`, `mutable`
> Phase 1 · Est. time 60–90 min · Builds on: Day 03 (static storage, globals)

## 🎯 Today in plain words
All six keywords answer one of two questions:
1. **"Can this value change?"** (`const`, `mutable`, `volatile`)
2. **"When is this value calculated: while compiling, or while the program runs?"** (`constexpr`, `consteval`, `constinit`)

Why a trader cares: anything the **compiler** calculates costs **zero time** when the market is open. Lookup tables, unit conversions and limits can all be ready before the program even starts. And `const` stops you, or a teammate, from changing a risk limit by accident.

Files: `impl.cpp` (you implement it, step by step) and `exercises.cpp` (predict-the-output, compile or not, bugs).

---

## 📺 Watch first

| # | Video | Length | What to watch for |
|---|-------|--------|-------------------|
| 1 | **The Cherno: "CONST in C++"** and **"The Mutable Keyword in C++"**. Search YouTube for those exact titles (part of his C++ playlist) | short, about 10–15 min each | The easiest start: `const` variables, const member functions, and `mutable`. Covers §1.1–1.3 and §1.7. |
| 2 | [Back to Basics: static, inline, const, and constexpr](https://youtu.be/hLakx0KYiR0), Andreas Fertig, CppCon 2025 | conference talk, about 1 hr | `const`, `constexpr`, `consteval`, `constinit`, plus `static`/`inline`, which is also a **great review of Days 02–03**. If time is short, watch the const/constexpr parts today and the rest at the weekend. |
| 3 | *(optional)* [Back to Basics: Const as a Promise](https://www.youtube.com/watch?v=NZtr93iL3R0), Dan Saks, CppCon 2019 | about 1 hr | The deepest look at what `const` really promises, especially with member functions. |

**Not covered well by the videos:** `volatile` (§1.8) and the HFT/performance angle (§6). Read those parts of this README carefully.

---

## 0. Warm-up (Day 03)
1. A normal local variable vs a `static` local variable: what's the difference?
2. What does putting a variable inside `namespace { ... }` (no name) do?
3. Each thread calls a function that does `thread_local int n = 0; return ++n;` three times. What does each thread see?

<details><summary>Answers</summary>

1. A normal local is created and destroyed **every call**. A `static` local is created **once** (on the first call) and **keeps its value** between calls.
2. It makes the variable **private to that .cpp file** (internal linkage). Other files can't see it, even if they use the same name.
3. Each thread sees **1, 2, 3**. Every thread has its **own** copy of `n`.
</details>

---

## 1. Theory

### 1.1 `const`: "read-only after creation"
**In one line:** a `const` variable gets a value once and can never be changed afterwards.

```cpp
const int max_qty = 1000;   // must be given a value right away
max_qty = 2000;             // ❌ compile error: max_qty is read-only
const int lot;              // ❌ compile error: a const must be initialized
```
**Analogy:** a "DO NOT TOUCH" sticker. The compiler is the security guard.

**Why care:** it documents your intent ("this is a risk limit, nobody changes it") and the compiler enforces it for free.

### 1.2 `const` references: "look, don't touch"
**In one line:** `const T&` lets a function **read** a big object **without copying it**, and promises not to change it.

```cpp
// ❌ makes a full copy of the whole vector on EVERY call (slow)
long long sum(std::vector<long long> prices);

// ✅ no copy (&), and the function can't change the caller's data (const)
long long sum(const std::vector<long long>& prices);
```
A rule of thumb for parameters:
| Type | Pass it as |
|------|------------|
| small (`int`, `double`, `char`, a pointer) | by value: `int qty` |
| big (`std::string`, `std::vector`, your own structs) that you only read | `const T&` |
| something the function must change | `T&` |

⚠️ `const` only means "**I** won't change it **through this name**". Someone else can still change it (see Q1):
```cpp
int price = 100;
const int& ref = price;   // a read-only view of price
price = 105;              // allowed: we change price directly
std::cout << ref;         // 105: ref sees the new value
```

### 1.3 `const` member functions
**In one line:** writing `const` after a member function means "this function will not change the object".

```cpp
class Instrument {
public:
    long long tick_size() const { return tick_; }  // ✅ only reads
    void set_tick(long long t) { tick_ = t; }      // changes the object, so NOT const
private:
    long long tick_ = 5;
};

const Instrument es;   // a const object...
es.tick_size();        // ✅ ...can call const functions
es.set_tick(10);       // ❌ compile error: can't change a const object
```
**Why care:** when you pass an object as `const Instrument&` (§1.2), you can only call its `const` functions. So **mark every function that doesn't change the object as `const`**. Forgetting it is one of the most common beginner compile errors.

### 1.4 `constexpr`: "can be calculated by the compiler"
**In one line:** `constexpr` means the value or function **can** be computed **while compiling**.

**`constexpr` variable:** it **must** be computed at compile time (and it is also `const`).
```cpp
constexpr int kMaxLevels = 10;          // ✅ known while compiling
constexpr int kTotal = kMaxLevels * 2;  // ✅ 20, calculated by the compiler

int n = 5;
constexpr int bad = n * 2;              // ❌ n is a normal variable, unknown until the program runs
```

**`constexpr` function:** it **can** run at compile time, **if** you give it values the compiler knows. Otherwise it simply runs at run time like a normal function.
```cpp
constexpr int square(int x) { return x * x; }

constexpr int a = square(5);   // compiler calculates 25 → no work at run time
int n = 7;
int b = square(n);             // n is unknown at compile time → calculated when the program runs
```
**Analogy:** a constexpr function is a recipe that can be cooked **in advance** (at compile time) **or** **to order** (at run time), whichever fits.

**`static_assert`** is a check the **compiler** performs. If the condition is false, the program won't compile:
```cpp
static_assert(square(4) == 16);   // checked while compiling, costs nothing at run time
```

### 1.5 `consteval`: "must be calculated by the compiler" (C++20)
**In one line:** a `consteval` function is **only** allowed to run at compile time. Calling it with a run-time value is an error.
```cpp
consteval int twice(int x) { return 2 * x; }

int a = twice(4);    // ✅ the compiler computes 8
int n = 4;
int b = twice(n);    // ❌ compile error: n isn't known at compile time
```
**Why care:** use it when you want a **guarantee** that some work never happens while the program runs (for example, building a hash of a constant string).

### 1.6 `constinit`: "start with a compile-time value, but you can change it later" (C++20)
**In one line:** `constinit` makes a global or static variable get its **first value at compile time**, but it's **not** const.
```cpp
constexpr int default_limit() { return 500; }
constinit int max_orders = default_limit();   // ✅ 500, set while compiling
max_orders = 1000;                            // ✅ allowed: constinit is NOT const

int read_from_file();                         // a normal function
constinit int bad = read_from_file();         // ❌ compile error: not a compile-time value
```
**Why care:** globals that are initialized at **run time** can be created in the wrong order across files (Day 14 covers this). `constinit` guarantees "this one is ready before anything runs".

### 1.7 `mutable`: "allowed to change even inside a const object"
**In one line:** a `mutable` member can be changed inside `const` member functions.
```cpp
class Quote {
public:
    int bid() const {
        ++reads_;        // ✅ allowed because reads_ is mutable
        return bid_;
    }
private:
    int bid_ = 100;
    mutable int reads_ = 0;   // a counter, not part of the quote's real "value"
};
```
**When it's OK:** for things that don't change what the object **means**: counters, caches, and a mutex (Day 57).
**When it's wrong:** using it to sneak around `const` and change real data.

### 1.8 `volatile`: "the compiler must really read or write memory every time"
**In one line:** `volatile` stops the compiler from optimizing away reads and writes, because something **outside the program** (hardware) might change the value.
```cpp
volatile int* status_register = /* address of a hardware device */;
while (*status_register == 0) { }   // read memory EVERY loop, never assume it stays 0
```
**Use it for:** hardware registers and memory-mapped devices (embedded systems).
**❌ NOT for threads.** `volatile` does **not** make code thread-safe. For sharing data between threads, use `std::atomic` (Day 64). That's a classic interview trap.

### 1.9 Summary table

| Keyword | When is the value set? | Can it change later? | Typical use |
|---------|-----------------------|----------------------|-------------|
| `const` | compile time or run time | ❌ | limits, parameters, `const&` |
| `constexpr` variable | compile time (guaranteed) | ❌ | constants, tables |
| `constexpr` function | compile time **or** run time | n/a | math helpers, table builders |
| `consteval` function | compile time **only** | n/a | must-never-cost-runtime work |
| `constinit` | compile time (guaranteed) | ✅ | globals that must be ready at start |
| `mutable` | n/a | ✅ even in const objects | caches, counters, mutexes |
| `volatile` | n/a | changed by "outside" (hardware) | device registers, **not** threads |

---

## 2. Internals: what actually happens

### 2.1 Where compile-time values go
```cpp
constexpr std::array<long long, 19> kPow10 = make_pow10_table();
```
- While compiling, the compiler **runs** `make_pow10_table()` itself, using a built-in C++ interpreter.
- It writes the finished 19 numbers into the program file, in a **read-only** memory section (`.rodata`).
- At run time **no code runs at all** to build the table. The numbers are just there.

You can check it yourself. Make a small file `look.cpp` with your `pow10`, `make_pow10_table` and `kPow10`, plus this function:
```cpp
long long lookup(int d) { return kPow10[d]; }   // d is only known at run time
```
Run `g++ -std=c++20 -O2 -S look.cpp`, then open `look.s`. You'll find the table written out as plain data (`.quad 1`, `.quad 10`, `.quad 100`, …) and **no loop** that builds it.
(If you try this on `impl.cpp` instead, you may find nothing at all: at `-O2` the compiler works out every test in `main()` by itself, so it doesn't even need to keep the table.)

### 2.2 A `constexpr` function at run time is just a normal function
```cpp
int n = read_input();
int b = square(n);   // compiles into an ordinary call (or inlined code), nothing special
```
`constexpr` doesn't make run-time code faster. It only **allows** compile-time use.

> 🔍 **Going deeper (optional)**
> - Inside a compile-time evaluation, the compiler **refuses undefined behaviour**: overflow, out-of-bounds access, and reading uninitialized memory all become compile **errors**. That makes `static_assert` + `constexpr` a free bug detector.
> - There are limits: no `std::cout`, no I/O, and no `std::pow` (the standard doesn't promise it's `constexpr` before C++26. GCC happens to allow some math functions as an extension, but don't rely on that). C++20 allows loops, `if`, local variables, and even `std::vector`/`std::string` *during* evaluation, as long as they're freed before the end.
> - `const int k = 5;` (with a constant value) **can** also be used in compile-time expressions, for historical reasons. `constexpr` makes the intent explicit, so prefer it for constants.

### 2.3 How `const` member functions work
Every member function gets a hidden pointer, `this`, to the object.
- In a normal member function, `this` is `Instrument*` (it can change the object).
- In a `const` member function, `this` is `const Instrument*` (read-only). That's exactly why `++tick_` inside a const function is an error.
- `mutable` members are the one exception the compiler allows through that read-only pointer.

### 2.4 Why `const_cast` on a truly const object is dangerous
```cpp
const int risk_limit = 100;
int* p = const_cast<int*>(&risk_limit);
*p = 200;                                 // ❌ undefined behaviour
std::cout << risk_limit << ' ' << *p;     // on our compiler: "100 200"
```
The compiler **knows** `risk_limit` is 100 forever, so it just prints `100` without reading memory. Meanwhile `*p` reads the memory, which was overwritten with 200. Two different answers for the same variable: this is what undefined behaviour looks like. **Never write to something that was born `const`.**

---

## 3. Implementation (`impl.cpp`)

Work through the steps in order. Each step has its own tests in `main()`.
```
g++ -std=c++20 -O2 -Wall -Wextra -pedantic impl.cpp -o impl && ./impl
```

**Step 1: `pow10(n)`.** Return 10ⁿ for n = 0…18 with a simple loop.
<details><summary>Hint</summary>Start with <code>long long result = 1;</code> and multiply by 10, <code>n</code> times.</details>

**Step 2: `make_pow10_table()`.** Fill `t[i] = pow10(i)` for all 19 entries. `kPow10` is then built **by the compiler**.

**Step 3: `rescale(value, from_dec, to_dec)`.** Change how many decimals a fixed-point price has.
- More decimals: **multiply** by 10^(difference). `rescale(295035, 2, 4) == 29503500`
- Fewer decimals: **divide** (the extra digits are dropped). `rescale(29503567, 4, 2) == 295035`
<details><summary>Hint</summary>The difference is <code>to_dec - from_dec</code>. If it's ≥ 0, use <code>value * kPow10[diff]</code>. Otherwise use <code>value / kPow10[-diff]</code>.</details>

**Step 4: `units_per_one(decimals)`** (`consteval`). Return `kPow10[decimals]`.

**Step 5: `constinit`.** Already written: just read it. You'll break it in Part B.

**Step 6: `Instrument` + `mutable`.** Make `tick_size()` count how many times it's called, and return the count from `lookup_count()`. Add a counter member. The compiler will complain until you use the right keyword. Read the error message: it tells you exactly why.

**Step 7: `sum_prices`.** Add up the vector (taken by `const&`).

**Step 8: the final check.** Uncomment the 4 `static_assert` lines at the bottom. If the file still compiles, your functions really run **at compile time**. 🎉

### Part B: break it on purpose (write what happened at the bottom of `impl.cpp`)
1. Remove `const` from `long long tick_size() const`. Which line in `main()` fails, and why?
2. Remove `mutable` from your counter. Read the error.
3. In `main()`, add `int n = 2; units_per_one(n);`. Why is that an error now, when `units_per_one(2)` was fine?
4. Remove `constexpr` from `default_max_orders_per_sec()`. What does `constinit` complain about?
5. Change `constinit int g_max_orders_per_sec` to `constexpr int ...`. Which line in `main()` breaks, and why?

---

## 4. Output Questions
Predict first, then run `exercises.cpp`.

**Q1**
```cpp
int price = 100;
const int& ref = price;
price = 105;
std::cout << ref;
```
**Q2**
```cpp
constexpr int square(int x) { return x * x; }
constexpr int a = square(5);
int n = 7;
int b = square(n);
std::cout << a << ' ' << b;
```
**Q3**
```cpp
struct Quote { int bid; mutable int reads = 0; int get_bid() const { ++reads; return bid; } };
const Quote q{100};
q.get_bid(); q.get_bid();
std::cout << q.reads;
```
**Q4**
```cpp
constinit int limit = 10;
limit += 5;
std::cout << limit;
```
**Q5**
```cpp
struct Book {
    int best_bid = 99;
    int& get()             { std::cout << "non-const "; return best_bid; }
    const int& get() const { std::cout << "const ";     return best_bid; }
};
Book b; const Book cb;
b.get(); cb.get();
```

<details><summary>Answers</summary>

1. **`105`**. `ref` is a read-only *view* of `price`. We changed `price` directly, so the view shows the new value.
2. **`25 49`**. The same function: `square(5)` ran at compile time and `square(n)` ran at run time.
3. **`2`**. `reads` is `mutable`, so the const function is allowed to increase it, even on a const object.
4. **`15`**. `constinit` only fixes the *starting* value. The variable is not const.
5. **`non-const const`**. C++ picks the `const` version of the function for a `const` object, and the normal version otherwise.
</details>

### Will it compile?
Build with `-DC1`, `-DC2`, … and predict ✅ or ❌ first.
```cpp
// C1
const int max_qty;
// C2
int lot = 5;
constexpr int two_lots = lot * 2;
// C3
consteval int twice(int x) { return 2 * x; }
int n = 4;  return twice(n);
// C4
int read_limit_from_config() { return 500; }
constinit int max_orders = read_limit_from_config();
// C5
struct Account { int cash = 0; void deposit(int x) { cash += x; } };
void c5(const Account& acc) { acc.deposit(100); }
```
<details><summary>Answers</summary>

All five are ❌:
- **C1**: a `const` must get a value when it's created. Fix: `const int max_qty = 1000;`
- **C2**: `lot` is a normal variable, so the compiler doesn't know its value. Fix: make it `constexpr int lot = 5;`
- **C3**: a `consteval` function needs compile-time arguments, and `n` is a normal variable.
- **C4**: `read_limit_from_config()` isn't `constexpr`, so it can only run at run time. Fix: make it `constexpr`, or drop `constinit`.
- **C5**: `acc` is `const&`, and `deposit` isn't a `const` function (it changes `cash`). You can't change a const object.
</details>

---

## 5. Debugging
**B1** (`-DBUG1`): a speed bug. How many copies does it make?
```cpp
int count_orders(std::vector<Order> orders) { return orders.size(); }
// called 10 times with a vector of 1000 orders
```
**B2** (`-DBUG2`): doesn't compile. Add **one word** to fix it.
```cpp
class PriceCache {
public:
    int get() const { if (!ready_) { value_ = 42; ready_ = true; } return value_; }
private:
    int value_ = 0;
    bool ready_ = false;
};
```
**B3** (`-DBUG3`): what do you expect it to print? What does it print?
```cpp
const int risk_limit = 100;
int* p = const_cast<int*>(&risk_limit);
*p = 200;
std::cout << risk_limit << ' ' << *p;
```
**B4**: a teammate's code (just read it)
```cpp
volatile bool g_stop = false;          // thread A sets it to true
void worker() { while (!g_stop) { process_tick(); } }   // thread B
```

<details><summary>Answers</summary>

- **B1. 10,000 copies.** The vector is passed **by value**, so all 1000 orders are copied on each of the 10 calls. Fix: `const std::vector<Order>& orders`. Copies made: **0**.
- **B2.** `get()` is `const` but changes `value_` and `ready_`. Fix: `mutable int value_ = 0; mutable bool ready_ = false;`. It's a cache, so `mutable` is the right tool. (With threads you'd also need a lock: Day 57.)
- **B3.** It prints **`100 200`**: the same variable shows two different values! Writing to an object that was born `const` is **undefined behaviour**. The compiler assumed `risk_limit` is always 100 and printed that directly (§2.4).
- **B4.** `volatile` is **not** for threads. It's a **data race** (undefined behaviour). The CPU or compiler may delay or reorder the write, and there's no guarantee thread B ever sees it in time. Fix: `std::atomic<bool> g_stop{false};` (Day 64).
</details>

---

## 6. Performance (the HFT angle)
| Idea | In simple words |
|------|-----------------|
| **Compile-time work is free at run time** | Every number the compiler works out (tables, conversion factors, limits) costs **0 ns** while trading. Move setup work to compile time whenever you can. |
| **Lookup table vs calculation** | `kPow10[d]` is one memory read from a small table that's almost always in the CPU cache. `std::pow(10, d)` is a slow `double` math function, plus a risky conversion back to an integer. |
| **Fixed-point prices** | Storing prices as integers (295035 instead of 2950.35) avoids `double` rounding bugs (Day 01's `to_ticks` trap), and integer math is fast and exact. Most trading systems do this. |
| **`const&` saves copies** | Passing a big vector by value copies all of it, with memory allocation, on every call (B1). On a hot path, that can be the most expensive thing in the program. |
| **`const` doesn't make code faster by itself** | The compiler usually can't assume a `const&` value never changes, because someone else may change it (Q1). Use `const` for **correctness**. Use `constexpr` when you want the **speed**. |
| **`volatile` makes code slower** | It forces a real memory access every time and blocks optimizations. Never use it "for speed" or "for threads". |
| **`mutable` + threads** | People expect `const` functions to be safe to call from many threads at once. A `mutable` cache or counter breaks that unless you protect it with an atomic or a mutex. |

---

## 7. Interview Questions
<details><summary>1. What's the difference between const and constexpr?</summary><code>const</code> means "can't change after creation", and the value may only be known at run time. A <code>constexpr</code> variable must be known at <b>compile time</b> (and is also const).</details>
<details><summary>2. Can a constexpr function run at run time?</summary>Yes. With compile-time arguments it <i>can</i> run at compile time. With run-time arguments it runs as a normal function.</details>
<details><summary>3. What does consteval add?</summary>It <b>forces</b> compile-time evaluation. Calling it with a run-time value is a compile error.</details>
<details><summary>4. What is constinit and how is it different from constexpr?</summary><code>constinit</code> guarantees a global or static variable is initialized at compile time, but the variable is <b>not</b> const. <code>constexpr</code> is compile time <i>and</i> const.</details>
<details><summary>5. What does a const member function promise?</summary>It won't change the object's members (except <code>mutable</code> ones). It's the only kind of member function you can call on a const object or a <code>const&</code>.</details>
<details><summary>6. When would you use mutable?</summary>For members that aren't part of the object's logical value: caches, counters, and mutexes used inside const functions.</details>
<details><summary>7. Why pass by const reference?</summary>To avoid copying big objects, while promising the function won't modify them.</details>
<details><summary>8. Does volatile make a variable thread-safe?</summary>No. It only stops the compiler from removing reads and writes. It gives no atomicity and no ordering between threads. Use <code>std::atomic</code>.</details>
<details><summary>9. What is volatile actually for?</summary>Memory-mapped hardware registers, and memory that something outside the program can change.</details>
<details><summary>10. Is it OK to const_cast away const and modify the value?</summary>Only if the original object was <b>not</b> declared const. If it was born const, modifying it is undefined behaviour.</details>
<details><summary>11. What is static_assert?</summary>A check done by the compiler. If the condition is false, the program doesn't compile. It costs nothing at run time.</details>
<details><summary>12. Can you overload a member function on const?</summary>Yes. <code>T& get()</code> and <code>const T& get() const</code> can both exist. Const objects call the const one.</details>

---

## 8. Hard Questions (optional for now: come back after Day 15)
<details><summary>H1. Why is `const` alone usually not an optimization, while `constexpr` is?</summary>

A `const int&` only says *you* won't modify the value through that reference. Another reference or pointer to the same memory can still change it, so the compiler must re-read it (this problem is called *aliasing*). `constexpr` gives the compiler the **actual value**, so it can fold it straight into the instructions, remove branches, and precompute tables.
</details>

<details><summary>H2. A const member function uses a mutable cache. Two threads call it on the same const object. What can go wrong, and how do you fix it?</summary>

Both threads may see "not ready" and write the cache at the same time. That's a data race, which is undefined behaviour. The standard library assumes `const` functions are safe to call concurrently. Fixes: protect the cache with a `mutable std::mutex`, use a `std::atomic` flag plus careful ordering, compute the value eagerly in the constructor (often the best for HFT, since no work happens later), or make it `constexpr` if possible.
</details>

<details><summary>H3. Design the price representation for a multi-exchange backtester: some instruments use 2 decimals, others 4 or 8. How do you use today's tools?</summary>

Store prices as `int64_t` fixed-point, with a per-instrument decimal count. Keep a `constexpr` power-of-10 table (today's `kPow10`) for conversions. Use `consteval` helpers for constants in code, like `units_per_one(4)`. Normalize to a common internal precision when comparing across instruments, using `rescale`. Use `static_assert`s to check that the largest price times the largest scale can't overflow `int64_t`. This avoids `double` rounding errors, and every conversion is a multiply or divide by a cached constant.
</details>

---

## 9. Mini Project: a compile-time tick-size table for your backtester
Many exchanges use a **different tick size depending on the price band** (e.g. below 100 → tick 0.01, 100–1000 → tick 0.05, above 1000 → tick 0.10).
1. Write `struct Band { long long upto_price; long long tick; };` (prices in fixed-point with 2 decimals).
2. Make a `constexpr std::array<Band, 3> kBands = {...};`.
3. Write `constexpr long long tick_for(long long price)`, which finds the band with a loop.
4. Write `constexpr long long round_to_tick(long long price)`, which rounds down to a valid tick.
5. Prove it works with **`static_assert`** only (no running needed), e.g. `static_assert(round_to_tick(12347) == 12345);`.
6. Bonus: time 10 million `round_to_tick` calls on random prices vs a version that uses `double` and `std::floor`, and explain the difference.

Then drop it into your backtester wherever you round order prices.

---

## Checklist
- [ ] watched at least video 1 (+ video 2 if there's time)
- [ ] read sections 1–2
- [ ] `impl.cpp`: Steps 1–7 pass, Step 8 `static_assert`s compile
- [ ] Part B: 5 breakages written down
- [ ] exercises: Q1–Q5, C1–C5, B1–B4 predicted before running
- [ ] mini project: tick table proven with `static_assert`
