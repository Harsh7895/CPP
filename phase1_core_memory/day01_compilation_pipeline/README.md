# Day 01 — Compilation Pipeline: Preprocess → Compile → Assemble → Link
> Phase 1 · Est. time 60–90 min · Builds on: — (first day)

Files in this folder:
| File | Purpose |
|------|---------|
| `pricing.h` / `pricing.cpp` | A tiny "pricing library": **you implement it** |
| `impl.cpp` | The driver plus tests. It also includes the pipeline tasks |
| `exercises.cpp` | Output-prediction and debugging snippets |

> **Toolchain note:** the commands below use GCC/binutils (Linux, WSL, or MinGW on Windows).
> On MinGW the shared-library extension is `.dll` (not `.so`), and you check dependencies with `objdump -p app.exe | grep DLL` instead of `ldd`.

---

## 0. Warm-up (pre-check, since this is Day 1)
1. When you type `g++ main.cpp -o main`, how many separate programs do you think actually run?
2. Your code compiles but the build fails with `undefined reference to 'foo()'`. Is that a compiler error or something else?

<details><summary>Answers</summary>

1. At least **three or four**. `g++` is a *driver*. It runs the preprocessor and compiler (`cc1plus`), then the assembler (`as`), then the linker (`collect2` → `ld`). Add `-v` to see every one of them.
2. It is a **linker** error. Each `.cpp` compiled fine on its own, but when the linker combined the object files, no definition of `foo()` was found.
</details>

---

## 1. Theory

### 1.1 The four stages
```
 main.cpp ──[preprocess]──► main.ii ──[compile]──► main.s ──[assemble]──► main.o ──┐
 pricing.cpp ─────────────────────────── same 3 steps ─────────────────► pricing.o ─┼─[link]──► app
                                                       libstdc++, libc, crt*.o ──────┘
```
| Stage | Tool | Input → Output | Stop here with |
|-------|------|----------------|----------------|
| Preprocess | `cpp` (inside `cc1plus`) | `.cpp` → `.ii` (pure C++ text) | `g++ -E` |
| Compile | `cc1plus` | `.ii` → `.s` (assembly) | `g++ -S` |
| Assemble | `as` | `.s` → `.o` (machine code + metadata) | `g++ -c` |
| Link | `ld` (via `collect2`) | many `.o` + libs → executable | (default) |

### 1.2 Preprocessing = text manipulation, nothing more
- `#include "x.h"` **pastes the whole file text** in place. That is why a 10-line `.cpp` with `#include <iostream>` becomes roughly 30k+ lines in `.ii`.
- `#define` does token substitution. It knows nothing about types, scope or precedence.
- `#if/#ifdef` drops code before the compiler ever sees it.
- The output of this stage is a **translation unit (TU)**: one `.cpp` plus everything it includes. **The compiler only ever sees one TU at a time.**

### 1.3 Compilation proper
For one TU, the compiler parses the code, does semantic analysis (types, overloads, template instantiation), lowers it to an IR (GIMPLE/RTL in GCC, LLVM IR in Clang), optimizes (`-O2`) and emits assembly.
It is fine to call a function the compiler has only seen **declared**. The compiler just emits a call to a *symbol name* and leaves the address for the linker to fill in.

### 1.4 Assembly → object file
`as` turns the assembly into an **object file** (ELF on Linux, COFF/PE on Windows). It contains:
- **Sections**: `.text` (code), `.data` (initialized globals), `.bss` (zero-initialized globals, which take no file space), `.rodata` (constants and string literals)
- A **symbol table**: what this file *defines* and what it *needs*
- **Relocations**: "patch the address at offset X with the address of symbol Y once it is known"

### 1.5 Name mangling
C++ supports overloading, so `f(int)` and `f(double)` need different symbol names. The compiler encodes the signature into the name:
```
double mid_price(double, double)  →  _Z9mid_pricedd
                                       │ │        └┴─ params: double, double
                                       │ └─ name length 9
                                       └─ "this is a mangled C++ name"
```
`extern "C"` turns mangling off, so C code or other languages can call the function. That means it can't be overloaded. Use `nm -C` (or `c++filt`) to demangle.

### 1.6 Linking
The linker does two jobs:
1. **Symbol resolution**: match every *undefined* symbol to exactly one *definition*. Zero matches gives `undefined reference`. Two strong definitions give `multiple definition`.
2. **Relocation**: merge all the `.text` sections into one, all the `.data` sections into one, and so on, assign final addresses, then patch every relocation.

### 1.7 Static linking
- A static library `libfoo.a` is just an **archive of `.o` files** (`ar rcs libfoo.a a.o b.o`).
- The linker copies in only the archive *members* that resolve a currently-undefined symbol.
- **Order matters.** `ld` scans left to right and only pulls from an archive what is *already* needed. So `g++ -lfoo main.o` fails, while `g++ main.o -lfoo` works.
- Result: a self-contained binary that is bigger, has no runtime dependency, and allows LTO and cross-module inlining.

### 1.8 Dynamic linking
- A shared library is `libfoo.so` (Linux) or `foo.dll` (Windows). The code is **not copied** into your binary. The binary records "I need libfoo.so".
- At startup, the **dynamic loader** (`ld-linux.so`) maps the library into memory and resolves symbols.
- Shared libs are compiled with **`-fPIC`** (position-independent code) because they can load at any address.
- Calls into a shared lib go through the **PLT** (procedure linkage table) and **GOT** (global offset table), which adds an indirection. See Internals.
- Pros: smaller binaries, one copy of the library in RAM shared by many processes, and you can update the library without relinking. Cons: indirection, no cross-library inlining, "DLL hell", and startup cost.

### 1.9 What runs before `main`
`_start` (from `crt1.o`) → `__libc_start_main` → runs **static initializers** (constructors of globals, via `.init_array`) → `main()` → `exit()` → `atexit` handlers and static destructors.

---

## 2. Internals

### 2.1 Inside `main.o` before linking
```cpp
// impl.cpp
double m = mid_price(100.0, 100.5);
```
Run `objdump -dr impl.o`:
```
  1a:  e8 00 00 00 00     call   1f <main+0x1f>
                    1b: R_X86_64_PLT32   _Z9mid_pricedd-0x4
```
The call target is **`00 00 00 00`**, a placeholder. The relocation entry says: "at offset `0x1b`, write the (PC-relative) address of `_Z9mid_pricedd`". `nm impl.o` shows `U _Z9mid_pricedd` (**U = undefined, needed from elsewhere**).
`nm pricing.o` shows `T _Z9mid_pricedd` (**T = defined in .text**).

Common `nm` letters: `T/t` code (global/local), `D/d` data, `B/b` BSS, `R/r` read-only data, `U` undefined, `W` weak (inline functions and template instantiations).

### 2.2 Static link
```
 impl.o:   .text[ main ... call ???? ]  needs _Z9mid_pricedd
 pricing.o:.text[ _Z9mid_pricedd ... ]  defines it
                     │
              ld merges .text sections, assigns addresses
                     ▼
 app:      .text[ main ... call 0x401230 ][ 0x401230: mid_price ... ]
```
After this, the call is a direct `call rel32` with zero indirection.

### 2.3 Dynamic link: PLT/GOT
```
 app .text:   call mid_price@plt ─────┐
                                      ▼
 app .plt:    mid_price@plt: jmp *GOT[3] ──┐
                                           ▼
 app .got:    GOT[3] = <address>  ── first call: points back into the resolver
                                  ── after resolution: real address in libpricing.so
```
- **Lazy binding** (the default): the first call goes into the dynamic loader's resolver, which looks up the symbol and patches `GOT[3]`. Later calls are `call` → `jmp *GOT` → function.
- `LD_BIND_NOW=1` or linking with `-Wl,-z,now` resolves everything at startup instead.
- The function may also *not be inlinable* across the library boundary, and with default ELF semantics a symbol in a `.so` can even be **interposed** (replaced by another library's definition, e.g. via `LD_PRELOAD`).

### 2.4 Why headers + inline work
An `inline` function (or template instantiation) can be emitted in many TUs. Each copy is marked **weak/COMDAT** (`W` in `nm`), and the linker keeps just one. A *non-inline* function defined in a header that is included in 2 TUs gives **two strong `T` definitions**, which is a `multiple definition` error.

---

## 3. Implementation

You build a mini **pricing library** split across files, then push it through every stage by hand.

### Part A: implement the functions (`pricing.h`, `pricing.cpp`)
| Function | Where | Spec |
|----------|-------|------|
| `double mid_price(double bid, double ask)` | `pricing.cpp` | `(bid + ask) / 2` |
| `double spread_bps(double bid, double ask)` | `pricing.cpp` | `(ask - bid) / mid * 10'000` |
| `long long to_ticks(double price, double tick_size)` | `pricing.cpp` | Price as an integer number of ticks, **rounded to nearest**. `to_ticks(100.07, 0.01)` must be `10007` (not `10006`!) |
| `inline bool is_crossed(double bid, double ask)` | `pricing.h` | `true` if `bid >= ask` |

Edge cases: think about what `100.07 / 0.01` actually equals in `double`. Hint: `std::llround`.

Build and run the tests:
```
g++ -std=c++20 -O2 -Wall -Wextra -pedantic impl.cpp pricing.cpp -o impl && ./impl
```

### Part B: walk the pipeline by hand (write your observations as comments at the bottom of `impl.cpp`)
```bash
g++ -std=c++20 -E impl.cpp -o impl.ii      # 1. how many lines is impl.ii? (wc -l) Why so many?
g++ -std=c++20 -O2 -S pricing.cpp -o pricing.s   # 2. find mid_price in the asm. How many instructions?
g++ -std=c++20 -O2 -c pricing.cpp -o pricing.o   # 3.
g++ -std=c++20 -O2 -c impl.cpp    -o impl.o
nm impl.o | grep -i price                  # 4. which symbols are U, which are T?
nm -C pricing.o                            # 5. compare with plain `nm pricing.o` (mangled names)
objdump -dr impl.o | grep -A1 call         # 6. find the relocation for mid_price
g++ impl.o pricing.o -o impl               # 7. link
g++ -v impl.o pricing.o -o impl 2>&1 | tail -3   # 8. spot collect2/ld and the crt*.o files
```

### Part C: static vs shared library
```bash
# static
ar rcs libpricing.a pricing.o
ar t libpricing.a                                    # list members
g++ impl.o -L. -lpricing -o app_static               # works
g++ -L. -lpricing impl.o -o app_bad                  # 9. why does this fail? (Linux/GNU ld)

# shared (Linux)
g++ -std=c++20 -O2 -fPIC -shared pricing.cpp -o libpricing.so
g++ impl.o -L. -lpricing -Wl,-rpath,'$ORIGIN' -o app_shared
ldd app_shared                                       # 10. is libpricing.so listed?
# shared (MinGW): g++ -shared pricing.cpp -o pricing.dll && g++ impl.o pricing.dll -o app_shared.exe

ls -l app_static app_shared                          # 11. compare sizes
```
Watch out: when **both** `libpricing.a` and `libpricing.so` exist, `-lpricing` picks the **`.so`**. Rename one, or use `-l:libpricing.a`, to be sure which one you got.

### Part D: break it on purpose
1. Remove `inline` from `is_crossed` in `pricing.h` and rebuild with both `.cpp` files. Read the error. Is it compiler or linker? Put `inline` back.
2. Comment out the definition of `spread_bps` in `pricing.cpp`. Read the error and note which stage reports it.

---

## 4. Output Questions
Predict **before** running `exercises.cpp`.

**Q1**
```cpp
#define SQUARE(x) x * x
std::cout << SQUARE(1 + 2) << '\n';
```
**Q2**
```cpp
#define MAX(a, b) ((a) > (b) ? (a) : (b))
int i = 5, j = 3;
int m = MAX(i++, j);
std::cout << m << ' ' << i << '\n';
```
**Q3**
```cpp
#define STR(x) #x
#define XSTR(x) STR(x)
#define VERSION 42
std::cout << STR(VERSION) << ' ' << XSTR(VERSION) << '\n';
```
**Q4**
```cpp
// FOO is never defined anywhere
#if FOO == 0
    std::cout << "zero\n";
#else
    std::cout << "nonzero\n";
#endif
```
**Q5**
```cpp
#define CAT(a, b) a##b
int xy = 7;
std::cout << CAT(x, y) << '\n';
```
**Q6**: what does the *whole program* print first?
```cpp
struct Init { Init() { std::cout << "global ctor\n"; } };
Init g_init;
int main() { std::cout << "main starts\n"; }
```

<details><summary>Answers</summary>

1. **5**. It expands to `1 + 2 * 1 + 2`. Always parenthesize macro params *and* the whole body.
2. **`6 7`**. It expands to `((i++) > (j) ? (i++) : (j))`. `i++` runs in the condition (5 > 3, i becomes 6), then again in the true branch (yields 6, i becomes 7). Macros evaluate arguments as many times as they appear.
3. **`VERSION 42`**. `#` stringifies the argument *before* macro expansion. The extra level in `XSTR` lets `VERSION` expand to `42` first.
4. **`zero`**. Inside `#if`, an undefined identifier is replaced by `0`. `-Wundef` warns about this.
5. **`7`**. `##` pastes `x` and `y` into the single token `xy`.
6. **`global ctor`**. Static initializers run before `main` (from `__libc_start_main` via `.init_array`).
</details>

---

## 5. Debugging
Find the bug and say **which stage** catches it (preprocessor / compiler / linker / runtime / nobody).

**B1**: `exercises.cpp`, build with `-DBUG1`
```cpp
double vwap(const double* px, const double* qty, int n);   // declared...
std::cout << vwap(p, q, 2);                                 // ...used, never defined
```
**B2**: `exercises.cpp`, build with `-DBUG2`
```cpp
#define SPREAD(ask, bid) ask - bid
double s = 2 * SPREAD(101.0, 100.0);   // expected 2.0
```
**B3**: header used by two `.cpp` files
```cpp
// config.h
#pragma once
int max_position = 1000;          // definition in a header
```
**B4**: build command
```bash
g++ -L. -lpricing main.o -o app
```
**B5**: C library called from C++
```cpp
// legacy.h   (functions compiled by a C compiler in legacy.c)
int checksum(const char* buf, int len);
// main.cpp
#include "legacy.h"
int c = checksum(msg, n);   // undefined reference to `checksum(char const*, int)`
```

<details><summary>Answers</summary>

- **B1**: **Linker**: `undefined reference to vwap(double const*, double const*, int)`. The compiler accepts a declaration. Only the linker notices there's no definition.
- **B2**: **Nobody, it's a silent runtime bug.** It expands to `2 * 101.0 - 100.0 = 102`. Fix: `#define SPREAD(ask, bid) ((ask) - (bid))`, or better, an `inline constexpr` function.
- **B3**: **Linker**: `multiple definition of max_position`. `#pragma once` only protects *within one TU*, not across TUs. Fix: `inline int max_position = 1000;` (C++17), or `extern int max_position;` in the header plus one definition in a `.cpp`, or `constexpr int` (internal linkage) if it never changes.
- **B4**: **Linker**: undefined references. The archive is scanned before `main.o` creates any undefined symbols. Put libraries **after** the objects that use them.
- **B5**: **Linker**. The C compiler emitted the unmangled symbol `checksum`, but C++ looks for `_Z8checksumPKci`. Fix: wrap the declarations in `extern "C" { ... }` (usually guarded with `#ifdef __cplusplus`).
</details>

---

## 6. Performance (the HFT angle)
| Topic | What matters |
|-------|--------------|
| **Cross-TU inlining** | The compiler sees one TU at a time, so a tiny function in another `.cpp` (like `mid_price`) **can't be inlined**. You pay a call per tick. Fixes: put hot small functions in headers as `inline`/templates, or use **LTO** (`-flto`), which lets the linker re-optimize across TUs. |
| **Static vs shared call cost** | A static call is a direct `call rel32`. A shared-library call is `call` → PLT `jmp *GOT`: an extra indirect jump, no inlining, and possible symbol interposition that blocks optimization. Low-latency shops usually **link hot-path code statically**. |
| **Lazy binding jitter** | The *first* call to each shared-lib function goes through the dynamic resolver, which costs microseconds. In a trading system that first call might be your first order. Use `-Wl,-z,now` / `LD_BIND_NOW=1` to resolve at startup, and **warm up** code paths before the market opens. |
| **Visibility** | `-fvisibility=hidden` plus explicit exports shrinks the dynamic symbol table, speeds up loading, and lets the compiler assume no interposition (`-fno-semantic-interposition`). |
| **Binary size / i-cache** | Huge binaries and heavy inlining can thrash the instruction cache. Hot code should be small and contiguous (PGO helps lay it out). |
| **Build time** | Every `#include` is re-parsed in every TU. Keep headers lean, forward-declare, and use precompiled headers. For big projects build time is a real productivity cost. |
| **Static init** | Global constructors run before `main` in an **unspecified order across TUs** (the "static initialization order fiasco", Day 14). Keep them trivial. |

---

## 7. Interview Questions
<details><summary>1. What are the stages of turning a .cpp into an executable?</summary>Preprocess (`-E`) → compile to assembly (`-S`) → assemble to object (`-c`) → link. `g++` is a driver that invokes each tool.</details>
<details><summary>2. What is a translation unit?</summary>One source file after preprocessing, with all includes pasted and macros expanded. It is the unit the compiler works on.</details>
<details><summary>3. What does #include actually do?</summary>Textually copies the file's contents into the including file. There is no module system involved (pre-C++20 modules).</details>
<details><summary>4. Compiler error vs linker error: give an example of each.</summary>Compiler: syntax or type error, use of an undeclared name. Linker: `undefined reference` (declared but never defined, or a missing library) and `multiple definition` (an ODR violation such as a non-inline function defined in a header).</details>
<details><summary>5. What is inside an object file?</summary>Machine code and data in sections (.text/.data/.bss/.rodata), a symbol table (defined and undefined symbols), relocation entries, and optionally debug info.</details>
<details><summary>6. What is name mangling and why does C++ need it?</summary>It encodes namespace, class, function name and parameter types into the symbol name so overloads and namespaces get unique linker symbols. Example: `_Z9mid_pricedd`.</details>
<details><summary>7. What does extern "C" do?</summary>Gives the function C language linkage: no mangling, so it can be called from C or other languages or loaded via `dlsym`. Such a function can't be overloaded.</details>
<details><summary>8. Static vs dynamic linking: trade-offs?</summary>Static: self-contained, fast direct calls, LTO possible, bigger binary, must relink to update. Dynamic: shared memory pages, updatable libraries, smaller binaries, but PLT indirection, loader cost, and version issues.</details>
<details><summary>9. What is a .a file?</summary>An `ar` archive of `.o` files plus an index. The linker extracts only the members that satisfy currently-undefined symbols.</details>
<details><summary>10. Why does library order matter on the command line?</summary>GNU ld processes inputs left to right and only pulls archive members for symbols that are already undefined at that point. Libraries go after the objects that use them.</details>
<details><summary>11. What is -fPIC and why do shared libraries need it?</summary>Position-independent code: addresses go through PC-relative addressing and the GOT, so the code works wherever the loader maps it, and code pages can be shared between processes.</details>
<details><summary>12. If libfoo.a and libfoo.so both exist, what does -lfoo link?</summary>The `.so` by default. Use `-static`, `-Wl,-Bstatic`, or `-l:libfoo.a` to force static.</details>
<details><summary>13. What are the PLT and GOT?</summary>The GOT holds the runtime addresses of external symbols. The PLT holds small stubs that jump through the GOT, which enables lazy binding of shared-library functions.</details>
<details><summary>14. How do you see which shared libraries a binary needs?</summary>`ldd app` or `readelf -d app | grep NEEDED` (Linux). On Windows, `objdump -p app.exe | grep DLL` or Dependencies.exe.</details>
<details><summary>15. What does nm show, and what do T, U and W mean?</summary>The symbol table. T is defined in text, U is undefined (needed), W is weak (inline functions and templates, deduplicated by the linker).</details>

---

## 8. Hard Questions
<details><summary>H1. Two static libraries depend on each other (libA needs libB and libB needs libA). How do you link them, and why is it needed?</summary>

A single left-to-right pass can't satisfy a cycle. After libB is scanned, new undefined symbols pointing back into libA won't be resolved. Options:
- Repeat a library: `main.o -lA -lB -lA`.
- Group them: `-Wl,--start-group -lA -lB -Wl,--end-group` makes ld rescan the group until no new symbols are resolved. This is slower, but correct.
- Best: break the cycle in the design.
(LLD and MSVC's linker don't have this ordering problem by default.)
</details>

<details><summary>H2. Walk through exactly what happens on the first and second call to a function in a shared library. What are the latency implications in a trading system?</summary>

1st call: `call foo@plt` → the PLT stub does `jmp *GOT[n]`, and GOT[n] initially points back into the PLT, which pushes a relocation index and jumps to `_dl_runtime_resolve` → the loader hashes the symbol name and searches each loaded library's symbol table → writes the real address into GOT[n] → jumps to `foo`. This takes microseconds and touches cold memory.
2nd call: `call` → `jmp *GOT[n]` → `foo`. That's one extra indirect jump, which is usually well predicted but still can't be inlined.
Implications: the first-call latency spike can land on the first order of the day. Mitigate with `-z now`/`LD_BIND_NOW`, `-fno-plt`, static linking of the hot path, and warm-up calls before the open.
</details>

<details><summary>H3. What happens between the OS starting your process and main() running, and after main returns?</summary>

The kernel maps the ELF, and for dynamic binaries starts at the dynamic loader. The loader maps the needed `.so` files, applies relocations, and runs each library's initializers. Then `_start` (crt1.o) → `__libc_start_main` sets up libc and runs `.init_array` (global constructors of *your* TUs, in an unspecified order across TUs) → `main(argc, argv, envp)`. When main returns, its value goes to `exit()` → `atexit` handlers and static destructors run in reverse order of construction → stdio buffers are flushed → `_exit` syscall.
</details>

<details><summary>H4. Why can a function template be fully defined in a header included by 100 .cpp files without "multiple definition" errors, while a normal function can't?</summary>

Templates and `inline` functions have *vague linkage*. Each TU that instantiates them emits a copy into a COMDAT group (a weak symbol), and the linker keeps one and discards the rest. The ODR allows multiple definitions **as long as they are token-for-token identical**. A normal function emits a strong symbol in each TU, which gives duplicate strong symbols and a link error. A trap: if two TUs see *different* definitions (different macros, for example), the linker silently picks one. That is an ODR violation, which is undefined behaviour with no diagnostic.
</details>

---

## 9. Mini Project: does linking style matter for a tick loop?
In `impl.cpp`, after the tests, write a small benchmark:
1. Generate 10 million synthetic `(bid, ask)` quotes in a `std::vector` (random walk around 100.00, tick 0.01).
2. Loop over them computing `mid_price` and `spread_bps`, and accumulate a checksum so the optimizer can't delete the loop.
3. Time it with `std::chrono::steady_clock` and report **ns per quote**.
4. Build and compare 4 variants:
   - `app_static` (separate TU, no LTO)
   - `app_shared` (`.so`/`.dll`)
   - LTO: `g++ -O2 -flto impl.cpp pricing.cpp`
   - Header-only: move the bodies into `pricing.h` as `inline`
5. Write down the 4 numbers and **explain** the differences using what you learned: function call cost, PLT indirection, and inlining enabling vectorization.

---

## Checklist
- [ ] read theory + internals
- [ ] `impl.cpp` passes all tests (Part A)
- [ ] Parts B–D done, observations written as comments
- [ ] exercises.cpp: predicted every output before running
- [ ] mini project: 4 timings + explanation
