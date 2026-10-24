# Day 13 — Object lifetime: creation & destruction, temporaries, lifetime extension
> Phase 1 · Est. time 40–45 min (paired with Day 12) · Builds on: Day 03 (storage duration), Day 07 (`const&` binds temporaries), Day 08 (dangling), Day 11 (placement new, explicit destructor calls)

## 🎯 Today in plain words
Every object has a **birth** (its constructor finishes) and a **death** (its destructor runs). C++ promises **exactly when** both happen, and that promise is what RAII (Day 22) is built on.
Today you build a **lifetime tracer**: a class that logs `+name` when it's born and `-name` when it dies. Then you watch the rules happen: locals die in reverse order, temporaries die at the `;`, a `const&` can keep a temporary alive, and returning an object usually makes **no copy at all**.

Why a trader cares: locks, file handles, order "in-flight" guards and timers are all tied to lifetimes. The nastiest dangling bugs (Day 08) come from temporaries dying earlier than you think. One of them, the range-for over `make_book().levels()`, is legal-looking C++20 code that reads freed memory.

Files: `impl.cpp` (the tracer + factories + member order + a safe loop) and `exercises.cpp`.

---

## 📺 Watch first

| # | Video | Length | What to watch for |
|---|-------|--------|-------------------|
| 1 | [Object Lifetime: From Start to Finish, Thamara Andrade, CppCon 2023](https://isocpp.org/blog/2023/09/cppcon-2023-object-lifetime-from-start-to-finish-thamara-andrade) (the page links the video; it's a beginner-level talk) | about 1 hr (watch the first 35–40 min) | When objects start and end, temporaries, and destruction order. Covers §1.1–1.5. |

**Not covered well by the video:** copy elision rules (§1.6), the C++20 range-for trap (§1.7, BUG1) and `string_view` dangling (BUG3). Read those carefully.

---

## 0. Warm-up (Days 11–12)
1. *(Day 11)* How do you destroy an object you created with placement new?
2. *(Day 12)* Why can't the compiler reorder your struct's members to remove padding?
3. *(Day 07)* `const Price& best = better_bid(295000, 295035);`: why does this dangle, when `const Price& r = 100 + 1;` doesn't?

<details><summary>Answers</summary>

1. Call the destructor yourself, `p->~T();`, then release the memory the way you got it (never `delete`).
2. C++ promises that members are laid out in **declaration order**. Today you'll see they're also **constructed** in that order.
3. `r` binds **directly** to the temporary `100 + 1`, so its lifetime is extended. `best` binds to the reference a function **returned**. The temporaries `295000` and `295035` die at the `;`. Extension doesn't pass through functions (§1.4).
</details>

---

## 1. Theory

### 1.1 Lifetime in one sentence
**In one line:** an object's lifetime starts when its constructor **finishes**, and ends when its destructor **starts**.

```cpp
{
    Tracer x("x");      // +x   (born)
    Tracer y("y");      // +y
}                       // -y -x  (die in REVERSE order of birth)
```
**Analogy:** a stack of plates. The last one put on is the first one taken off. That's why the reverse order is guaranteed: `y` may depend on `x`, so `y` must go first.

| Storage (Day 03) | Born | Dies |
|---|---|---|
| automatic (local) | when the definition is reached | at the closing `}` of its scope, in reverse order |
| static / global | before `main` (or at the first call, for a static local) | after `main` returns, in reverse order |
| dynamic (`new`) | at `new` | at `delete` (or never: a leak) |
| temporary | while evaluating an expression | **at the end of the full expression** (the `;`) unless extended |

### 1.2 Temporaries die at the end of the full expression
**In one line:** a **temporary** (an unnamed object, like `Tracer("tmp")` or the result of `a + b`) is destroyed when the whole statement it's part of finishes.

```cpp
Tracer("tmp");          // +tmp -tmp   ← dies right here, at the ;
g_log.add("after");     // after
```
```cpp
use(Tracer("a"), Tracer("b"));   // both live until use() returns, then die
// your g++: +b +a use -a -b    ← argument order is UNSPECIFIED (g++ built b first)
```
Two lessons: temporaries live **through** the whole call, and **never** rely on the order of function arguments.

### 1.3 Lifetime extension: a `const&` (or `&&`) can keep a temporary alive
**In one line:** binding a temporary **directly** to a `const T&` or `T&&` local extends the temporary's life to the reference's life.

```cpp
{
    const Tracer& r = Tracer("ext");   // +ext   (would have died at the ;, but now lives)
    g_log.add("after");                // after
}                                      // -ext   (dies with r)
```

### 1.4 …but NOT through a function returning a reference
```cpp
const T& pass(const T& t) { return t; }
const T& r = pass(T("p"));   // +p -p   ← the temporary dies at the ;. r dangles (BUG2)
```
| Code | Extended? |
|---|---|
| `const T& r = T("x");` | ✅ bound directly |
| `const T& r = make_named("x");` (returns `T` **by value**) | ✅ the returned temporary is bound directly |
| `const T& r = pass(T("x"));` (returns `const T&`) | ❌ the function's reference isn't "the temporary" |
| `const T& r = obj.member_function_returning_ref();` on a temporary `obj` | ❌ (the range-for trap, §1.7) |
| A member `const T&` set in a constructor from a temporary | ❌ |

**Rule:** extension only works when you bind **the temporary itself**, not something a function hands back.

### 1.5 Members: built in declaration order, destroyed in reverse
**In one line:** members are constructed **in the order they're declared in the class**, before the constructor body runs. The order you write the init list doesn't matter. They're destroyed in reverse, after the destructor body.

```cpp
struct Strategy {
    Tracer config;     // 1st
    Tracer book;       // 2nd
    Tracer risk;       // 3rd
    Strategy() : risk("risk"), book("book"), config("config") { log("body"); }
    // ⚠️ g++ -Wreorder: "will be initialized after". The order is STILL config, book, risk
};
// lifetime of one Strategy: +config +book +risk body ... -risk -book -config
```
**Why care:** if `book` uses `config` in its constructor, `config` must be declared first. Writing the init list in a different order doesn't help, it only confuses readers (and `-Wreorder` catches it).

### 1.6 Returning objects: copy elision
**In one line:** when a function returns an object, C++ usually builds it **directly in the caller's variable**, with no copy and no extra destructor. This is called **copy elision**.

```cpp
Tracer make_tracer(const char* n) { return Tracer(n); }   // return a temporary
Tracer make_named(const char* n)  { Tracer t(n); return t; }   // return a named local

Tracer m = make_tracer("m");   // +m          (no copy: GUARANTEED since C++17)
Tracer n = make_named("n");    // +n          (no copy: "NRVO", optional but g++ does it)
```
| Kind | Name | Guaranteed? | With `-fno-elide-constructors` |
|---|---|---|---|
| `return T(...);` (a temporary / prvalue) | RVO | **yes**, since C++17 | still no copy |
| `return local;` | NRVO (named RVO) | no, but every major compiler does it in simple cases | `+n copy(n) -n …` |

Day 25 goes deeper (and adds moves). Today, just know that returning by value is cheap. Don't "optimize" it into out-parameters.

### 1.7 The C++20 range-for trap
```cpp
for (int v : make_book().levels()) { ... }   // ❌ C++20: UB
```
Range-for is defined roughly as `auto&& range = make_book().levels();`, then the loop. The `Book` temporary dies at the end of **that** line, and `range` refers into its dead vector. On your machine: garbage numbers (BUG1). C++23 extends the temporaries in a range-for header, but you compile with C++20, so name the object: `Book b = make_book(); for (int v : b.levels())`.

---

## 2. Internals: what actually happens

### 2.1 Where temporaries live
A temporary is usually just a slot in the current **stack frame** (Day 09). "Dying at the `;`" means the compiler calls the destructor there. The slot may be reused by the next temporary, which is exactly why a dangling reference to it reads "someone else's" data.

### 2.2 How the compiler implements lifetime extension
```cpp
const Tracer& r = Tracer("ext");
// ≈ the compiler secretly writes:
Tracer __hidden("ext");          // a named object with r's scope
const Tracer& r = __hidden;
```
That's why it can only work when the compiler can **see** the temporary at the binding. Through a function call, it can't know which object the returned reference refers to.

### 2.3 How copy elision works (return slot)
The caller reserves space for the result in **its** frame and passes the address in secretly (on x86-64, as a hidden first argument). The callee constructs straight into that space. For NRVO, the compiler simply places the local `t` *in* the caller's slot. The copy constructor is never called. There's nothing to copy.
```
caller frame:  [ m: space for a Tracer ] ──address──► make_tracer(&m_slot, "m")
                                                         constructs Tracer in m_slot
```

### 2.4 The destruction-order rules in one table
| What | Order |
|---|---|
| locals in a scope | reverse of construction |
| members of a class | reverse of declaration (after the destructor body) |
| base classes (Day 16+) | after the members, in reverse of construction |
| array elements | from the last element to the first |
| statics / globals | reverse of their construction (Day 14) |
| temporaries in one full expression | reverse of construction |

---

## 3. Implementation (`impl.cpp`)

Build: `g++ -std=c++20 -O2 -Wall -Wextra -pedantic impl.cpp -o impl` then `./impl`.

**Step 1: `Tracer`.** Fill in the three bodies: log `"+" + name`, `"copy(" + name + ")"` and `"-" + name` with `g_log.add(...)`, and keep `alive` up to date. Then **predict every Step 1 assert** before you run it.

**Step 2: factories.** `make_tracer` must `return Tracer(n);` (a temporary). `make_named` must create `Tracer t(n);` and `return t;`. The tests expect **zero copies** for both.

**Step 3: `Strategy`.** Declare members `config`, `book` and `risk` (in that order), and write a constructor with an init list naming each one. The body logs `"body"`.
<details><summary>Hint</summary>`Tracer config; Tracer book; Tracer risk;` and `Strategy() : config("config"), book("book"), risk("risk") { g_log.add("body"); }`.</details>

**Step 4: `sum_levels()`.** Sum `make_book().levels()` **without** the dangling range-for. Keep the `Book` alive in a named variable.

### Part B (write it at the bottom of `impl.cpp`)
1. Build with `-fno-elide-constructors`. Which Step 2 test fails, and why only that one? (§1.6)
2. Write `Strategy`'s init list in reverse. Copy the `-Wreorder` text. Did the log change?
3. `const Tracer& r = make_named("ext");`: is it extended? (§1.4 table)

---

## 4. Output Questions
(`T` prints `+name`, `copy(name)`, `-name`.)

**Q1**
```cpp
int use(const T&, const T&) { std::printf("use "); return 0; }
use(T("a"), T("b"));
std::printf("| next\n");
```
<details><summary>Answer</summary>Your g++: `+b +a use -a -b | next`. Both temporaries live until the end of the full expression (after `use` returns), and die in reverse order of construction. Which argument is built first is <b>unspecified</b>. Another compiler may print `+a +b use -b -a`.</details>

**Q2**
```cpp
{
    const T& r = make_named("r");   // returns T BY VALUE
    std::printf("after ");
}
```
<details><summary>Answer</summary>`+r after -r`. The returned object is a temporary bound <b>directly</b> to `r`, so it's extended. (And NRVO meant no copy.)</details>

**Q3**
```cpp
struct Pair { T a; T b; Pair() : b("b"), a("a") { std::printf("body "); } };
{ Pair p; }
```
<details><summary>Answer</summary>`+a +b body -b -a`, plus a `-Wreorder` warning while compiling. Declaration order wins.</details>

**Q4**
```cpp
struct Holder { T t; explicit Holder(const char* s) : t(s) {} };
{ Holder h = Holder("h"); std::printf("after "); }
```
<details><summary>Answer</summary>`+h after -h`. Only <b>one</b> `T` exists: `Holder("h")` is a prvalue that initializes `h` directly (guaranteed elision), so there's no temporary `Holder` and no copy.</details>

---

## 5. Debugging (`-DBUG1` … `-DBUG3`)

**BUG1: range-for over a temporary's member**
```cpp
for (int v : make_book().levels()) std::printf("%d ", v);
```
<details><summary>Answer</summary>The `Book` dies before the loop runs, so you iterate a freed vector. Your run printed garbage like `-867368960 492 -4 …`. g++ warns (`-Wdangling-reference`, `-Wdangling-pointer`). Fix: `Book b = make_book(); for (int v : b.levels())`, or return the vector by value.</details>

**BUG2: lifetime extension through a function**
```cpp
const T& pass(const T& t) { return t; }
const T& r = pass(T("p"));
std::printf("%s", r.n);
```
<details><summary>Answer</summary>It prints `+p -p after, r.n = (null)` (garbage, because the object is dead). `r` refers to a destroyed temporary. g++ warns `-Wdangling-reference`. Fix: `T r = pass(T("p"));` (make a copy), or don't return references to parameters that may be temporaries.</details>

**BUG3: `std::string_view` of a temporary string**
```cpp
std::string_view sym = std::string("ESZ6-quarterly-future-contract");
std::string other(40, 'Z');
std::printf("%.30s", sym.data());
```
<details><summary>Answer</summary>A `string_view` is a non-owning pointer + length (Day 08: an observer). The temporary string dies at the `;`, so `sym` points into a freed heap block, which `other` then reuses. Your run printed `ZZZZ…`, and g++ gave <b>no warning</b>. Fix: keep a `std::string` that owns the data, and only make views of things that outlive the view.</details>

---

## 6. Performance (the HFT angle)
- **Return by value is free.** RVO/NRVO construct directly in the caller. Prefer `Order make_order(...)` over `void make_order(Order& out)`. It's just as fast and clearer.
- **Temporaries can be costly when they allocate.** A temporary `std::string` built per tick means a heap allocation (Day 11) plus a free at the `;`. In hot loops, reuse buffers and pass `std::string_view` of data that **outlives** the call.
- **Destruction is work too.** A scope that ends with 10 big objects runs 10 destructors (freeing memory, closing things). Hot-path objects should be trivially destructible (Day 11 arenas) or long-lived.
- **Deterministic destruction is a feature.** C++ has no garbage-collector pauses: you know to the line when cleanup happens. RAII (Day 22) uses this for scoped timers, locks and "order in flight" guards.
- **Don't rely on argument order** for side effects (logging, timestamps). It's unspecified, and it really does differ (Q1).

---

## 7. Interview Questions
<details><summary>1. When does an object's lifetime begin and end?</summary>It begins when initialization (the constructor) completes, and ends when the destructor call starts. For trivial types, it begins when storage is obtained and initialized.</details>
<details><summary>2. In what order are local objects destroyed?</summary>In reverse order of construction, at the end of their scope.</details>
<details><summary>3. When is a temporary destroyed?</summary>At the end of the full expression that created it, unless it's bound directly to a `const&` / `&&` reference, which extends it.</details>
<details><summary>4. Give an example where lifetime extension does NOT apply.</summary>A temporary passed to a function that returns a reference to it (`const T& r = pass(T());`), a member reference initialized in a constructor, or a range-for over a temporary's member function result (until C++23).</details>
<details><summary>5. In what order are members initialized?</summary>In the order they're declared in the class, regardless of the init-list order. They're destroyed in reverse.</details>
<details><summary>6. What is RVO, and what is NRVO? Which is guaranteed?</summary>RVO: returning a prvalue constructs straight into the caller (guaranteed since C++17). NRVO: returning a named local is elided too, but that's optional (compilers do it in simple cases).</details>
<details><summary>7. What does `-fno-elide-constructors` change in C++17+?</summary>It disables the optional elisions (NRVO, so copies or moves appear). The guaranteed prvalue elision still happens.</details>
<details><summary>8. Why is `std::string_view` dangerous with temporaries?</summary>It doesn't own its characters. Built from a temporary string, it points at freed memory once the string dies.</details>
<details><summary>9. Why is the order of function arguments relevant to lifetime?</summary>The order of construction of argument temporaries is unspecified, so side effects in constructors (logging, timestamps, counters) happen in compiler-dependent order. Their destruction is in reverse of whatever order was used.</details>
<details><summary>10. What does an explicit destructor call do, and when do you use one?</summary>`p->~T()` ends the object's lifetime without freeing the memory. You use it with placement new (arenas, pools, containers like `std::vector`).</details>
<details><summary>11. What is the C++20 range-for dangling problem?</summary>`for (x : make().member())`: the temporary from `make()` dies before the loop body, so the range dangles. C++23 extends the temporaries in a range-for header.</details>

---

## 8. Hard Questions (optional for now: come back after Day 15)
<details><summary>1. Why are members destroyed in reverse declaration order, and what bug happens if you depend on the wrong order?</summary>Later members may depend on earlier ones (a `Book` member that holds a pointer to the `Config` member), so they must die first. If you declare the dependent member <b>before</b> the thing it depends on, construction uses an unbuilt object (UB). `-Wreorder` only warns about the init-list order, not about the declaration order itself. Design rule: declare members in dependency order.</details>
<details><summary>2. A function `const std::string& name() const` on a temporary `make_instrument().name()` is passed into a logger that stores the reference for later. Why does it fail, and how do you design the API so it can't happen?</summary>The `Instrument` temporary dies at the `;`, so the stored reference dangles. Fixes: return by value (cheap with SSO for short names, Day 35), take ownership in the logger (store a `std::string`), or add `const std::string& name() const&` plus `std::string name() &&` (ref-qualified overloads), so that calls on temporaries return a copy. (Ref-qualifiers come up again around move semantics.)</details>
<details><summary>3. Copy elision means the copy constructor isn't called. Can you still have side effects in it, and is that a problem?</summary>Yes. The standard explicitly allows eliding a copy even if the copy constructor has side effects (logging, counting). So code that <b>relies</b> on copy-constructor side effects (counting copies for correctness) is broken by design. The tracer is fine because we're observing, not depending.</details>

---

## 9. Mini Project: trace your backtester's objects
1. Copy `Tracer` into your backtester as a debugging tool (under `#ifdef TRACE_LIFETIME`).
2. Add a `Tracer` member to your `Order`, `Fill` and `Strategy` (or whichever classes are created per event).
3. Run a tiny backtest (10 bars) and read the log. Count: how many objects are created per bar? Any **unexpected copies** (`copy(...)`)? Any temporaries created and destroyed inside a loop?
4. Remove the copies you find (pass by `const&`, return by value, `reserve` vectors), and rerun to confirm.

---

## Checklist
- [ ] Watched the video and read the theory
- [ ] `impl.cpp` passes all tests (`All Day 13 tests passed`)
- [ ] Part B written down
- [ ] `exercises.cpp`: Q1–Q4, BUG1–BUG3 done
- [ ] Mini project: copies found and removed
