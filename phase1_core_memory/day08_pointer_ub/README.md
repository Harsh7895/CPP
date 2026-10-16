# Day 08 — Dangling & wild pointers, ownership, invalidation, strict aliasing
> Phase 1 · Est. time 40–45 min (paired with Day 09) · Builds on: Day 06 (pointers, `new[]/delete[]`), Day 07 (references, callbacks with `void* ctx`)

## 🎯 Today in plain words
A pointer is only an address. It doesn't know whether the object at that address is still **alive**.
Today you learn the four ways a pointer goes bad: the object died (**dangling**), it never pointed anywhere (**wild**), nobody is sure who should delete it (**ownership**), or a container moved its data (**invalidation**).
You also learn **strict aliasing**: the rule that lets the optimizer assume an `int*` and a `float*` never point at the same memory. Break it and `-O2` gives a different answer than `-O0`.

Why a trader cares: these are the bugs that pass every test and then, once a week, send a wrong quantity. Every one is **undefined behaviour (UB)**: the program may do anything, including "look fine".

Files: `impl.cpp` (safe tools: bit-casting, parsing wire bytes, handles instead of pointers) and `exercises.cpp` (the **10-bug hunt**: the PLAN build for today).

---

## 📺 Watch first

| # | Video | Length | What to watch for |
|---|-------|--------|-------------------|
| 1 | **Back to Basics: Pointers and Memory, Ben Saks, CppCon 2020.** Search YouTube for that exact title | about 1 hr (watch the first 30 min) | Object lifetime, heap vs stack, and what "dangling" means. Covers §1.1–1.4. |
| 2 | [Type Punning in C++17: Avoiding Pun-defined Behavior, Scott Schurr, CppCon 2017](https://isocpp.org/blog/2018/09/cppcon-2017-type-punning-in-cpp17-avoiding-pun-defined-behavior-scott-schur) (the page links the video) | about 1 hr (watch about 20 min around the `memcpy` / union examples) | Why `reinterpret_cast` punning is UB and what to do instead. Covers §1.6 and §2.3. |

**Not covered well by the videos:** container invalidation and handles (§1.5, §2.2), and the tools you can actually use on your Windows g++ (§1.7). Read those carefully.

---

## 0. Warm-up (Days 06–07)
1. What's wrong with `const Price& best = better_bid(295000, 295035);`?
2. `std::int64_t* p = prices + 4;` for a 4-element array. Legal? And `*p`?
3. In a callback dispatcher, why does each handler get a `void* ctx`?

<details><summary>Answers</summary>

1. Both arguments are temporaries. They die at the end of the line, so `best` refers to a dead object (a dangling reference). Store a copy instead: `Price best = ...`.
2. Making a pointer one past the end is legal. Reading `*p` is UB.
3. A plain function pointer can't carry state. `ctx` points at the handler's data, and the handler casts it back to the real type. (Today: what if that data dies before the handler runs? BUG10.)
</details>

---

## 1. Theory

### 1.1 Lifetime: the object can die while the pointer lives on
**In one line:** every object has a **lifetime** (from creation to destruction), and a pointer that outlives its object is **dangling**.

An object dies in one of three ways:
| How it dies | Example |
|---|---|
| Its scope ends (stack) | a local variable when the function returns |
| Someone frees it (heap) | `delete p;` |
| Its container moves it | `std::vector` grows and copies everything to a new block |

```cpp
std::int64_t* p = nullptr;
{
    std::int64_t px = 295035;
    p = &px;            // p points at px
}                       // px dies here
std::cout << *p;        // ❌ UB: px is gone. It may still print 295035. That's the trap.
```

**Analogy:** a pointer is a house number written on a note. The house can be demolished and the note still says "No. 12". Going to No. 12 might find rubble, or a brand-new house with strangers in it.

### 1.2 Dangling vs wild vs null
| Kind | What it is | Example | Can you check it? |
|---|---|---|---|
| **null** | points at nothing, on purpose | `int* p = nullptr;` | yes: `if (p)` |
| **wild** | never set: holds a random address | `int* p;` | no |
| **dangling** | was valid, the object died | after `delete p;` | **no**: it looks exactly like a valid pointer |

**Rule:** you can never test whether a pointer is dangling. You can only **design** so it can't happen.

### 1.3 Use-after-free often "works", and that's the danger
```cpp
Order* o = new Order{1, 295035, 10};
delete o;
std::cout << o->qty;    // ❌ UB. Often still prints 10: delete doesn't erase memory.
Order* n = new Order{2, 300000, 99};   // the allocator may REUSE the same block…
std::cout << o->qty;    // …now prints 99: you're reading someone else's order
```
Habit: after `delete p;` write `p = nullptr;`. It doesn't fix other copies of the pointer, though. That's the next topic.

### 1.4 Ownership: who calls `delete`?
**In one line:** the **owner** of a heap object is the one piece of code responsible for destroying it. Everything else just **observes** it.

| Problem | What happened |
|---|---|
| **Leak** | nobody deleted it (zero owners) |
| **Double free** | two places deleted it (two owners) |
| **Dangling** | the owner deleted it while an observer still used it |

The modern C++ convention (C++ Core Guidelines):
| Type | Meaning |
|---|---|
| `T*` or `T&` (raw) | **non-owning**: "I'm looking at it, I'll never delete it" |
| `std::unique_ptr<T>` | the single owner (Day 26) |
| `std::vector<T>`, `std::string` | own their elements |

So in new code, a raw pointer **never** owns anything. If you see `delete` on a raw pointer in modern code, ask "is this really the only owner?".

**Analogy:** a rental car has one renter who must return it (the owner). Passengers can ride in it (observers), but a passenger can't return the car, and can't keep riding after it's been returned.

### 1.5 Invalidation: the container moved your object
**In one line:** some container operations move elements to new memory, so old pointers, references and iterators into it become dangling. That's called **invalidation**.

```cpp
std::vector<Level> book;
book.reserve(2);
book.push_back({295035, 10});
Level* best = &book[0];          // pointer INTO the vector's buffer
book.push_back({295040, 5});     // fits (capacity 2): best still OK
book.push_back({295045, 7});     // capacity full → new bigger buffer, copy, free old one
best->qty = 0;                   // ❌ UB: best points into the freed old buffer
```

What invalidates pointers into a `std::vector`:
| Operation | Pointers/refs/iterators |
|---|---|
| `push_back` / `emplace_back` / `insert` **when size == capacity** | **all** invalid |
| `insert` / `push_back` with spare capacity | at or after the insert point invalid |
| `erase(it)` | at or after `it` invalid |
| `reserve(n)` that grows / `shrink_to_fit` | **all** invalid |
| `clear()` | all invalid |
| reading, `operator[]` | nothing |

**Fixes:**
1. Keep an **index** (a "handle"), not a pointer: `std::size_t best = 0; book[best].qty`. The index survives a reallocation. You build this in `impl.cpp` Step 4.
2. `reserve()` enough up front, so it never reallocates (common in HFT: reserve at startup).
3. Take the pointer again after every modification.

### 1.6 Strict aliasing: don't read an object through the wrong type
**In one line:** you may only read or write an object through its **own type** (or its signed/unsigned twin). There's one exception: `char`, `unsigned char` and `std::byte` may look at anything.

**Aliasing** = two pointers pointing at the same memory. The rule lets the compiler assume **an `int*` and a `float*` never alias**, so it can keep values in registers.

```cpp
int foo(int* i, float* f) {
    *i = 1;
    *f = 0.0f;       // compiler: "a float write can't change an int"
    return *i;       // so it just returns 1 without reloading
}
int x = 5;
std::cout << foo(&x, reinterpret_cast<float*>(&x));
// your g++:  -O0 prints 0   |   -O2 prints 1   ← same code, different answers (UB)
```

**The common real-world case: type punning** (reading the bits of one type as another):
```cpp
float f = 1.0f;
std::uint32_t bad = *reinterpret_cast<std::uint32_t*>(&f);   // ❌ UB (strict aliasing)

std::uint32_t ok1;
std::memcpy(&ok1, &f, sizeof f);                             // ✅ always allowed
auto ok2 = std::bit_cast<std::uint32_t>(f);                  // ✅ C++20, <bit>, cleanest
// both give 0x3F800000
```
`memcpy` of 4 or 8 bytes is **not** a function call at `-O2`. It becomes a single `mov`. Safe and free.

### 1.7 Tools that work on YOUR machine (Windows MinGW g++)
| Tool | Catches | How |
|---|---|---|
| `-Wall -Wextra` (always on) | return of a local's address, storing a local's address somewhere long-lived, type punning, uninitialized vars. It does **not** catch most use-after-free or invalidation (BUG1, 4, 5, 7) | already in our build line |
| `-O2` while compiling | many warnings only appear when the optimizer runs (it does the deep analysis) | build with `-O2` too, not only `-O0` |
| `-D_GLIBCXX_ASSERTIONS` | `v[i]` out of range on vectors/strings: stops with a clear message | add it to debug builds |
| `-fsanitize=address,undefined` | use-after-free, out-of-bounds, misaligned, overflow at run time | ❌ not on MinGW. Use Linux/WSL (Day 79) |

> 🔍 **Going deeper (optional):** `-D_GLIBCXX_DEBUG` turns on full checked iterators, which catches using an iterator after invalidation. On your MinGW it does stop the program, but it prints nothing (exit code 127). On Linux it prints a clear report.

---

## 2. Internals: what actually happens

### 2.1 Why use-after-free reads "valid-looking" data
```
new Order{1,...}      delete o               new Order{2,...}
heap: [ id=1 qty=10 ]  →  [ (free) qty=10? ]  →  [ id=2 qty=99 ]
        ▲ o                  ▲ o (dangling)          ▲ o AND n
```
`delete` gives the block back to the **allocator** (the library code that hands out heap memory). The allocator writes its own bookkeeping into part of the block (a pointer to the next free block), but the rest is left as it was. The next `new` of the same size often gets **the same block**. The bug shows up later, somewhere else. That's why these bugs are so hard to find.

### 2.2 What `push_back` does when the vector is full
```
before:  data ─► [ A ][ B ]            size 2, capacity 2      best ─► A
step 1:  allocate new block           [ _ ][ _ ][ _ ][ _ ]    (capacity doubles on libstdc++)
step 2:  move A, B across             [ A ][ B ][ C ][ _ ]
step 3:  free the old block           [ freed ][ freed ]      best ─► still the OLD A ❌
```
The old and new blocks exist **at the same time** (during step 2), so the new address is always different from the old one. A pointer kept from before is **guaranteed** to be stale.

### 2.3 How the optimizer uses the aliasing rule
For `foo(int* i, float* f)` at `-O2`, g++ emits roughly:
```asm
mov DWORD PTR [rcx], 1        ; *i = 1
mov DWORD PTR [rdx], 0        ; *f = 0.0f
mov eax, 1                    ; return 1  ← no reload of *i: "a float can't alias an int"
```
This is **type-based alias analysis**. It's why `-O2` code is fast: values stay in registers instead of being reloaded after every write. If your code breaks the rule, you get stale values, silently.

### 2.4 Parsing wire bytes: the memcpy rule
An exchange message arrives as raw bytes: `unsigned char buf[...]`. The tempting way is:
```cpp
std::int64_t px = *reinterpret_cast<const std::int64_t*>(buf + 4);   // ❌ two problems
```
1. **Aliasing:** there's no `int64_t` object at `buf + 4`, only bytes. Reading them as an `int64_t` is UB.
2. **Alignment:** an `int64_t` should sit at an address divisible by 8. `buf + 4` may not be. x86 tolerates this (a little slower), but some CPUs crash, and the compiler may emit instructions that require alignment.

`std::memcpy(&px, buf + 4, 8);` has neither problem, and compiles to one `mov` on x86. This is how production feed handlers parse.

---

## 3. Implementation (`impl.cpp`)

Build: `g++ -std=c++20 -O2 -Wall -Wextra -pedantic impl.cpp -o impl` then `./impl`.

**Step 1: `float_bits(float)` and `float_from_bits(std::uint32_t)`.** Use `std::memcpy` in one and `std::bit_cast` in the other, so you've used both. 1.0f must give `0x3F800000`.

**Step 2: `load_i64(const unsigned char* p)` and `load_i32(const unsigned char* p)`.** Read 8 / 4 little-endian bytes from **any** address, aligned or not. No `reinterpret_cast`.
<details><summary>Hint</summary>`std::int64_t v; std::memcpy(&v, p, sizeof v); return v;`. Your CPU is little-endian (the file `static_assert`s it), so the bytes are already in the right order.</details>

**Step 3: `parse_quote(buf, len, out)`.** Wire format (16 bytes):
```
offset 0..3   symbol, 4 ASCII chars, no '\0'
offset 4..11  price  (int64, little-endian)
offset 12..15 qty    (int32, little-endian)
```
Return `false` and touch nothing if `len < 16`. Otherwise fill `out->sym` (add the `'\0'` yourself), `out->px` and `out->qty`, and return `true`. Reuse Step 2.

**Step 4: `LevelStore`: handles instead of pointers.** `add(px, qty)` appends a level and returns its **index** as a `LevelId`. `at(id)` returns a reference to it. `size()` is the count. The tests keep an id, add 1,000 more levels (forcing several reallocations), then check that the id still finds the right level.
<details><summary>Hint</summary>`levels_.push_back(...)` then `return static_cast<LevelId>(levels_.size() - 1);`. Notice: the <b>reference</b> returned by `at()` is only safe until the next `add()`. The <b>id</b> is safe forever.</details>

**Step 5: `remove_filled(std::vector<Order>& v)`.** Remove every order with `qty == 0`, keep the others **in their original order**, and return how many you removed. Watch out for the classic erase-in-a-loop bug (BUG5).
<details><summary>Hint</summary>Option A: a loop with `it = v.erase(it);` (erase returns the next valid iterator), and `++it` only when you <b>didn't</b> erase. Option B (faster, O(n)): `std::erase_if(v, [](const Order& o){ return o.qty == 0; });` (C++20). Try A first, then B.</details>

### Part B: label the ownership (write the answers at the bottom of `impl.cpp`)
```cpp
struct Book;                        // some order book
Book* make_book();                  // returns a new Book: caller must delete it
void print_book(const Book* b);     // reads it
struct Strategy { Book* book; };    // a strategy that looks at a book it doesn't create
Book* b = make_book();  Strategy s{b};  print_book(b);  delete b;
```
For `make_book`'s return value, `print_book`'s parameter, `Strategy::book` and `b`: owning or non-owning? Then: what's dangerous about `s` after the last line?

---

## 4. Output Questions

**Q1: same code, two answers**
```cpp
[[gnu::noinline]] int foo(int* i, float* f) { *i = 1; *f = 0.0f; return *i; }
int x = 5;
std::cout << foo(&x, reinterpret_cast<float*>(&x)) << '\n';
```
Build twice: `-O0`, then `-O2`.
<details><summary>Answer</summary>On your g++: `0` at `-O0` and `1` at `-O2`. At `-O0` it reloads `*i` and sees the float write (0.0f is all zero bits). At `-O2` it trusts the aliasing rule and returns the 1 it just stored. That's UB showing itself: there is no "correct" answer.</details>

**Q2**
```cpp
std::vector<int> v;
v.reserve(4);
v.push_back(1);
int* p = &v[0];
v.push_back(2); v.push_back(3); v.push_back(4);
std::cout << (p == &v[0]) << ' ';
v.push_back(5);
std::cout << (p == &v[0]) << '\n';
```
<details><summary>Answer</summary>`1 0`. The first four fit in the reserved capacity, so there's no move. The fifth exceeds the capacity, so there's a new buffer, and the address <b>always</b> changes (§2.2). Note: comparing a stale pointer is technically questionable too, but on every real compiler it just compares the addresses.</details>

**Q3**
```cpp
float f = 1.0f;
std::cout << std::hex << std::bit_cast<std::uint32_t>(f) << ' '
          << std::bit_cast<std::uint32_t>(-1.0f) << '\n';
```
<details><summary>Answer</summary>`3f800000 bf800000`. The only difference is the top bit, the <b>sign bit</b> of an IEEE-754 float.</details>

**Q4**
```cpp
std::vector<int> ids{1, 2, 3, 4, 6, 7};
for (auto it = ids.begin(); it != ids.end(); ) {
    if (*it % 2 == 0) it = ids.erase(it);
    else ++it;
}
for (int id : ids) std::cout << id << ' ';
```
<details><summary>Answer</summary>`1 3 7`. This is the <b>correct</b> pattern: `erase` returns the iterator to the next element, and you only `++` when you didn't erase. Compare with BUG5.</details>

---

## 5. Debugging: the 10-bug hunt (today's PLAN build)
All 10 are in `exercises.cpp`. Build each one with `-DBUG1` … `-DBUG10` (one at a time), using **`-O2 -Wall -Wextra`**.
For each one: **(a)** name the bug type, **(b)** say whether g++ warned, **(c)** write the fix. Many will seem to "work". That's the lesson.

| # | Snippet (short) | Your notes |
|---|---|---|
| 1 | `Order* o = new Order{...}; delete o; o->qty` | |
| 2 | `Price* best_price() { Price p = ...; return &p; }` | |
| 3 | two structs both `delete` the same `Book*` | |
| 4 | `Level* best = &book[0];` then `push_back` past capacity | |
| 5 | `for (it...) if (filled) ids.erase(it); ++it` | |
| 6 | `const char* sym = std::string("ESZ6").c_str();` | |
| 7 | `Level& top = levels.front();` then `levels.insert(begin, ...)` | |
| 8 | `*reinterpret_cast<std::uint32_t*>(&f)` | |
| 9 | `*reinterpret_cast<const std::int64_t*>(buf + 3)` | |
| 10 | callback `ctx` points at a local that has gone out of scope | |

<details><summary>Answers (try first!)</summary>

1. **Use-after-free.** On your g++: no warning, and it printed `qty = 7` (garbage: the allocator reused the bytes). Fix: read what you need before `delete`, then set `o = nullptr`. Better: let one owner (later `unique_ptr`) control the lifetime.
2. **Returning the address of a local.** It dangles as soon as the function returns. g++ warns (`-Wreturn-local-addr`), and at `-O2` your g++ really returns `nullptr` (it prints `-1`). Fix: return by value.
3. **Double free (two owners).** On your machine it exits <b>silently</b>, with no crash. The heap is corrupted, and the crash comes later, somewhere else. Fix: decide on <b>one</b> owner. The other holds a non-owning pointer and never deletes.
4. **Invalidation by reallocation.** Prints `qty = 10`: the write went into the freed old buffer. Fix: store an index (handle), or `reserve` enough first.
5. **Erase in a loop.** After `erase(it)`, `it` is invalid, and `++it` on it is UB (and it also skips an element: it prints `left 2`). Fix: `it = ids.erase(it);` and only `++it` otherwise, or `std::erase_if`.
6. **Dangling pointer into a temporary.** The `std::string` dies at the end of the line, so `sym` points into freed memory. On your machine it prints `ZZZZ…`: the next string got the same heap block! Fix: keep the string alive in a variable: `std::string s = ...; const char* sym = s.c_str();`.
7. **Invalidation by insert.** Inserting at the front shifts every element, so `top` now refers to a different element, or to freed memory if it reallocated (on your machine: a garbage price). Fix: re-read `levels.front()` after the insert, or use a handle.
8. **Strict-aliasing violation (type punning).** g++ warns `-Wstrict-aliasing`, but the output looks right. That's luck, not correctness. Fix: `std::bit_cast<std::uint32_t>(f)` or `memcpy`.
9. **Aliasing + misalignment** on a byte buffer. No warning, and the right answer on x86. It's still UB, and a different CPU or compiler can break it. Fix: `std::memcpy(&px, buf + 3, 8);` (impl Step 2).
10. **Dangling `ctx`.** The callback was registered with the address of a local in `setup()`. When `setup()` returns, the local is gone, but the dispatcher still writes to it later. g++ warns `-Wdangling-pointer`. Fix: `ctx` must point at something that lives as long as the registration (a member of a long-lived object, or a static). This is a real bug in callback-based systems.
</details>

---

## 6. Performance (the HFT angle)
- **Handles cost almost nothing.** `book[idx]` is one add plus one load, the same as using a pointer. You trade nothing for safety against reallocation.
- **`reserve` at startup.** A `push_back` that reallocates copies every element and calls the allocator: microseconds on the hot path, plus it invalidates every pointer. Reserve the maximum book depth up front.
- **`memcpy` / `bit_cast` are free.** For 1, 2, 4 or 8 bytes, `-O2` turns them into one `mov`. There's never a speed reason to use `reinterpret_cast` punning.
- **Strict aliasing helps you.** It lets the compiler keep values in registers. Some projects (like the Linux kernel) turn it off with `-fno-strict-aliasing` and accept slower code.
- **`char*` / `unsigned char*` writes can slow loops.** Because `char` may alias anything, the compiler must assume a write through a `char*` might change any other value, and reload it. In tight parsing loops, copy values into locals first.
- **UB isn't a "slow path", it's a wrong path.** The optimizer can delete checks it can prove "never matter" under the rules. A dangling read can return an old price with no crash at all.

---

## 7. Interview Questions
<details><summary>1. Dangling vs wild pointer?</summary>Wild: it was never initialized, so it holds a random address. Dangling: it pointed at a valid object, and that object died (freed, out of scope, or moved by a container).</details>
<details><summary>2. Can you detect a dangling pointer at run time with an `if`?</summary>No. It holds a normal-looking address. You prevent it by design: clear ownership, handles, and lifetimes that are obviously long enough. Tools like AddressSanitizer find it while testing.</details>
<details><summary>3. What does "owning pointer" mean? What's the modern convention?</summary>The owner is responsible for destroying the object. Modern C++: raw `T*` and `T&` are non-owning, and ownership lives in `unique_ptr`, `shared_ptr` or containers.</details>
<details><summary>4. Name three ownership bugs.</summary>A leak (no owner deletes), a double free (two owners delete), and use-after-free (the owner deletes while someone still uses it).</details>
<details><summary>5. When does `vector::push_back` invalidate pointers?</summary>When `size() == capacity()`: it reallocates, and every pointer, reference and iterator becomes invalid. Otherwise only `end()` is invalidated.</details>
<details><summary>6. How do you erase from a vector while iterating?</summary>`it = v.erase(it)`, incrementing only when you don't erase. Or `std::erase_if` / erase-remove (O(n), and better).</details>
<details><summary>7. What is the strict aliasing rule?</summary>An object may only be accessed through its own type (or the signed/unsigned version, or `char`/`unsigned char`/`std::byte`). Otherwise it's UB, and the optimizer assumes pointers of different types don't overlap.</details>
<details><summary>8. How do you safely reinterpret the bits of a float as an int?</summary>`std::bit_cast<std::uint32_t>(f)` (C++20) or `std::memcpy`. Not `reinterpret_cast`, and not union punning (that's allowed in C, but UB in C++).</details>
<details><summary>9. Is `reinterpret_cast` always UB?</summary>No. The cast itself is fine. <b>Accessing</b> the object through the wrong type is the problem. Viewing any object as `unsigned char*` is allowed, for example.</details>
<details><summary>10. Why can a use-after-free bug print the correct value?</summary>`delete` doesn't erase memory. The bytes remain until the allocator reuses the block. That's why these bugs pass tests.</details>
<details><summary>11. Why is `c_str()` on a temporary string dangerous?</summary>The pointer points into the string's buffer, and the temporary is destroyed at the end of the full expression, so the pointer dangles.</details>

---

## 8. Hard Questions (optional for now: come back after Day 15)
<details><summary>1. Your order book stores `std::vector<Level>` and strategies keep `Level*` to their price level. Redesign it so it's safe and still fast.</summary>Options: (a) handles (indices) instead of pointers, plus `reserve` for the maximum depth; (b) a stable-address container (a fixed array or pool allocated once, never moved: Day 11+); (c) if you need erase in the middle, a free-list of slots plus a generation counter in the handle, so a stale handle is <b>detected</b> instead of silently reading a reused slot. All are O(1) with no pointer chasing beyond what you had.</details>
<details><summary>2. Why is `std::memcpy` allowed when `reinterpret_cast` access isn't, if they read the same bytes?</summary>`memcpy` is defined to copy the <b>object representation</b> (bytes), and it starts a new object's value in the destination of the right type. So no object is ever read through the wrong type. The compiler knows this, and recognises small `memcpy`s as plain loads and stores.</details>
<details><summary>3. A function takes `(unsigned char* out, const std::int64_t* in, std::size_t n)` and is slower than expected. Why might aliasing matter?</summary>`out` is `unsigned char*`, which may alias <b>anything</b>, including `in` and `n`. After each write to `out`, the compiler must assume `in[...]` or `n` might have changed, and reload them, which blocks vectorization. Fixes: copy `n` into a local, use `__restrict`, or write through a non-char type.</details>

---

## 9. Mini Project: UB audit of your backtester
1. Build your backtester with `-O2 -Wall -Wextra -D_GLIBCXX_ASSERTIONS`. Fix every warning, or write down why it's safe.
2. Search your code for `&v[`, `.data()`, `&vec.back()`, `reinterpret_cast` and `c_str()`. For each pointer kept across a `push_back`/`insert`/`erase`, switch it to an index.
3. If you parse any input as bytes (CSV numbers are fine; binary files aren't), switch to `memcpy` loads.
4. Write a 5-line "ownership map": for each heap object in the backtester, who owns it, and who only observes it?

---

## Checklist
- [ ] Watched the video(s) and read the theory
- [ ] `impl.cpp` passes all tests (`All Day 08 tests passed`)
- [ ] Part B ownership answers written
- [ ] `exercises.cpp`: Q1–Q4 and **all 10 bugs** with notes
- [ ] Mini project audit done
