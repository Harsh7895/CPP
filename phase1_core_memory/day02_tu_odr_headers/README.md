# Day 02 — Translation Units, Declarations vs Definitions, ODR, Headers
> Phase 1 · Est. time 60–90 min · Builds on: Day 01 (pipeline, `nm`, linker errors)

Files in this folder:
| File | Purpose |
|------|---------|
| `instrument.h` / `instrument.cpp` | Instrument registry. Uses include guards. **You implement it** |
| `book.h` / `book.cpp` | OrderBook. Uses `#pragma once` and a **forward declaration**. You implement `price_to_ticks` |
| `impl.cpp` | Tests and the Part B "break it" lab |
| `exercises.cpp` + `odr_other.cpp` | Output questions. **Two TUs on purpose** |

> **MSYS2 tip:** sometimes the linker prints only `collect2.exe: error: ld returned 1 exit status` without the real reason.
> When that happens, compile with `-c` and look at the symbols: `nm -C *.o | grep <name>`. Two `T` lines for the same symbol means a multiple definition, and one `U` with no `T` anywhere means an undefined reference.

---

## 0. Warm-up (Day 01)
1. `nm main.o` shows `U _Z9mid_pricedd`. What does `U` mean, and which stage must resolve it?
2. Why does `g++ -L. -lpricing main.o` fail with GNU ld, while `g++ main.o -L. -lpricing` works?
3. Why is `(long long)(100.07 / 0.01)` a bug in a trading system?

<details><summary>Answers</summary>

1. **Undefined**: this object file uses the symbol but doesn't define it. The **linker** must find exactly one definition in another `.o` or a library.
2. GNU ld scans left to right and only pulls archive members for symbols that are **already** undefined. When `-lpricing` is scanned, nothing is needed yet, so nothing is pulled.
3. `100.07 / 0.01` is `10006.999…` in binary floating point, and the cast **truncates** it to 10006. You send an order at the wrong price. Use `std::llround`, and keep prices as integer ticks internally.

> Day 01 `impl.cpp` isn't implemented yet. Finish it alongside today's work, since today's `price_to_ticks` reuses the same idea.
</details>

---

## 1. Theory

### 1.1 Translation unit (TU)
**TU = one `.cpp` file + everything `#include`d into it, after preprocessing.** The compiler compiles each TU in isolation and knows nothing about the others. The linker is the first tool that sees the whole program.

### 1.2 Declaration vs definition
- A **declaration** introduces a name and its type: "this exists somewhere".
- A **definition** is a declaration that also *creates* the thing: it allocates storage, provides a body, or gives a complete class.

| Code | Declaration | Definition? |
|------|------------|-------------|
| `int f(int);` | ✔ | ✘ |
| `int f(int x) { return x; }` | ✔ | ✔ |
| `extern int g;` | ✔ | ✘ |
| `int g;` / `int g = 5;` | ✔ | ✔ (storage allocated) |
| `extern int g = 5;` | ✔ | ✔ (**the initializer makes it a definition!**) |
| `struct S;` | ✔ (forward declaration, incomplete type) | ✘ |
| `struct S { int a; };` | ✔ | ✔ (class definition) |
| `static int count;` inside a class | ✔ | ✘ (needs an out-of-class definition, or `inline`) |
| `static inline int count = 0;` inside a class (C++17) | ✔ | ✔ |
| `using Price = long long;` | ✔ | n/a (an alias, creates nothing) |

You can declare something any number of times. Definitions are limited by the ODR.

### 1.3 The One Definition Rule (ODR)
1. **Per TU**: at most **one** definition of any variable, function, class, enum or template.
2. **Per program**: exactly **one** definition of every non-inline function or variable that is **odr-used** (address taken, called, or bound to a reference).
3. **Exceptions**: classes, enums, `inline` functions and variables, templates, and `constexpr` functions may be defined in **every TU that needs them**, as long as **all definitions are token-for-token identical** (and mean the same thing).

Breaking rule 1 gives a **compiler** error (`redefinition`). Breaking rule 2 gives a **linker** error (`multiple definition` / `undefined reference`). Breaking rule 3 (different definitions in different TUs) is **undefined behaviour, and no diagnostic is required**. That is the dangerous one.

### 1.4 What `inline` means in modern C++
`inline` is mostly **not** about inlining anymore. It means **"this definition may appear in multiple TUs; the linker keeps one."** The compiler decides on actual inlining by itself.
- These are implicitly `inline`: member functions defined inside the class body, `constexpr` functions, and `static constexpr` data members (C++17).
- **Inline variables (C++17):** `inline int x = 0;` or `inline constexpr double kTick = 0.01;` in a header give **one shared object** across all TUs.

### 1.5 Headers: what goes where
| Header (`.h`) | Source (`.cpp`) |
|---------------|-----------------|
| class definitions, function declarations | non-inline function definitions |
| `inline` functions/variables, templates, `constexpr` | non-inline variable definitions (the one `extern`'s target) |
| `extern` variable declarations | file-private helpers (`static` / anonymous namespace, Day 03) |
| type aliases, enums, forward declarations | `using namespace` (if you must) |

**Never** put these in a header: non-inline function or variable definitions, or `using namespace std;`, which leaks into every includer.

### 1.6 Include guards vs `#pragma once`
```cpp
#ifndef PROJ_INSTRUMENT_H      // classic: works everywhere, guaranteed by the standard
#define PROJ_INSTRUMENT_H
...
#endif
```
```cpp
#pragma once                   // non-standard, but supported by GCC/Clang/MSVC
```
Both only prevent **double inclusion within one TU**. They do **nothing** across TUs, which is why `int x = 5;` in a guarded header still breaks the link.

### 1.7 Forward declarations and incomplete types
After `struct Instrument;` the type is **incomplete**. Here is what you can and can't do with it:

| ✔ Allowed with an incomplete type | ✘ Needs the complete type |
|---------------------------------|-------------------------|
| declare `Instrument*` / `Instrument&` | `sizeof(Instrument)`, a by-value member |
| declare functions taking or returning it | access members (`p->tick_size`) |
| copy and store pointers | create objects, `new Instrument` |
| | inherit from it, `delete p` (UB if the destructor is non-trivial!) |

Why it matters: fewer includes means **faster builds** and **fewer rebuilds** when a header changes, and it **breaks circular includes**.

---

## 2. Internals

### 2.1 What the compiler sees for `impl.cpp`
```
impl.cpp ──#include "instrument.h"──► pastes <string>, <string_view> (~25k lines), struct Instrument, ...
         ──#include "book.h"────────► struct Instrument;  class OrderBook {...}   (tiny, thanks to the fwd decl)
         ──#include <iostream> ...  ──► ...
         = ONE translation unit, compiled alone into impl.o
```
Check it yourself: `g++ -std=c++20 -E impl.cpp | wc -l`, and `g++ -H -c impl.cpp` prints the include tree.

### 2.2 How each kind of definition looks to the linker (`nm -C`)
| Source | In each TU's `.o` | Linker behaviour |
|--------|-------------------|------------------|
| `int f() {...}` in a `.cpp` | `T f()` once | fine |
| `int f() {...}` in a header included by 2 TUs | `T f()` **twice** | ❌ multiple definition |
| `inline int f() {...}` in a header | `W f()` (weak / COMDAT) in each TU | keeps **one**, discards the rest |
| `inline int x = 0;` | `u x` (unique global) in each TU | merged into **one** object |
| `const int k = 100;` at namespace scope | `r k` / local or optimized away | **each TU has its own copy** (internal linkage) |
| `extern int g;` | `U g` | must match one `D g` / `B g` elsewhere |

> **Your toolchain (MinGW/COFF) differs from Linux/ELF.** There, `nm -C` shows an inline function as `T fee_bps()` in **both** objects too. The difference is an extra line, `t .text$_Z7fee_bpsv`: the function lives in its own **COMDAT section** (`.text$<mangled name>`), and the linker deduplicates those. A plain non-inline function sits in the ordinary `.text` with no `.text$...` line, so two of them clash. Inline variables likewise show up as `R kInlineLimit` alongside an `.rdata$kInlineLimit` section.

### 2.3 Why "different inline bodies" is silent UB
```
exercises.o:  W fee_bps() { return 1; }     odr_other.o:  W fee_bps() { return 2; }
                         \                             /
                          linker: "same name, both weak, keep the first one"
```
- At **-O0** every call goes to the one surviving copy, so both TUs get the *same* answer, whichever copy the link order picked.
- At **-O2** each TU may **inline its own** body, so the TUs disagree.
You'll see both in Q3. The linker never compares bodies: it only sees names. GCC's `-flto -Wodr` catches some **type/layout** mismatches across TUs, but not differing inline function bodies.

### 2.4 How `#pragma once` and include guards actually work
- **Guards:** the preprocessor still opens the file the second time, sees that `#ifndef` is false, and skips to `#endif`. GCC optimizes this: it remembers guarded files and doesn't even reopen them.
- **`#pragma once`:** the compiler records the file's identity (path, inode or contents) and refuses to include it again. It can fail with the same file reached via symlinks, network shares, or two copies of a header. Guards can fail when two headers accidentally use the **same macro name**.

---

## 3. Implementation

### Part A: implement (tests are in `impl.cpp`)
| What | Where | Requirements |
|------|-------|--------------|
| `whole_lots(qty, lot_size)` | `instrument.h` (inline) | complete lots in `qty`; `0` if `lot_size <= 0` |
| table storage | `instrument.cpp` | fixed capacity `kMaxInstruments`, **no heap growth**. A `static std::array<Instrument, kMaxInstruments>` plus a count works |
| `register_instrument` | `instrument.cpp` | `false` on a duplicate symbol or a full table; stores a copy |
| `find_instrument` | `instrument.cpp` | `++g_lookup_count` on **every** call; returns a stable pointer or `nullptr` |
| `registered_count` | `instrument.cpp` | |
| `OrderBook::price_to_ticks` | `book.cpp` | round to the nearest tick using the instrument's `tick_size` |

```
g++ -std=c++20 -O2 -Wall -Wextra -pedantic impl.cpp instrument.cpp book.cpp -o impl && ./impl
```
Hints: compare `std::string` with `std::string_view` directly (`==` works). Pointers into a `std::array` never move, but pointers into a `std::vector` would be invalidated when it grows. Remember that for Day 33.

### Part B: break it on purpose (record each result at the bottom of `impl.cpp`)
Restore the file after each step.
1. Add a second `#include "instrument.h"` in `impl.cpp`. Builds fine? Now delete the `#ifndef/#define/#endif` guard. Error? **Which stage?**
2. **Move** the definition: delete `long long g_lookup_count = 0;` from `instrument.cpp` and change the header line to `long long g_lookup_count = 0;`. Which stage complains? (Bonus: leave it in **both** places. Why is that a *different* error?)
3. Remove `inline` from `whole_lots`. Compile with `-c` and run `nm -C *.o | grep whole_lots`, **before and after** removing it. Both show `T` in several objects on MinGW. What's the extra line in the inline version? *(Hint: the note under §2.2.)*
4. Remove `inline` from `kMaxInstruments` (leave `constexpr`). Does it still link? Why? *(Hint: §2.2, `const` at namespace scope.)*
5. In `book.h`, try defining `price_to_ticks` inline in the class using `instrument_->tick_size`. Read the error about the **incomplete type**.
6. Replace `struct Instrument;` in `book.h` with `#include "instrument.h"`, then compare `g++ -std=c++20 -E book.cpp | wc -l` for both versions. Now imagine 200 files include `book.h`.

---

## 4. Output Questions
Predict, then run **both** builds (the header of `exercises.cpp` has the commands). Don't open `odr_other.cpp` before predicting.

**Q1**
```cpp
int never_defined();     // no definition anywhere
std::cout << sizeof(never_defined());
```
Does it link? What does it print?

**Q2**: in **both** TUs:
```cpp
const int kLimit = 100;
inline const int kInlineLimit = 100;
```
```cpp
std::cout << (&kLimit == other_klimit_addr()) << ' ' << (&kInlineLimit == other_kinline_addr());
```

**Q3**: `exercises.cpp` has `inline int fee_bps() { return 1; }`, `odr_other.cpp` has `inline int fee_bps() { return 2; }`, and `other_fee()` (in odr_other) returns `fee_bps()`.
```cpp
std::cout << fee_bps() << ' ' << other_fee();
```
Predict for `-O0` **and** `-O2`. Bonus: at `-O0`, swap the file order on the command line.

**Q4**
```cpp
int twice(int);
int twice(int);
int twice(int x) { return 2 * x; }
std::cout << twice(21);
```

**Q5**: `extern int counter;` here, `int counter = 7; int bump() { return ++counter; }` in odr_other:
```cpp
std::cout << counter << ' ' << bump() << ' ' << counter;
```

<details><summary>Answers</summary>

1. **Links, prints `4`.** `sizeof` doesn't evaluate its operand, so `never_defined` is not **odr-used** and needs no definition. Only the return type matters.
2. **`0 1`.** A namespace-scope `const` (non-`inline`, non-`extern`) has **internal linkage**: each TU has its own copy at a different address. An `inline` variable is **one object** program-wide.
3. **`-O0`: `1 1`** (the linker kept the first copy, from exercises.o). **Swapped order: `2 2`.** **`-O2`: `1 2`**: each TU inlined its own body. Same source, three different behaviours. This is the ODR "no diagnostic required" UB.
4. **`42`.** Repeated declarations are fine. Only one definition is allowed.
5. **`7 8 8`.** `extern` refers to the one `counter` defined in the other TU. Since C++17 the operands of `<<` are evaluated **left to right**, so before C++17 this was unspecified.
</details>

---

## 5. Debugging
Say **what's wrong**, **which stage** catches it, and the **fix**.

**B1**
```cpp
// risk.h
#pragma once
double max_notional = 5'000'000.0;
```
included by `risk.cpp` and `strategy.cpp`.

**B2**
```cpp
// order.h                         // fill.h
#pragma once                       #pragma once
#include "fill.h"                  #include "order.h"
struct Order { Fill last_fill; };  struct Fill { Order* parent; };
```

**B3**: run `exercises.cpp` with `-DBUG3`
```cpp
// stats.h
static int g_orders = 0;    // "shared" order counter
```
Both TUs increment it. Expected `here=2 other=2`.

**B4**
```cpp
// fees.h
extern double fee_rate = 0.0002;
```

**B5**
```cpp
// lib.cpp compiled with -DDEBUG_BOOK, main.cpp without it:
struct Level {
    long long price;
    int qty;
#ifdef DEBUG_BOOK
    long long debug_ts;
#endif
};
```
`main.cpp` builds a `std::vector<Level>` and passes it to a function in `lib.cpp`.

<details><summary>Answers</summary>

- **B1. Linker**: `multiple definition of max_notional`. `#pragma once` only works within one TU. Fix: `inline double max_notional = …;` (C++17), or `extern` in the header plus one definition in `risk.cpp`, or `inline constexpr` if it's constant.
- **B2. Compiler.** Whichever header is included first, the other sees the guard already set, so one struct is used before it's declared. Fix: `fill.h` only needs a **pointer** to `Order`, so replace `#include "order.h"` with `struct Order;`. `Order` holds a `Fill` **by value**, so it needs the full include.
- **B3. Nobody, it's silent.** `static` gives **internal linkage**: each TU gets its *own* `g_orders`, so the output is `here=1 other=1`. Fix: `inline int g_orders = 0;` (and in real code, make it thread-safe or a member of a class).
- **B4. Linker** (once two TUs include it): `extern` **with an initializer is a definition**. Fix: `extern double fee_rate;` in the header, `double fee_rate = 0.0002;` in one `.cpp`.
- **B5. Nobody: silent memory corruption.** The two TUs disagree on `sizeof(Level)`: 16 bytes vs 24 (8 + 4 + 4 bytes of padding + 8), so `lib.cpp` reads every element at the wrong offset. This is an ODR violation of the class definition. It happens in real life with `NDEBUG`/debug-only members and with libraries built with different flags. Fix: never let macros change a layout in a shared header, and build everything with identical flags. `-flto -Wodr` can sometimes catch it.
</details>

---

## 6. Performance (the HFT angle)
| Topic | What matters |
|-------|--------------|
| **Build time** | Every `#include` is re-preprocessed and re-parsed **in every TU**. `<string>` alone is about 25k lines. Forward-declare in headers, include the full header only in `.cpp` files, and use `<iosfwd>` instead of `<iostream>` in headers. Check with `g++ -H` (include tree) or Clang's `-ftime-trace`. |
| **Rebuild cascade** | When a header changes, every TU that (transitively) includes it is recompiled. A widely-included header with heavy includes turns a one-line change into a full rebuild. |
| **Inlining vs separation** | A tiny function **defined in a header** can be inlined into the hot loop. **Declared** in a header but defined in a `.cpp`, it can't be (without LTO): you pay a call, and you lose vectorization across the call. Hot, tiny functions (price conversions, comparisons) belong in headers. Big or cold ones belong in `.cpp` files. |
| **Header-only libraries** | Easy to use and fully inlinable, but they lengthen compile time for every includer. Templates have to live in headers anyway. Use `extern template` to stop re-instantiating the same template in every TU. |
| **`const` vs `inline constexpr` in headers** | `const`/`constexpr` at namespace scope gives every TU its **own copy**, which costs a little binary size if the address is taken. `inline constexpr` gives one copy. That rarely matters for speed, but it matters for identity: Q2. |
| **Fixed-capacity storage** | Today's registry uses a `std::array`: no heap allocation, stable pointers, contiguous and cache-friendly. That is a typical HFT choice over a growable `std::vector` or `std::map`. |

---

## 7. Interview Questions
<details><summary>1. What is a translation unit?</summary>A source file after preprocessing, with all its includes pasted in. It is the unit the compiler compiles in isolation into one object file.</details>
<details><summary>2. Declaration vs definition, with an example of each for a variable, a function and a class.</summary>Variable: `extern int x;` vs `int x = 1;`. Function: prototype vs a function with a body. Class: `struct S;` (forward declaration) vs `struct S { ... };`. A definition creates the entity (storage, body or complete type).</details>
<details><summary>3. State the One Definition Rule.</summary>At most one definition per TU. Exactly one definition program-wide for each odr-used non-inline function or variable. Classes, inline entities and templates may be defined in multiple TUs if the definitions are identical.</details>
<details><summary>4. What does `inline` mean in modern C++?</summary>It allows the definition in multiple TUs, and the linker merges them into one. It's only a weak hint for actual inlining.</details>
<details><summary>5. What are inline variables (C++17) and why were they added?</summary>`inline T x = ...;` in a header gives one object across all TUs. Before C++17 you needed `extern` in the header plus a definition in a `.cpp`, which made header-only libraries with globals or static members awkward.</details>
<details><summary>6. Include guards vs #pragma once?</summary>Both stop double inclusion within a TU. Guards are standard but risk macro-name collisions. `#pragma once` is non-standard but universally supported and shorter, and can misbehave with symlinks or duplicated files.</details>
<details><summary>7. Do include guards prevent multiple-definition linker errors?</summary>No. They only work inside one TU. Each TU still gets its own copy of any non-inline definition in the header.</details>
<details><summary>8. What can and can't you do with an incomplete type?</summary>Can: declare pointers and references, and declare functions using it. Can't: sizeof, by-value members, member access, construction, inheritance, and safe delete.</details>
<details><summary>9. Why use forward declarations?</summary>Faster builds, fewer rebuild cascades, and they break circular include dependencies.</details>
<details><summary>10. Why are templates usually defined in headers?</summary>The compiler needs the full definition at the point of instantiation in each TU. Template instantiations are emitted as weak/COMDAT symbols and deduplicated. (The alternative is explicit instantiation in a `.cpp`.)</details>
<details><summary>11. What's wrong with `using namespace std;` in a header?</summary>It injects every `std` name into every file that includes the header, transitively. That causes ambiguities and silent overload changes that the includer can't undo.</details>
<details><summary>12. What linkage does `const int k = 5;` at namespace scope have?</summary>Internal linkage in C++ (unlike C), so every TU has its own copy. `inline` or `extern` gives it external linkage.</details>
<details><summary>13. Is `extern int x = 5;` a declaration or a definition?</summary>A definition: the initializer makes it one, and `extern` is effectively ignored. In a header it causes multiple definitions.</details>
<details><summary>14. Can a function be declared and used without ever being defined?</summary>Yes, if it's never odr-used: for example only inside `sizeof`, `decltype` or an unevaluated context, or in an unused overload. If it's called, the linker needs it.</details>
<details><summary>15. What happens if two TUs define the same class differently?</summary>An ODR violation, and undefined behaviour with no diagnostic required. Layouts and inline functions can silently mismatch, which leads to memory corruption.</details>

---

## 8. Hard Questions
<details><summary>H1. A trading library works in Debug and corrupts data in Release, only when linked into one particular app. The struct in question has a member guarded by #ifdef. Explain and fix.</summary>

The app and the library were compiled with different macros (`NDEBUG`, `DEBUG_BOOK`, or different `-D` flags), so they disagree on `sizeof`, member offsets, and the bodies of inline functions. Every `T` passed across the boundary, and every inline member function, is misinterpreted. This is an ODR violation, so there's no diagnostic. Fixes: no layout-affecting macros in public headers; identical build flags across the whole program (CMake target-wide definitions); ABI checks (`static_assert(sizeof(Level) == 16)` in the header); `-flto -Wodr`; or put the debug data in a separate side structure.
</details>

<details><summary>H2. `struct S { static const int N = 10; };` and `void f(const int&); f(S::N);`. This links at -O2 but fails at -O0 with "undefined reference to S::N". Why? How do you fix it properly?</summary>

`static const int N = 10;` inside the class is only a **declaration** with an initializer (for integral const it can be used in constant expressions). Binding it to `const int&` **odr-uses** it, which needs an actual object with an address, and therefore a definition (`const int S::N;` in one `.cpp`). At -O2 the compiler inlines `f` or constant-folds the value and never emits a reference to the symbol, so the missing definition goes unnoticed. The program is still ill-formed (no diagnostic required). Fix: `static constexpr int N = 10;` (implicitly `inline` since C++17) or `static inline const int N = 10;`.
</details>

<details><summary>H3. Your 2-million-line codebase takes 25 minutes to build, and changing one widely-included header rebuilds everything. What do you do?</summary>

Measure first: `-H`/`-ftime-trace`, ClangBuildAnalyzer, and include-what-you-use. Then:
- Forward-declare instead of including, and use `<iosfwd>`.
- Use **PImpl** to hide implementation details behind a pointer (stable header, fewer rebuilds).
- Split "god headers".
- Use `extern template` / explicit instantiation for heavy templates.
- Precompiled headers for stable third-party headers, and unity/jumbo builds.
- ccache/sccache and distributed builds.
- C++20 **modules** in the long run.
Trade-off for HFT: PImpl adds a pointer indirection and blocks inlining, so keep it off hot paths.
</details>

<details><summary>H4. When would you deliberately define a function in a header rather than a .cpp, and when not?</summary>

Header (inline, constexpr or a template): small, hot functions that must be inlined across TUs (price/tick math, comparators, accessors), templates, and header-only libraries. `.cpp`: large or cold code (error paths, setup, logging formatting), code with heavy dependencies, anything that would bloat compile times or the binary, and code you want to change without rebuilding every includer. LTO blurs the line: it can inline across TUs without moving code into headers.
</details>

---

## 9. Mini Project: header hygiene on your backtester
Pick one module from your C++ backtester (e.g. `Order`, `Trade`, `Strategy`, `Portfolio`) and:
1. **Measure:** for each `.cpp` that includes it, record `g++ -std=c++20 -E file.cpp | wc -l` and the compile time (`time g++ -c ...`).
2. **Refactor its header:** add guards or `#pragma once`; remove `using namespace`; replace includes with forward declarations wherever only pointers or references are used; move non-trivial function bodies into the `.cpp`; keep tiny hot ones `inline`; turn global constants into `inline constexpr`; and remove any non-inline variable definitions (`extern` + one definition).
3. **Audit with `nm -C`:** make sure no symbol is `T` in more than one object file, and that globals you meant to share aren't secretly `static`/`const` copies (B3/Q2).
4. **Measure again** and write the before/after numbers in a comment in `impl.cpp`.

No backtester at hand? Do the same with today's `book.h`: make it include `<map>`, `<vector>`, `<string>` and `<iostream>` "for convenience", measure, then strip it back down to the forward declaration.

---

## Checklist
- [ ] read theory + internals
- [ ] `impl.cpp` passes all tests (Part A)
- [ ] Part B: all 6 breakages done, observations written
- [ ] exercises: predicted Q1–Q5 (both -O0 and -O2 for Q3) + B3 before running
- [ ] mini project: before/after numbers
- [ ] (carry-over) Day 01 `impl.cpp` passing
