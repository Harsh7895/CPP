# Day 07 — References (lvalue & rvalue), function pointers, pointer-to-member
> Phase 1 · Est. time 40–45 min (paired with Day 06) · Builds on: Day 06 (pointers), Day 05 (`enum class`, structs)

## 🎯 Today in plain words
Yesterday a pointer was "a sticky note with an address". Today you meet three relatives:
- A **reference** is a second **name** for an object that already exists. It's a pointer you can't move and can't leave empty.
- A **function pointer** holds the address of a **function**, so you can decide at run time which function to call. That's a callback.
- A **pointer-to-member** says "the `price` field" or "the `on_tick` method" of a struct, without naming a particular object yet.

Why a trader cares: an event loop says "when a tick arrives, call these handlers". That's function pointers. Passing an order book by `const&` instead of by value saves a copy on every call. And references are the gateway to move semantics (Day 23–25).

Files: `impl.cpp` (build a callback event dispatcher, step by step) and `exercises.cpp`.

---

## 📺 Watch first

| # | Video | Length | What to watch for |
|---|-------|--------|-------------------|
| 1 | **The Cherno: "REFERENCES in C++"** (episode 17 of his series, right after pointers). Search YouTube for that exact title | short (about 10–15 min) | A reference is an alias, and how it differs from a pointer. Covers §1.1–1.3. |
| 2 | **The Cherno: "Function Pointers in C++"**. Search YouTube for that exact title | short (about 10–15 min) | Function-pointer syntax, `auto`, aliases, passing a function as a callback. Covers §1.5. |
| 3 | *(optional)* [Back to Basics: Functions, Mike Shah, CppCon 2023](https://isocpp.org/blog/2023/09/cppcon-2023-back-to-basics-functions-mike-shah) (the page links the video) | about 1 hr | Functions have addresses in the machine code, function pointers, and member functions. Good for §2.2 and the hard questions. |

**Not covered well by the videos:** rvalue references and which overload wins (§1.4), pointer-to-member syntax (§1.6–1.7), and the cost of an indirect call (§6). Read those carefully.

---

## 0. Warm-up (Day 06)
1. `const std::int64_t* p` vs `std::int64_t* const p`: which one can move, and which one can change the value?
2. In a contiguous `Matrix` with 4 columns, what's the index of `(r=2, c=1)`? Why is scanning a **column** slower than scanning a **row**?

<details><summary>Answers</summary>

1. `const T* p`: the pointer can move, but you can't change `*p`. `T* const p`: the pointer can't move, but `*p` can change.
2. `2 * 4 + 1 = 9`. A row is 8-byte neighbours in one block, so one 64-byte cache line gives you 8 values. A column jumps `cols * 8` bytes each step, so you use 1 value per cache line and waste the rest.
</details>

---

## 1. Theory

### 1.1 lvalue reference (`T&`): a second name
**In one line:** `T& r = x;` makes `r` **another name for `x`**. Anything you do to `r` happens to `x`.

```cpp
std::int32_t position = 10;
std::int32_t& pos = position;   // pos IS position (no copy)
pos += 5;
std::cout << position << '\n';  // 15

std::int32_t other = 99;
pos = other;                    // ⚠️ does NOT re-point pos! It copies 99 INTO position.
std::cout << position << '\n';  // 99
```

**Three rules:**
| Rule | Example |
|---|---|
| Must be initialized | `int& r;` ❌ |
| Can never be re-pointed ("reseated") | `r = other;` assigns a value, it doesn't re-point |
| No "null reference" | there is no `nullptr` for references |

**Analogy:** a nickname. "Harsh" and "H" are the same person. You can't later move the nickname "H" onto someone else.

### 1.2 Reference vs pointer
| | Pointer `T*` | Reference `T&` |
|---|---|---|
| Can be null | yes | no |
| Can be re-pointed | yes | no |
| Must be initialized | no (but do it!) | yes |
| Syntax to use | `*p`, `p->x` | just `r`, `r.x` |
| Pointer arithmetic | yes | no |
| In machine code | an address | usually an address too (§2.1) |

**Rule of thumb:** use a reference when "there is always exactly one object". Use a pointer when "maybe none" (`nullptr`) or "it may change which one".

### 1.3 Passing to functions: value, `&`, `const&`
```cpp
struct Book { std::int64_t bids[512]; std::int64_t asks[512]; };   // 8 KB

void by_value(Book b);           // COPIES 8 KB on every call 😬
void by_ref(Book& b);            // no copy, CAN modify the caller's book
void by_cref(const Book& b);     // no copy, read-only ✅ (the usual choice)
void small(std::int64_t px);     // small types: just pass by value (fits in a register)
```

`const T&` has a superpower: it can bind to a **temporary** (a value with no name, like `5` or `a + b`).
```cpp
const std::int64_t& r = 100 + 1;   // ✅ the temporary lives as long as r ("lifetime extension")
std::int64_t& bad = 100 + 1;       // ❌ a plain & can't bind to a temporary
```
⚠️ Lifetime extension only works when the reference is bound **directly** to the temporary. A reference *returned from a function* doesn't extend anything (see BUG2). Full details are on Day 13.

### 1.4 rvalue reference (`T&&`): "a reference to something that's about to die"
**In one line:** `T&&` binds **only to temporaries** (rvalues), so a function knows "nobody else needs this, I may steal its insides".

First, two words:
- **lvalue** = has a name and an address, and you can take `&x` of it. Example: `x`, `book.bids[3]`.
- **rvalue** = a temporary with no name. Example: `5`, `a + b`, `make_order()`.

```cpp
void f(int&)        { std::cout << "L\n"; }   // takes named, non-const things
void f(const int&)  { std::cout << "C\n"; }   // takes anything (read-only)
void f(int&&)       { std::cout << "R\n"; }   // takes temporaries

int x = 1;
const int y = 2;
f(x);             // L
f(y);             // C
f(42);            // R
f(std::move(x));  // R: std::move doesn't move anything, it just CASTS x to an rvalue
int&& z = 7;
f(z);             // L ❗ z has a NAME, so z itself is an lvalue
```

**The ❗ rule:** "if it has a name, it's an lvalue", even when its type is `int&&`. This one rule explains most move-semantics confusion. We'll use it on Day 23–25. Today, just know what binds where:

| Argument → | `T&` | `const T&` | `T&&` |
|---|---|---|---|
| named non-const (`x`) | ✅ best | ✅ | ❌ |
| named const (`y`) | ❌ | ✅ | ❌ |
| temporary (`42`, `std::move(x)`) | ❌ | ✅ | ✅ best |

### 1.5 Function pointers: "which function to call" as a value
**In one line:** a function lives at an address in the program's code. A function pointer stores that address, so you can call it later.

```cpp
int add(int a, int b) { return a + b; }
int sub(int a, int b) { return a - b; }

int (*op)(int, int) = add;        // raw syntax: the parentheses around *op are needed
std::cout << op(5, 3) << '\n';    // 8
op = sub;
std::cout << op(5, 3) << '\n';    // 2

using BinOp = int (*)(int, int);  // much easier to read: give the type a name
BinOp table[] = {add, sub};       // an array of functions = a "dispatch table"
std::cout << table[1](10, 4) << '\n';  // 6
```

**The C callback pattern:** a plain function pointer can't remember any state. So C APIs pass a `void*` "context" next to it (`void*` = "a pointer to anything, type unknown"):
```cpp
using Handler = void (*)(const Event& e, void* ctx);

void count_ticks(const Event&, void* ctx) {
    ++*static_cast<int*>(ctx);   // we KNOW ctx is really an int*, so cast it back
}
```
You build exactly this in `impl.cpp`. Lambdas and `std::function` (later in the plan) are the modern versions, but they compile down to the same idea.

### 1.6 Pointer to data member: "the `price` field of *some* Order"
**In one line:** `Price Order::* f = &Order::price;` names a field without an object. You pick the object later with `obj.*f` or `ptr->*f`.

```cpp
struct Fill { std::int64_t px; std::int32_t qty; std::int32_t fee; };

std::int32_t Fill::* field = &Fill::qty;   // "the qty field"
Fill f{295035, 10, 2};
std::cout << f.*field << '\n';             // 10
field = &Fill::fee;
std::cout << f.*field << '\n';             // 2
```

**Why care:** you can write **one** function, `sum_field(fills, n, &Fill::qty)`, that works for any field. No copy-pasted loops.

### 1.7 Pointer to member function
**In one line:** the same idea for methods. `void (Strategy::*fn)(const Event&) = &Strategy::on_tick;`, then call it with `(s.*fn)(e)`.

```cpp
struct Strategy {
    void on_tick(const Event&) { std::cout << "tick\n"; }
    void on_fill(const Event&) { std::cout << "fill\n"; }
};
using StratFn = void (Strategy::*)(const Event&);

StratFn fn = &Strategy::on_fill;   // & is REQUIRED for member functions
Strategy s;
(s.*fn)(ev);                       // prints "fill". The outer parentheses are REQUIRED:
// s.*fn(ev);                      // ❌ parsed as s.*(fn(ev))
```

> 🔍 **Going deeper (optional):** `std::invoke(fn, s, ev)` (from `<functional>`) calls any of these the same way: function pointers, member pointers and lambdas. The standard library uses it everywhere.

### 1.8 Summary table
| Thing | Declare | Use |
|---|---|---|
| lvalue ref | `T& r = x;` | `r` |
| const ref | `const T& r = anything;` | `r` (read-only) |
| rvalue ref | `T&& r = temp;` | `r` (but `r` itself is an lvalue!) |
| function ptr | `using F = R(*)(Args...);` | `f(args)` |
| ptr to data member | `T C::* m = &C::field;` | `obj.*m`, `ptr->*m` |
| ptr to member fn | `using M = R (C::*)(Args...);` | `(obj.*m)(args)`, `(ptr->*m)(args)` |

---

## 2. Internals: what actually happens

### 2.1 A reference is (usually) a pointer in disguise
```cpp
void bump(std::int32_t& q) { q += 1; }
void bump(std::int32_t* q) { *q += 1; }
```
Both compile to the **same** machine code: the address arrives in a register (`rcx` on Windows, `rdi` on Linux), and the CPU adds 1 at that address. A reference costs **nothing extra** compared with a pointer. It's a compile-time rule ("never null, never reseated"), not a different machine thing.

Inside a function the compiler often removes the reference completely (it just uses the original variable). Inside a **struct**, though, a reference member takes real space:
```cpp
struct Holder { char& c; };   // sizeof(Holder) == 8 on 64-bit: it stores an address
```

### 2.2 Direct call vs indirect call
```asm
call add            ; DIRECT: the target is written in the instruction
call rax            ; INDIRECT: the target is whatever address is in rax right now
```
- A **direct** call can be **inlined**: the compiler copies the function body into the caller and optimizes across it.
- An **indirect** call (through a function pointer) usually can't be inlined, because the compiler doesn't know the target. The CPU must **predict** the target. If the prediction is wrong, you pay a pipeline flush (about 15–20 cycles).

### 2.3 What pointers-to-member look like inside
| Kind | Size on your g++ (x86-64) | What it stores |
|---|---|---|
| `T*`, function pointer | 8 bytes | an address |
| `std::int32_t Fill::*` (data member) | 8 bytes | the field's **offset** in bytes (`qty` → 8). Null is stored as `-1`, because offset 0 is a real field |
| `void (Strategy::*)(const Event&)` (member fn) | **16 bytes** | a function address (or a vtable slot, if the method is virtual) **plus** an adjustment for `this` (needed with multiple inheritance, Day 19) |

```
Fill in memory:           offset 0      offset 8   offset 12
                          ┌─────────────┬──────────┬──────────┐
                          │ px (8 B)    │ qty (4B) │ fee (4B) │
                          └─────────────┴──────────┴──────────┘
&Fill::qty  ==  "8"   →   f.*field  ==  *(int32_t*)((char*)&f + 8)
```
These sizes follow the "Itanium C++ ABI" that g++ and clang use. MSVC uses different sizes. Don't rely on them in portable code.

---

## 3. Implementation (`impl.cpp`)

Build: `g++ -std=c++20 -O2 -Wall -Wextra -pedantic impl.cpp -o impl` then `./impl`.

**Step 1: references.** `add_fill(Qty& position, Qty fill_qty)` adds the fill to the caller's position. `better_bid(const Price& a, const Price& b)` returns a reference to the **higher** price.
<details><summary>Hint</summary>`return a >= b ? a : b;`. The tests check that it returns a reference to the <b>original</b> variable (`&better_bid(x, y) == &y`), not to a copy.</details>

**Step 2: which overload wins?** Make `kind(int&)` return 1, `kind(const int&)` return 2 and `kind(int&&)` return 3. Then **predict every test line before you run it**. One of them surprises most people (§1.4 ❗).

**Step 3: `Dispatcher`.** This is a callback event dispatcher with **no heap allocation**: a fixed 2D array of slots, `[event type][handler]`.
- `subscribe(type, fn, ctx)`: store `{fn, ctx}` in the next free slot for that type. Return `false` if `fn == nullptr` or that type already has `kMaxPerType` handlers.
- `dispatch(e)`: call every handler subscribed to `e.type`, **in subscribe order**, as `fn(e, ctx)`. Return how many you called.
- `count(type)`: how many handlers that type has.
<details><summary>Hint</summary>The row index is `static_cast<std::size_t>(t)` (Day 05: `enum class` needs an explicit cast). `used_[i]` is the next free column of row `i`.</details>

**Step 4: `sum_field(const Fill* fills, std::size_t n, Qty Fill::* field)`.** Sum one chosen field over an array of fills. Use `fills[i].*field` (or `p->*field` while walking a pointer).

**Step 5: route through a member-function table.** Fill `kStratTable` so that index `Tick` → `&Strategy::on_tick`, `Fill` → `&Strategy::on_fill`, `Cancel` → `&Strategy::on_cancel`. Then `route(s, e)` looks up the method and calls `(s.*fn)(e)`. If the slot is `nullptr`, do nothing.

### Part B: break it on purpose (notes at the bottom of `impl.cpp`)
1. Remove the parentheses: `s.*fn(e)`. Copy the error.
2. Write `int& r;` inside `main`. Copy the error.
3. Add `static_assert(sizeof(StratFn) == 8);`. What's the real size, and why (§2.3)?

---

## 4. Output Questions

**Q1**
```cpp
int a = 1;
int& r = a;
r = 5;
int b = 9;
r = b;
b = 10;
std::cout << a << ' ' << r << ' ' << b << '\n';
```
<details><summary>Answer</summary>`9 9 10`. `r = b` copies 9 into `a` (a reference can't be re-pointed). Changing `b` afterwards doesn't affect `a`.</details>

**Q2**
```cpp
void f(int&) { std::cout << 'L'; }  void f(const int&) { std::cout << 'C'; }  void f(int&&) { std::cout << 'R'; }
int x = 1; const int y = 2; int&& z = 3;
f(x); f(y); f(4); f(z); f(std::move(x));
```
<details><summary>Answer</summary>`LCRLR`. `f(z)` is `L` because `z` has a name, so it's an lvalue, even though its type is `int&&`.</details>

**Q3**
```cpp
int add(int a, int b) { return a + b; }
int sub(int a, int b) { return a - b; }
int mul(int a, int b) { return a * b; }
using Op = int (*)(int, int);
Op ops[] = {add, sub, mul};
int acc = 10;
for (Op op : ops) acc = op(acc, 2);
std::cout << acc << '\n';
```
<details><summary>Answer</summary>`20`, because ((10 + 2) − 2) × 2 = 20. The table is run in order.</details>

**Q4**
```cpp
char c = 'x';
char& rc = c;
struct Holder { char& r; };
std::cout << sizeof(rc) << ' ' << sizeof(Holder) << '\n';
```
<details><summary>Answer</summary>`1 8`. `sizeof` on a reference gives the size of the object it refers to (a `char`). A reference stored <b>inside a struct</b> is kept as an address: 8 bytes.</details>

### Will it compile? (`-DC1` … `-DC4`)
```cpp
int x = 1;
/*C1*/ int& r1;
/*C2*/ int& r2 = 5;
/*C3*/ const int& r3 = 5;
/*C4*/ int&& r4 = x;
```
<details><summary>Answers</summary>

- C1 ❌: a reference must be initialized.
- C2 ❌: a plain `&` can't bind to a temporary.
- C3 ✅: `const&` can, and the temporary's lifetime is extended.
- C4 ❌: `&&` binds only to rvalues, and `x` is an lvalue. `std::move(x)` would compile.
</details>

---

## 5. Debugging (`-DBUG1` … `-DBUG3`)

**BUG1: returning a reference to a local**
```cpp
const Price& mid(const Price& bid, const Price& ask) {
    Price m = (bid + ask) / 2;
    return m;
}
```
<details><summary>Answer</summary>`m` dies when the function returns, so the caller gets a <b>dangling reference</b> (it refers to dead stack memory). g++ warns with `-Wreturn-local-addr`. Fix: return by value, `Price mid(...)`. Returning an 8-byte integer by value costs nothing.</details>

**BUG2: a dangling reference that looks safe**
```cpp
const Price& best = better_bid(295000, 295035);   // two temporaries
std::cout << best << '\n';
```
<details><summary>Answer</summary>The two temporaries die at the end of the first line (the end of the "full expression"). `best` then refers to a dead temporary. Lifetime extension does <b>not</b> apply, because `best` binds to the function's return value, not directly to the temporary. Recent g++ versions may warn (`-Wdangling-reference`). `std::max` has the same trap. Fix: `Price best = better_bid(...);` (a copy).</details>

**BUG3: member-function-pointer precedence**
```cpp
StratFn fn = &Strategy::on_tick;
s.*fn(ev);
```
<details><summary>Answer</summary>This doesn't compile. A function call binds more tightly than `.*`, so it's parsed as `s.*(fn(ev))`. Fix: `(s.*fn)(ev);`.</details>

---

## 6. Performance (the HFT angle)
- **References and pointers cost the same.** Choose between them for clarity and safety, not for speed.
- **Pass small types by value.** `Price`, `Qty`, `Side` and a 16-byte `{px, qty}` pair travel in registers. Passing them by `const&` forces them into memory so their address can be taken, and adds a load.
- **Pass big types by `const&`.** An 8 KB `Book` copied by value at a million events a second is gigabytes of copying.
- **Indirect calls:** about free when the target is predictable (the same handler every time). A mispredict costs about 15–20 cycles (around 5 ns). The bigger cost is often the **lost inlining**: the compiler can't optimize across the call.
- **What HFT code does:** set up callbacks **at startup**, keep them in **fixed arrays** (no `std::vector` growth, no heap), and use **compile-time dispatch** (templates/CRTP, Day 29) on the very hottest path.
- **`std::function`** (later) can allocate memory when it holds a large callable. A raw function pointer plus `void*` never allocates.

---

## 7. Interview Questions
<details><summary>1. Reference vs pointer: what are the differences?</summary>A reference must be initialized, can't be null and can't be reseated, and has no pointer arithmetic. It's an alias. A pointer is an object holding an address.</details>
<details><summary>2. Can you have a null reference?</summary>Not in a valid program. `*(int*)nullptr` bound to a reference is already UB.</details>
<details><summary>3. When pass by `const T&` and when by value?</summary>`const T&` for big or expensive-to-copy types. By value for small, cheap types (ints, prices, small structs), and when you need your own copy anyway.</details>
<details><summary>4. What can a `const T&` bind to?</summary>Anything: lvalues, const lvalues and temporaries. When it binds directly to a temporary, it extends that temporary's lifetime.</details>
<details><summary>5. What does `T&&` bind to?</summary>Only rvalues: temporaries and the result of `std::move`.</details>
<details><summary>6. Is a named variable of type `int&&` an lvalue or an rvalue?</summary>An lvalue: it has a name. That's why you need `std::move` again to pass it on as an rvalue.</details>
<details><summary>7. What does `std::move` actually do?</summary>Nothing at run time. It's a cast to `T&&`, and the moving happens in whatever function takes that `&&`.</details>
<details><summary>8. Write the type of a pointer to a function taking `(int, int)` and returning `int`.</summary>`int (*)(int, int)`, or `using Op = int (*)(int, int);`.</details>
<details><summary>9. Can a call through a function pointer be inlined?</summary>Only if the compiler can prove the target (for example, the pointer is a known constant). Usually it can't, which is the real cost.</details>
<details><summary>10. What is a pointer to a data member, physically?</summary>The byte offset of the field inside the object. On g++/clang a null one is stored as -1.</details>
<details><summary>11. Why is a pointer to member function 16 bytes on g++?</summary>It stores the function address (or the vtable offset for virtual functions) <b>plus</b> a `this` adjustment for multiple inheritance.</details>
<details><summary>12. Why do C callback APIs take a `void* ctx`?</summary>A plain function pointer carries no state. The context pointer lets the callback reach its object or data. You cast it back to the real type inside.</details>

---

## 8. Hard Questions (optional for now: come back after Day 15)
<details><summary>1. In an event loop, compare a `switch` on the event type, a function-pointer table and virtual functions.</summary>A `switch` is a direct branch or a jump table, and every case can be inlined. A function-pointer table is one indirect call, flexible at run time, and usually not inlinable. A virtual call loads the vptr, then the vtable slot, then makes an indirect call (Day 18). All of them are cheap when well predicted. The difference is mostly inlining and code locality. On the hottest path, HFT code prefers compile-time dispatch (templates), and keeps run-time tables for configuration-driven parts.</details>
<details><summary>2. Why is `const T& max(const T& a, const T& b)` dangerous, and how would you design it safer?</summary>If called with temporaries, it returns a reference to one of them, and the reference dangles after the full expression (BUG2). Safer: return by value for cheap types, or (for a non-template function) add a deleted rvalue overload, `const Price& better_bid(Price&&, Price&&) = delete;`, so calls with two temporaries no longer compile.</details>
<details><summary>3. Design a callback registry for a market-data handler: zero allocations after startup, stable latency.</summary>Fixed-size arrays of `{fn, ctx}` per event type, registration only during startup (it can fail if full: no growth), dispatch as a tight loop over a contiguous array (cache-friendly), no `std::function` (it may allocate), no locks if it's single-threaded. You can freeze the table after startup and keep it `const` for safety.</details>

---

## 9. Mini Project: dispatch benchmark + `sort_by` field
1. In your backtester's event loop (or a new `dispatch_bench.cpp`), generate 10 million random `Event`s (types Tick/Fill/Cancel).
2. Process them in two ways and time each with `std::chrono::steady_clock`, using `-O2`:
   - a `switch (e.type)` calling three normal functions,
   - your `Dispatcher` with one handler per type.
3. Write down both times per event in ns. Then make the event types **predictable** (a repeating pattern) and run again. What changed, and why (branch prediction)?
4. Bonus: write `void sort_by(Fill* fills, std::size_t n, Qty Fill::* field)` (a simple insertion sort is fine) and sort by `&Fill::qty`, then by `&Fill::fee`, with the same function.

---

## Checklist
- [ ] Watched the video(s) and read the theory
- [ ] `impl.cpp` passes all tests (`All Day 07 tests passed`)
- [ ] Part B notes written
- [ ] `exercises.cpp`: Q1–Q4, C1–C4, BUG1–BUG3 done
- [ ] Mini project numbers written down
