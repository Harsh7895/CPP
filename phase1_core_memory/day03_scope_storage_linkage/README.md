# Day 03 — Namespaces, Scope, Storage Duration, Linkage, `static`, `extern`
> Phase 1 · Est. time 60–90 min · Builds on: Day 01 (`nm`, linker), Day 02 (TU, declaration vs definition, ODR, `inline`)

Files in this folder:
| File | Purpose |
|------|---------|
| `oms.h` / `oms.cpp` | Order-ID generator, lazy venue, thread counter, live-order counter. **You implement it** |
| `risk.h` / `risk.cpp` | Pre-trade check with its own private counter. **You implement it** |
| `impl.cpp` | Tests and the Part B "break it" lab |
| `exercises.cpp` | Output questions and debugging |

---

## 0. Warm-up (Days 01–02)
1. A header has `const int kLimit = 100;` and is included by two `.cpp` files. Why is there **no** "multiple definition" error?
2. A header has `int g_orders = 0;` and is included by two `.cpp` files. Name **three** different fixes.
3. `nm` prints `U foo()` in `a.o`, and `foo()` doesn't appear in any other object. What error do you get, and from which tool?

<details><summary>Answers</summary>

1. A namespace-scope `const` variable has **internal linkage** in C++, so each TU gets its own private copy. Today explains *why*.
2. (a) `inline int g_orders = 0;` gives one shared object. (b) `extern int g_orders;` in the header plus one definition in a `.cpp`. (c) `static int g_orders = 0;` links, but gives **one separate copy per TU**, which is usually *not* what you want.
3. `undefined reference to foo()` from the **linker**.
</details>

---

## 1. Theory

### 1.1 Four different properties (don't mix them up)
Every name and object in C++ has four **independent** properties. Most confusion about `static` comes from blending them.

| Property | Question it answers | Belongs to |
|----------|--------------------|-----------|
| **Scope** | *Where in the source* can I use this name? | the name |
| **Linkage** | Can *another scope or TU* refer to the same entity by this name? | the name |
| **Storage duration** | *When is the memory* allocated and freed? | the object |
| **Lifetime** | *When is the object alive* (between constructor and destructor)? | the object |

Example: `void f() { static int n = 0; }`. `n` has **block scope**, **no linkage** and **static storage duration**.

### 1.2 Scope
| Scope | Example | Visible… |
|-------|---------|----------|
| Block | `{ int x; }`, loop variables, `if (auto it = …)` | from the declaration to the closing `}` |
| Function parameter | `void f(int x)` | in the function body |
| Class | members | inside the class and its member functions, or via `obj.` / `Class::` |
| Namespace | `namespace oms { int x; }` | inside the namespace, or as `oms::x` |
| Global (the global namespace) | `int x;` at file level | everywhere after the declaration, or as `::x` |

**Name lookup goes from the innermost scope outwards and stops at the first match.** An inner name therefore **shadows** an outer one. `::x` forces the global one. `-Wshadow` warns about it. It isn't part of `-Wall`.

### 1.3 Namespaces
```cpp
namespace oms { ... }                 // named
namespace oms::limits { ... }         // nested (C++17 syntax)
namespace { int helper; }             // anonymous: everything inside gets INTERNAL linkage
namespace fs = std::filesystem;       // alias
inline namespace v2 { ... }           // members are also visible in the enclosing namespace (versioning)

using oms::next_order_id;             // using-DECLARATION: brings in ONE name (fine in a .cpp or function)
using namespace oms;                  // using-DIRECTIVE: brings in everything (never in a header)
```
**ADL (argument-dependent lookup):** for an unqualified call `print(q)`, the compiler also searches the namespaces of the **argument types**. That is why `std::cout << x` and `swap(a, b)` work without qualification.

### 1.4 Storage duration
| Duration | Declared as | Memory lives in | Allocated → freed |
|----------|-------------|-----------------|-------------------|
| **Automatic** | ordinary locals, parameters | the stack (or a register) | block entry → block exit |
| **Static** | namespace-scope variables, `static` locals, `static` data members | `.data` / `.bss` / `.rodata` | the whole program run |
| **Thread** | `thread_local` | a per-thread TLS block | thread start → thread exit |
| **Dynamic** | `new`, `malloc` | the heap | until `delete` / `free` |

### 1.5 Linkage
| Linkage | Meaning | Who has it |
|---------|---------|-----------|
| **None** | the name refers to an entity only in its own scope | local variables, parameters, local classes |
| **Internal** | visible only inside **this TU** | namespace-scope `static` variables and functions; everything in an **anonymous namespace**; namespace-scope `const` / `constexpr` variables (unless `inline` or `extern`) |
| **External** | the same entity across **all TUs** | ordinary functions, non-const globals, classes and their members, `inline` variables, `extern const` |

In `nm`, a **lowercase** letter (`t`, `d`, `b`, `r`) means a local symbol with internal linkage. An **uppercase** letter (`T`, `D`, `B`, `R`) means global, external linkage.

### 1.6 The four meanings of `static`
| Where | What `static` does |
|-------|--------------------|
| namespace-scope variable or function | gives the name **internal linkage** (private to the TU) |
| local variable | gives it **static storage duration**: created on the **first call**, kept until the program exits |
| class data member | **one object per class**, not one per instance. It is not part of `sizeof(obj)` |
| class member function | **no `this`**: callable as `Class::f()`, and can only touch static members |

### 1.7 `extern`
- `extern int x;` is a **declaration without a definition**: "x lives in some other TU" (Day 02).
- `extern const int k = 5;` gives a `const` **external** linkage, overriding the internal default.
- `extern "C" { ... }` selects C language linkage with no name mangling (Day 01).

### 1.8 `static` vs anonymous namespace
Both give internal linkage. Prefer the **anonymous namespace**, because it also works for **types**. You can't write `static struct Helper {...};` to make a class private to a TU. Two TUs that each define their own helper `struct Node` *differently* at global scope are an ODR violation (silent UB, Day 02). Inside anonymous namespaces they are two unrelated types.

### 1.9 Initialization of static-duration objects (preview of Day 14)
- **Constant initialization** (`int g = 5;`, `constexpr`, `constinit`): the value is baked into the binary, with zero runtime cost.
- **Dynamic initialization** (`std::string g = make();`): runs **before `main`**, in definition order *within* a TU, and in **unspecified order across TUs**.
- **Local statics**: initialized the **first time control passes through** the declaration. This is thread-safe since C++11.
- **Destruction**: in the reverse order of construction, after `main` returns.

---

## 2. Internals

### 2.1 Where each variable lives
```cpp
int g_init = 5;                 // .data   (static storage, external linkage)
int g_zero;                     // .bss    (zero-initialized, takes no space in the file)
const int k = 7;                // .rodata (or folded away), internal linkage
static int s_private = 1;       // .data, internal linkage
thread_local int t_count = 0;   // TLS block: one copy per thread
void f() {
    int a = 1;                  // stack frame of this call (or just a register)
    static int calls = 0;       // .bss/.data, the same single object for every call
    int* p = new int(3);        // p is on the stack, *p is on the heap
}
```

### 2.2 What `nm -C oms.o` looks like for a finished solution (your toolchain)
```
0000000000000000 B oms::session_id                          ← uppercase: external
0000000000000080 T oms::next_order_id()                     ← uppercase: external
0000000000000000 d (anonymous namespace)::g_seq             ← lowercase: internal, in .data (initialized to 1)
0000000000000008 b (anonymous namespace)::g_count           ← lowercase: internal, in .bss (zero)
0000000000000000 t (anonymous namespace)::make_primary()    ← internal function
0000000000000048 b guard variable for oms::primary_venue()::v
                 U __cxa_guard_acquire
                 U _tls_index                               ← Windows native TLS, from the thread_local
```
The linker never matches lowercase symbols across object files. That is the entire mechanism behind "internal linkage".

### 2.3 How a function-local static really works ("magic statics")
```cpp
const Venue& primary_venue() { static const Venue v = make_primary(); return v; }
```
compiles to roughly:
```
if (guard_for_v == 0) {                       // FAST PATH: one byte load + branch on EVERY call
    if (__cxa_guard_acquire(&guard_for_v)) {  // slow path: takes a lock, so other threads wait here
        construct v;
        atexit(destroy v);                    // registered for destruction at exit
        __cxa_guard_release(&guard_for_v);    // sets guard = 1
    }
}
return v;
```
- It is thread-safe, and the constructor runs exactly once even with 10 threads racing.
- If the initializer is a **constant expression** (`static int n = 0;`), there is **no guard at all**. It's just a `.data`/`.bss` variable.
- If the constructor throws, the guard is aborted and the next call tries again.
- Calling the function **recursively during its own initialization** is undefined behaviour (in practice a deadlock or an exception).

### 2.4 How `thread_local` works
Each thread has a **TLS block**. Access is `base_of_this_thread's_TLS + fixed offset`:
- Linux x86-64: the base is in the `fs` segment register, so in the main executable it's `mov eax, fs:[offset]`, as cheap as a global.
- Windows (your MinGW): the TEB (via `gs`) → the thread's TLS array → the slot at `_tls_index` → offset. That's a couple of extra loads.
- Inside a **shared library** on Linux, it may need a call to `__tls_get_addr`, which is much slower.
- A `thread_local` with a non-trivial constructor also gets a per-access "initialized yet?" check, like a local static.

### 2.5 Destruction order
Static-duration objects are destroyed in the **reverse order in which their construction completed**. A local static constructed during `main` is therefore destroyed **before** globals that were constructed before `main` (see Q3).

---

## 3. Implementation

### Part A: implement (tests are in `impl.cpp`)
| What | Where | Requirements |
|------|-------|--------------|
| private sequence + total counter | `oms.cpp`, global **anonymous namespace** | the total must be named `g_count`; nothing outside `oms.cpp` can see either |
| `next_order_id()` | `oms.cpp` | returns `(uint64(session_id) << 32) \| seq`, then advances |
| `reset_order_ids(start)` | `oms.cpp` | the next sequence becomes `start`; the total is **not** cleared |
| `ids_issued()` | `oms.cpp` | total since program start |
| `primary_venue()` | `oms.cpp` | a **function-local static** `Venue{"NSE", 1}`, constructed lazily, once |
| `venue_init_count()` | `oms.cpp` | number of times the venue was constructed (0 before the first call!) |
| `bump_thread_counter()` | `oms.cpp` | a `thread_local` counter; returns the new value |
| `Order` ctor / copy ctor / dtor | `oms.h` | maintain the **static member** `live_` |
| `g_count` + `check_qty` + `checks_done` | `risk.cpp` | a global `static` counter (same name as in oms.cpp!); valid if `0 < qty <= kMaxOrderQty` |

```
g++ -std=c++20 -O2 -Wall -Wextra -pedantic impl.cpp oms.cpp risk.cpp -o impl && ./impl
```
Hint for `venue_init_count`: `static const Venue v = make_primary();`, where `make_primary()` is a private helper that bumps a private counter.

### Part B: break it on purpose (record each result at the bottom of `impl.cpp`)
Restore the code after each step.
1. Make **both** `g_count`s external: remove `static` in `risk.cpp`, and move the one in `oms.cpp` out of the anonymous namespace. What does the linker say? Check `nm -C oms.o risk.o | grep -w g_count`. *(Notice that the two variables don't even have the same type. Does the linker care?)* Bonus: fix it **without** internal linkage by putting each one in its own named namespace.
2. In `impl.cpp`, add `extern std::uint64_t g_seq;` (use your sequence variable's name) and print it. Which stage fails, and why can't it be found?
3. `g++ -std=c++20 -O0 -c oms.cpp && nm -C oms.o`: list which symbols are uppercase and which are lowercase. Find the `guard variable`.
4. Move the `Venue` from a local static to a namespace-scope variable in `oms.cpp`. Which assert fires? What does that tell you about *when* it is constructed?
5. Change `thread_local` to `static` in `bump_thread_counter`. Which assert fires, and what value did the other thread see?
6. Delete your `Order` copy constructor (let the compiler generate it). Which assert fires? What would `live()` be at the end?

---

## 4. Output Questions
Predict first, then build and run `exercises.cpp`.

**Q1**
```cpp
int x = 1;
void q1() {
    int x = 2;
    { int x = 3; std::cout << x << ' ' << ::x << ' '; }
    std::cout << x;
}
```
**Q2**
```cpp
int next_seq() { static int n = 0; return ++n; }
std::cout << next_seq() << ' ' << next_seq() << ' ' << next_seq();
```
**Q3**: write down every line, in order, for the whole program
```cpp
struct Tracer { Tracer(const char* n) { /* prints "ctor n" */ }  ~Tracer() { /* prints "dtor n" */ } };
Tracer g_tracer("global");
void f() { static Tracer s("local-static"); Tracer a("auto"); }
int main() { std::cout << "main start\n"; f(); f(); std::cout << "main end\n"; }
```
**Q4**: does it compile? What does it print?
```cpp
namespace mkt { struct Quote {}; void print(const Quote&) { std::cout << "mkt::print"; } }
void q4() { mkt::Quote q; print(q); }
```
**Q5**
```cpp
namespace api {
    namespace v1 { int version() { return 1; } }
    inline namespace v2 { int version() { return 2; } }
}
std::cout << api::version() << ' ' << api::v1::version();
```
**Q6**
```cpp
struct T { static inline int n = 0; T() { ++n; } ~T() { --n; } };
void q6() { T a; { T b; T c = b; } std::cout << T::n; }
```

<details><summary>Answers</summary>

1. **`3 1 2`**. The innermost `x` wins, `::x` reaches the global, and after the block the middle `x` is visible again.
2. **`1 2 3`**. One `n` survives across calls. (Since C++17 the `<<` operands are evaluated left to right.)
3. ```
   ctor global          ← before main (static storage, dynamic initialization)
   main start
   ctor local-static    ← first call of f() only
   ctor auto
   dtor auto            ← end of f()'s block
   ctor auto            ← second call: the local static is NOT constructed again
   dtor auto
   main end
   dtor local-static    ← constructed last, so destroyed first
   dtor global
   ```
4. **Compiles, prints `mkt::print`**. **ADL**: the argument type `mkt::Quote` makes the compiler search namespace `mkt`.
5. **`2 1`**. Members of an `inline namespace` are visible as if they were in the enclosing namespace, and the old version stays reachable by its explicit name.
6. **`0`**, even though `a` is still alive. `T c = b;` uses the **implicit copy constructor**, which doesn't increment `n`, but `c`'s destructor still decrements it. (This is why `Order` needs a hand-written copy constructor today. The Rule of 3/5 is Day 22.)
</details>

---

## 5. Debugging
**B1** (`-DBUG1`): crashes or prints garbage
```cpp
const std::string& venue_name() { std::string s = "NSE"; return s; }
```
**B2** (`-DBUG2`): doesn't compile
```cpp
#include <algorithm>
using namespace std;
int count = 0;
void bug2() { ++count; }
```
**B3** (`-DBUG3`): the VWAP for the second symbol is wrong
```cpp
double vwap_update(double px, double qty) {
    static double pv = 0, vol = 0;
    pv += px * qty;  vol += qty;
    return pv / vol;
}
```
**B4** (`-DBUG4`): the position never changes
```cpp
class Position {
    int qty_ = 0;
public:
    void set(int qty) { int qty_ = qty; }
    int qty() const { return qty_; }
};
```
**B5**: no error, no warning, wrong numbers
```cpp
// limits.cpp                 // strategy.cpp
int g_max_position = 1000;    extern double g_max_position;
                              if (pos > g_max_position) reject();
```

<details><summary>Answers</summary>

- **B1. Dangling reference.** `s` has **automatic** storage and is destroyed when the function returns, so the reference points into a dead stack frame. GCC warns with `-Wreturn-local-addr`. Fixes: return `std::string` **by value** (it's moved or elided, so it's cheap), or make it `static const std::string s = "NSE";`.
- **B2. Compiler: `reference to 'count' is ambiguous`.** `using namespace std;` drags `std::count` into the global lookup, where it collides with your global `count`. Fixes: drop the using-directive; put your variable in your own namespace; write `::count`.
- **B3. A `static` local is ONE object for the whole program**, so every symbol shares the same `pv`/`vol`: `rel = 580.8`. It's also a **data race** if two threads call it. Fix: keep state per symbol in a struct (`struct Vwap { double pv = 0, vol = 0; double update(...); };`).
- **B4. Shadowing.** `int qty_ = qty;` declares a *new local* that hides the member. Fix: `qty_ = qty;`. Catch it with **`-Wshadow`**.
- **B5. Type mismatch across TUs.** Variable names are **not mangled with their type**, so the linker happily binds `double g_max_position` to the 4-byte `int`. `strategy.cpp` then reads 8 bytes as a double, which is UB and gives garbage limits. Fix: declare it **once in a header** that both files include. Never hand-write `extern` declarations in a `.cpp`.
</details>

---

## 6. Performance (the HFT angle)
| Topic | What matters |
|-------|--------------|
| **Globals / statics** | The address is fixed at link time, so a static executable reads a global with one instruction (`mov rax, [rip+off]`). There is no allocation and no pointer chase. In a **shared library**, an *external* global may go through the GOT, which is an extra load. |
| **Internal linkage helps the optimizer** | When a function or variable is `static` or in an anonymous namespace, the compiler **sees every use**. It can inline it and delete the out-of-line copy, constant-propagate a never-modified variable, and pick a custom calling convention. The result is smaller symbol tables and faster links. **Default to internal linkage for everything that isn't API.** |
| **Function-local statics in hot paths** | Every call pays for the **guard check** (a load plus a branch) unless the initializer is a constant expression. The *first* call also takes a lock and runs the constructor. That is a latency spike if the first call happens while the market is open. In a hot path, prefer `constinit`/`constexpr` statics, or initialize at startup and **warm up**. |
| **`thread_local`** | Almost free in the main executable on Linux. It costs a few loads on Windows and a function call in Linux shared libraries. It's great for per-thread scratch buffers and counters, because it needs **no locks and no atomics**. |
| **Shared counters** | A global counter bumped by several threads needs `std::atomic` (Day 64), and it bounces its cache line between cores (false sharing, Days 68 and 74). Per-thread counters that you sum on demand avoid both. |
| **Static init / exit cost** | Dynamic initializers run before `main`, which makes startup slower and the order fragile (Day 14). Destructors of statics run at exit. Some trading systems skip them on purpose with `quick_exit`. |
| **Cache locality** | Globals defined in different TUs end up scattered across `.data`. State that is used together on the hot path belongs **in one struct**, so it shares cache lines. |

---

## 7. Interview Questions
<details><summary>1. What is the difference between scope, linkage, storage duration and lifetime?</summary>Scope: where the name is visible in the source. Linkage: whether the name refers to the same entity from other scopes or TUs. Storage duration: when the memory exists (automatic, static, thread, dynamic). Lifetime: when the object is alive, from the end of construction to the start of destruction. Lifetime is always within the storage duration.</details>
<details><summary>2. What are the meanings of `static` in C++?</summary>At namespace scope: internal linkage. On a local variable: static storage duration, initialized once on the first pass. On a class data member: one per class. On a member function: no `this`.</details>
<details><summary>3. Internal vs external linkage: give examples.</summary>Internal: `static` free functions and variables, anonymous-namespace members, and namespace-scope `const`. External: normal functions, non-const globals, classes, and inline variables. Internal names are invisible to the linker from other TUs.</details>
<details><summary>4. `static` vs anonymous namespace?</summary>Both give internal linkage. An anonymous namespace also works for types (classes, enums), which prevents ODR violations between same-named helper classes in different TUs. It is the modern preference.</details>
<details><summary>5. When is a function-local static initialized? Is it thread-safe?</summary>The first time execution passes its declaration. Since C++11 it is thread-safe: the compiler emits a guard variable and takes a lock on the slow path.</details>
<details><summary>6. When are global variables initialized and destroyed?</summary>Constant initialization happens at compile or load time. Dynamic initialization happens before `main`, in order within a TU and unordered across TUs. Destruction happens after `main`, in reverse order of construction.</details>
<details><summary>7. What does `extern` do?</summary>It declares a variable or function without defining it (it has external linkage and is defined elsewhere). `extern const` forces external linkage. `extern "C"` selects C linkage.</details>
<details><summary>8. Why does `const int x = 5;` in a header not cause a link error but `int x = 5;` does?</summary>A namespace-scope `const` has internal linkage, so each TU has a private copy. A non-const variable has external linkage, which gives multiple definitions.</details>
<details><summary>9. What is `thread_local` and when would you use it?</summary>Thread storage duration: one instance per thread, created at thread start (or first use) and destroyed at thread exit. Use it for per-thread caches, scratch buffers, counters and RNG state, to avoid locking.</details>
<details><summary>10. Where is a static data member stored? Does it affect sizeof?</summary>In `.data` or `.bss`, like a global. It isn't part of any object, so it doesn't affect `sizeof`. Before C++17 it needed an out-of-class definition. Now `static inline` works.</details>
<details><summary>11. Can a static member function be `const` or `virtual`? Why not?</summary>No. Both apply to `this` (constness of the object, and dispatch on the object's dynamic type), and a static member function has no `this`.</details>
<details><summary>12. Using-declaration vs using-directive?</summary>`using std::string;` introduces one name. `using namespace std;` makes all the names visible for lookup. A directive in a header pollutes every includer and can silently change overload resolution.</details>
<details><summary>13. What is ADL?</summary>Argument-dependent lookup: for an unqualified function call, the namespaces of the argument types are also searched. It's what makes `operator<<`, `swap` and `begin`/`end` work.</details>
<details><summary>14. What is an inline namespace used for?</summary>Versioning. Its members appear in the parent namespace, so `lib::f` resolves to the current version, while symbols mangle with the version name. libstdc++ uses `std::__cxx11` for the new `std::string` ABI.</details>
<details><summary>15. What is the Meyers singleton, and what are its drawbacks?</summary>`static T& instance() { static T t; return t; }`. It is lazy and thread-safe, and it avoids the init-order fiasco. Drawbacks: a guard check on every access, a first-call latency spike, destruction-order problems at exit, and hidden global state.</details>

---

## 8. Hard Questions
<details><summary>H1. Your hot path calls `Config::instance()` (a Meyers singleton) about 5 million times a second. What does each call cost? How do you remove the cost?</summary>

Each call does a guard-byte load and a compare-and-branch before returning the address. The branch is well predicted, but it's still extra instructions and an extra cache line (the guard) touched. The first call takes a lock and runs the constructor, so there is a latency spike. Options:
- Fetch the reference **once** outside the loop, or store it as a member (`Config& cfg_;`).
- Make the object constant-initialized: `constinit static Config cfg;` with a `constexpr` constructor. Then there's no guard at all.
- Initialize explicitly at startup, and pass dependencies in (dependency injection) instead of using a global accessor.
- Warm the path up before the open.
</details>

<details><summary>H2. Two .cpp files each define `struct Node { ... };` at global scope with different members, both used only locally. Everything compiles and links. What can go wrong? What if the struct has an inline member function?</summary>

It's an ODR violation: two different definitions of the same class `::Node`. Both TUs emit inline member functions (constructors, destructors, methods) as weak/COMDAT symbols with the **same mangled name** (`Node::Node()`), and the linker keeps **one**. One TU then calls the *other* TU's constructor on its own differently-sized object, which corrupts memory. Whether it "works" depends on inlining and the optimization level (compare with Day 02 Q3). Fix: put file-local types in an **anonymous namespace**, so the two types become `(anonymous)::Node` in two different TUs with internal linkage.
</details>

<details><summary>H3. Compare the cost of reading (a) a local variable, (b) a global with internal linkage, (c) a global with external linkage in a shared library, (d) a thread_local, (e) a function-local static with a non-constant initializer.</summary>

(a) Free: it's in a register or at `[rsp+off]`, which is hot in L1.
(b) One RIP-relative load. The compiler may even prove that it never changes and fold it away.
(c) One load of the address from the GOT plus one load of the value (unless you use `-fvisibility=hidden` or `-fno-semantic-interposition`). The compiler must also assume that any opaque call can modify it, so it reloads it after calls.
(d) TLS base + offset. That's about one instruction in a Linux executable, several loads on Windows, and a `__tls_get_addr` call in a Linux shared object (general-dynamic model). Add a guard check if it has a dynamic initializer.
(e) A guard-byte check plus a RIP-relative access on every call. The first call takes a lock.
For any global in a hot loop, copy it into a local before the loop. Aliasing rules often stop the compiler from doing that for you.
</details>

<details><summary>H4. `limits.cpp` has `int g_max = 1000;` and `strategy.cpp` has `extern double g_max;`. Why is there no error? How would a large codebase protect itself?</summary>

Variables are not mangled with their type (functions are, because of overloading), so both sides use the symbol `g_max` and the linker only matches names. The reader interprets 4 bytes of int plus 4 bytes of whatever follows as a double. It's UB with no diagnostic required. Protection: a single declaration in a header that the defining `.cpp` also includes, so the compiler checks the definition against the declaration; `-Wmissing-declarations`; LTO with `-Wlto-type-mismatch`/`-Wodr`; and avoiding mutable globals altogether.
</details>

---

## 9. Mini Project: what does each kind of counter cost?
An order-ID generator sits on the hot path of every order. Benchmark five implementations with **100 million calls** each, and report **ns/call**:

| Variant | How |
|---------|-----|
| A | today's `oms::next_order_id()`: out-of-line in another TU, with an internal global |
| B | header-only: `inline` function + `inline` variable (the compiler can inline it) |
| C | a function-local static with a **non-constant** initializer (`static uint64_t n = start_from_clock();`) |
| D | a `thread_local` counter |
| E | a `std::atomic<uint64_t>` with `fetch_add(1)` (preview of Day 64) |

Rules: time with `std::chrono::steady_clock`, add each id to a checksum and print it (so the loop isn't optimized away), and build with `-O2`.
Then:
1. Explain the ranking using §2.3, §2.4 and §6.
2. Run `g++ -std=c++20 -O2 -S` on variant C, and find `__cxa_guard_acquire` and the guard check in the assembly.
3. Pick the right variant for (i) a single-threaded backtester and (ii) a multi-threaded order gateway, and say why.
4. Put the winner into your backtester as its order-ID source, inside a namespace, with its state in an anonymous namespace.

---

## Checklist
- [ ] read theory + internals
- [ ] `impl.cpp` passes all tests (Part A)
- [ ] Part B: 6 breakages + observations
- [ ] exercises: Q1–Q6 predicted, B1–B5 explained
- [ ] mini project: 5 timings + explanation
- [ ] (carry-over) Day 01 and Day 02 `impl.cpp` passing
