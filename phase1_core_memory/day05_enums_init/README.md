# Day 05 — `enum` vs `enum class`, struct vs class, `using` aliases, all initialization forms
> Phase 1 · Est. time 60–90 min · Builds on: Day 03 (storage duration: why globals start at zero), Day 04 (`constexpr`, `static_assert`)

## 🎯 Today in plain words
Today is about **how you describe your data** and **how you give it a starting value**.
- `enum class` gives names to a small set of choices (Buy/Sell, Limit/Market) safely.
- `struct` and `class` are *almost* the same thing. The difference is one rule.
- `using` gives a type a second name (`Price`, `Qty`). It does **not** create a new type, and that can hide bugs.
- C++ has many ways to initialize a variable (`=`, `()`, `{}`), and they do **not** all behave the same.

Why a trader cares: an order is a tiny struct of an id, a side, a price, a quantity and a type. Get the types right and the struct is **smaller** (more orders fit in the cache) and **safer** (you can't send a price where a quantity should go). Get initialization wrong and you send an order with a **garbage quantity**.

Files: `impl.cpp` (you implement it, step by step) and `exercises.cpp` (predict-the-output, compile or not, bugs).

---

## 📺 Watch first

| # | Video | Length | What to watch for |
|---|-------|--------|-------------------|
| 1 | **The Cherno: "ENUMS in C++"**. Search YouTube for that exact title (part of his C++ playlist) | about 8 min | The easiest start: what an enum is and why names beat magic numbers. Covers §1.1. He doesn't cover `enum class` much, so read §1.2 carefully. |
| 2 | **Back to Basics: Initialization, Ben Saks, CppCon 2023**. Search YouTube for "Back to Basics: Initialization in C++ Ben Saks" | about 1 hr | The clearest tour of all the initialization forms, and why braces `{}` exist. Covers §1.5–1.8. If time is short, watch the first 30 min. |
| 3 | *(optional, weekend)* [The Nightmare of Initialization in C++](https://www.youtube.com/watch?v=7DTlWPgX6zs), Nicolai Josuttis, CppCon 2018 | about 1 hr | A funny, honest look at *why* initialization is confusing. Great for §1.8 (the traps) and the hard questions. |

**Not covered well by the videos:** `using` aliases vs strong types (§1.4), designated initializers (§1.7), and the HFT angle on sizes and zeroing (§6). Read those parts carefully.

---

## 0. Warm-up (Day 04 + a Day 01 refresher)
1. What's the difference between `const int x = f();` and `constexpr int x = f();`?
2. Your `Instrument::tick_size()` is a `const` function, but it increments a counter. What keyword made that legal, and why was it needed?
3. *(Day 01, in simple words)* You compile `main.cpp` and `pricing.cpp`. The compiler is happy, but you get "undefined reference to `mid(...)`". Which step failed, and what is it looking for?

<details><summary>Answers</summary>

1. `const`: the value can't change afterwards, but it may be calculated **while the program runs**. `constexpr`: the value **must** be calculated **while compiling**, so `f()` must be a `constexpr` function.
2. `mutable`. Inside a `const` function every member is read-only, but a `mutable` member is allowed to change. The counter isn't part of the instrument's "real" value, so it's fine.
3. The **linker** failed. Each .cpp file is compiled separately into an object file. The linker then glues them together, and it couldn't find the **body** (definition) of `mid`. Usually you forgot to add `pricing.cpp` to the build, or the signatures don't match.
</details>

---

## 1. Theory

### 1.1 Plain `enum`: names for numbers
**In one line:** an `enum` (enumeration) is a type whose values are a fixed list of names, and each name is secretly a number.

```cpp
enum Side { Buy, Sell };        // Buy = 0, Sell = 1 (counting starts at 0)
enum Color { Red, Green = 5, Blue };   // Red = 0, Green = 5, Blue = 6 (keeps counting)

Side s = Buy;                   // note: just "Buy", no "Side::" needed
int n = s;                      // ✅ compiles! s quietly turns into the int 0
std::cout << n << '\n';         // prints 0
```

**Analogy:** a plain enum is like stickers with names on numbered boxes. Easy to use, but the names are stuck on **everything in the room** (the whole scope).

**Two problems:**
| Problem | Example |
|---------|---------|
| **Names leak** out into the surrounding scope | `enum Side { Buy, Sell };` and `enum Signal { Buy, Hold };` in the same file → ❌ error: `Buy` is declared twice |
| **Silent conversion** to `int` | `int x = Sell + 10;` compiles (x = 11). Nonsense code, and the compiler says nothing. |

**Why care:** in a big codebase (an OMS, an exchange gateway) the name `Buy` *will* clash with something. And silent conversion lets you compare a `Side` with an `OrdType` by accident.

### 1.2 `enum class`: the safe enum (use this one)
**In one line:** an `enum class` (scoped enum) keeps its names inside the enum and never turns into an `int` by itself.

```cpp
enum class Side { Buy, Sell };
enum class Signal { Buy, Hold };    // ✅ no clash: Side::Buy and Signal::Buy are different

Side s = Side::Buy;                 // you MUST write Side::
int n = s;                          // ❌ compile error: no silent conversion
int m = static_cast<int>(s);        // ✅ you can convert, but you must ask for it
if (s == Signal::Buy) {}            // ❌ compile error: can't compare different enums
```

**Analogy:** the names live inside a labeled drawer (`Side::`). To turn one into a number you must open the drawer on purpose (`static_cast`).

**Why care:** the compiler catches "compared a side with a signal" for you. This is the default choice in modern C++.

### 1.3 The underlying type: choosing the size
**In one line:** every enum is stored as some integer type (its *underlying type*), and you can pick it yourself.

```cpp
enum class Side : std::uint8_t { Buy, Sell };   // stored in 1 byte
enum class Big { A, B };                        // enum class default: int (4 bytes)

std::cout << sizeof(Side) << ' ' << sizeof(Big) << '\n';   // prints: 1 4
```
`std::uint8_t` = an unsigned 8-bit integer (one byte, 0..255), from `<cstdint>`.

**You can also choose the numbers**, which is handy when the exchange protocol already defines them:
```cpp
// FIX protocol (the standard message format between brokers and exchanges):
// OrdType tag 40: '1' = Market, '2' = Limit, '3' = Stop
enum class OrdType : std::uint8_t { Market = 1, Limit = 2, Stop = 3 };
```

**Why care:** a `Side` only needs 1 byte, not 4. In §6 you'll see that this makes an order struct smaller.

### 1.4 `using` aliases: a second name for a type
**In one line:** `using Price = std::int64_t;` makes `Price` another name for `std::int64_t`. It is **not** a new type.

```cpp
using Price = std::int64_t;     // modern way
typedef std::int32_t Qty;       // old C way: does exactly the same thing

Price p = 295035;               // fixed-point price with 2 decimals (Day 04): 2950.35
Qty   q = 10;
```

`using` can also make **alias templates** (a name with a blank to fill in), which `typedef` can't:
```cpp
template <typename T>
using Vec = std::vector<T>;     // Vec<int> is std::vector<int>
```

**⚠️ The trap:** because an alias is just a nickname, the compiler can't tell `Price` and `Qty` apart:
```cpp
void send_order(Price price, Qty qty);

send_order(q, p);   // ✅ compiles! price=10, qty=295035. You just bought 295,035 lots.
```

**The fix is a "strong type"**: wrap the number in its own tiny struct. Now it really is a different type.
```cpp
struct OrderId { std::uint64_t value; };
struct ClientId { std::uint64_t value; };

void cancel(OrderId id);
ClientId c{42};
cancel(c);          // ❌ compile error: ClientId is not an OrderId. Bug caught!
```
**Analogy:** an alias is a nickname for the same person. A strong type is a different person who happens to look similar.

**Why care:** mixing up ids, prices and quantities is a classic (and expensive) trading bug. A strong type costs **nothing** at run time (§6) and the compiler catches the mix-up.

### 1.5 `struct` vs `class`: only ONE real difference
**In one line:** `struct` and `class` are the same thing, except that members are **public by default** in a `struct` and **private by default** in a `class`.

```cpp
struct A { int x; };        // x is public
class  B { int x; };        // x is private

A a; a.x = 1;               // ✅
B b; b.x = 1;               // ❌ compile error: x is private
```
| | `struct` | `class` |
|-|----------|---------|
| Default member access | `public` | `private` |
| Default inheritance | `public` | `private` |
| Constructors, functions, virtual, templates | ✅ all allowed | ✅ all allowed |

**The convention** (not a rule):
- `struct` = a plain bag of data where any value is OK (an `Order`, a `Quote`).
- `class` = an object that protects a rule (an *invariant*, something that must always be true), e.g. "a `Position`'s net quantity only changes through `on_fill()`".

**Why care:** interviewers love "what's the difference?" The full answer is "only the defaults". Everything else is style.

### 1.6 Default init vs value init: garbage or zero?
**In one line:** `int x;` gives you whatever junk was in memory, while `int x{};` gives you zero.

```cpp
void f() {
    int a;          // DEFAULT init: a local int is NOT set. It holds leftover junk.
    int b{};        // VALUE init: empty braces mean "zero" → b = 0
    int c = int();  // also value init (older style) → c = 0

    std::cout << b << ' ' << c << '\n';   // prints 0 0
    std::cout << a << '\n';               // ❌ UNDEFINED BEHAVIOUR (UB): reading junk
}
int g;              // a GLOBAL is different: always zero (Day 03: static storage)
```
*Undefined behaviour (UB)* = the C++ rules give no promise at all. The program may print anything, crash, or "work" until it doesn't.

**Why care:** `int total;` and then `total += qty;` in a loop is a real bug. It may even work in Debug and fail in Release (`-O2`).

### 1.7 The many ways to initialize
**In one line:** you can write `=`, `( )` or `{ }`, and braces `{ }` are the safest because they refuse to lose data.

| Form | Example | Name | Note |
|------|---------|------|------|
| `T x;` | `int x;` | default init | junk for local ints (§1.6) |
| `T x{};` | `int x{};` | value init | zero |
| `T x = v;` | `int x = 5;` | copy init | the classic |
| `T x(v);` | `int x(5);` | direct init | calls a constructor |
| `T x{v};` | `int x{5};` | direct-list init | **no narrowing** (see below) |
| `T x = {v};` | `int x = {5};` | copy-list init | also no narrowing |
| `T x{a, b, …};` | `Order o{1, 2};` | aggregate init | fills a struct's members in order |
| `T x{.m = v};` | `Order o{.qty = 10};` | designated init (C++20) | fills members **by name** |

**Narrowing** = a conversion that can lose data (e.g. `double` → `int` drops the decimals). Braces forbid it:
```cpp
double px = 2950.35;
int a = px;      // ⚠️ compiles (maybe a warning). a = 2950: the .35 is silently lost
int b{px};       // ❌ compile error: narrowing conversion from double to int
```

**Aggregates and designated initializers.** An *aggregate* is a simple struct: all members public, no constructors you wrote, no virtual functions. You can fill it member by member:
```cpp
struct Order {
    std::uint64_t id = 0;       // default member initializer: used when you don't give a value
    std::int64_t  price = 0;
    std::int32_t  qty = 0;
};

Order a{7, 295035, 10};                        // by position: id=7, price=295035, qty=10
Order b{.id = 8, .qty = 5};                    // by name (C++20): price uses its default, 0
Order c{};                                     // everything uses its default
std::cout << b.id << ' ' << b.price << ' ' << b.qty << '\n';   // prints 8 0 5
```
Rules for designated init: names must be in the **same order** as in the struct. You may skip some, but you can't go backwards.

**Why care:** `Order{.price = 295035, .qty = 10}` reads like a sentence. With `Order{295035, 10}` it's easy to swap the two.

### 1.8 Three famous traps
**Trap 1: `{}` vs `()` with `std::vector`.** Braces prefer the "list of values" constructor (`std::initializer_list`):
```cpp
std::vector<int> a(3, 1);   // 3 elements, each 1 → {1, 1, 1}
std::vector<int> b{3, 1};   // 2 elements: 3 and 1 → {3, 1}
std::cout << a.size() << ' ' << b.size() << '\n';   // prints 3 2
```

**Trap 2: the "most vexing parse".** This line is a **function declaration**, not an object:
```cpp
Book book();     // declares a FUNCTION named book that returns a Book. No object is made!
Book book2{};    // ✅ this makes an object
```

**Trap 3: `auto` with braces.**
```cpp
auto x{5};       // int
auto y = {5};    // std::initializer_list<int> (a read-only list), NOT an int!
```

**Simple rule of thumb:**
- Use `{}` by default (safe: zero for empty braces, no narrowing).
- Use `()` when you want a constructor *with arguments* on a container (`std::vector<int>(n, 0)`).
- Use `auto x = value;` with `=` when you use `auto`.

### 1.9 Summary table
| Tool | One-line meaning | Use it for |
|------|------------------|-----------|
| `enum` | names for numbers, leak + convert to int | old code, C APIs |
| `enum class` | scoped names, no silent conversion | every new enum |
| `enum class X : std::uint8_t` | choose the size and numbers | protocol fields, compact structs |
| `using A = B;` | a nickname, same type | readability (`Price`, `Qty`) |
| strong type `struct X { T value; };` | a truly new type | ids, anything you must not mix up |
| `struct` | public by default | plain data (Order, Quote) |
| `class` | private by default | objects with rules (Position, OrderBook) |
| `T x{};` | zero / default value | almost always |
| `T x{.m = v};` | fill a struct by name | building orders and events |

---

## 2. Internals: what actually happens

### 2.1 An enum is just an integer in memory
```
enum class Side : std::uint8_t { Buy, Sell };
Side s = Side::Sell;

memory for s:  [ 0x01 ]          ← 1 byte, the value 1
```
The names `Buy`/`Sell` **don't exist** in the compiled program. The compiler replaces them with 0 and 1. That's why `to_string(Side)` has to be written by hand (C++ has no built-in "enum to text" yet).

### 2.2 A `switch` on an enum often becomes a jump table
```cpp
switch (type) {                 // type is OrdType: 1, 2 or 3
    case OrdType::Market: return "MKT";
    case OrdType::Limit:  return "LMT";
    case OrdType::Stop:   return "STP";
}
```
When the values are small and close together, the compiler builds a **jump table**: an array of addresses indexed by the value. Instead of 3 comparisons it does one "look up address, jump". Constant time.

With `-Wall`, if you **forget a case** in a switch over an enum, GCC warns: `enumeration value 'Stop' not handled in switch`. That warning is a free bug detector, so don't silence it with `default:`.

### 2.3 Aliases and strong types disappear after compiling
```
using Price = std::int64_t;        → the compiler just sees int64_t
struct OrderId { uint64_t value; } → in memory: exactly 8 bytes, same as a uint64_t
```
Passing an `OrderId` to a function uses the same register as passing a plain `uint64_t`. The safety exists only **at compile time**. Run-time cost: zero.

### 2.4 What `{}` does to memory
```
int a;            → nothing happens. The stack slot keeps old bytes:  [ ?? ?? ?? ?? ]
int b{};          → the compiler writes zero:                          [ 00 00 00 00 ]
Order o{};        → each member gets its default (or zero)
static int g;     → the loader zeroes it before main() (BSS segment, Day 09 preview)
```
For a huge buffer, `{}` means zeroing **every byte**. That costs time (§6).

### 2.5 Struct layout: member order and padding
The compiler puts members in memory **in the order you declare them**. It may add empty *padding* bytes so each member starts at an address its type likes (its *alignment*: an 8-byte int likes addresses divisible by 8). Full detail on Day 12. A quick preview:
```
struct Order {               offset  size
    uint64_t id;               0      8
    int64_t  price;            8      8
    int32_t  qty;             16      4
    Side     side;  (1 byte)  20      1
    OrdType  type;  (1 byte)  21      1
};                           22..23   2 bytes padding → sizeof = 24
```
If `Side` and `OrdType` were plain 4-byte enums: 8 + 8 + 4 + 4 + 4 = 28 → padded to **32**. Choosing `uint8_t` saves 8 bytes per order.

> 🔍 **Going deeper (optional)**
> - A plain enum *without* a fixed type has an underlying type picked by the compiler (big enough for all values, usually `int`). Casting a number outside its range into it is UB. With a fixed type (`: std::uint8_t`), any value of that type is allowed, even one with no name. So `static_cast<OrdType>(9)` is legal, but it's not a real order type. Always **validate** bytes that come from the network (impl.cpp Step 3).
> - C++20 `using enum OrdType;` inside a function or a `switch` lets you write `Market` instead of `OrdType::Market` in that one place.
> - C++23 adds `std::to_underlying(e)`, a shorter `static_cast<std::underlying_type_t<E>>(e)`. We target C++20, so use `static_cast`.
> - Since C++20, aggregates can also be initialized with parentheses: `Order o(1, 2, 3);`. Unlike braces, this allows narrowing.

---

## 3. Implementation (`impl.cpp`)
You will build the pieces of an **order** for your backtester: the enums, the aliases, a strong `OrderId`, the `Order` struct and a `Position` class.

**Step 1: fix `Side`.** It's given as a plain `enum`. Turn it into an `enum class` stored in 1 byte. Then write `sign(side)` (+1 for Buy, -1 for Sell) and `opposite(side)`.
<details><summary>Hint</summary>

`enum class Side : std::uint8_t { Buy, Sell };`. For `sign`, a simple `if` or the ternary `s == Side::Buy ? 1 : -1` is enough.
</details>

**Step 2: `to_string(OrdType)`.** Return `"MKT"`, `"LMT"` or `"STP"`. Use a `switch` with **no `default:`**, so the compiler warns if someone adds a new enum value later.
<details><summary>Hint</summary>

Inside the function you can write `using enum OrdType;` and then `case Market:`. After the switch, add `return "?";` so every path returns something.
</details>

**Step 3: wire conversion.** Exchanges send the order type as one byte. Write `to_wire(OrdType)` → the byte (1, 2, 3), and `from_wire(byte, out)`, which **checks** the byte: it returns `true` and sets `out` for 1/2/3, and returns `false` for anything else (0, 4, 255...).
<details><summary>Hint</summary>

`static_cast<std::uint8_t>(t)` gives the number. Never just cast an unchecked byte to the enum: `static_cast<OrdType>(9)` compiles but means nothing.
</details>

**Step 4: strong `OrderId`.** Give `value` a default of 0 and make `==` work. The test checks that a plain number **cannot** silently become an `OrderId`.
<details><summary>Hint</summary>

C++20 can write `==` for you: `friend constexpr bool operator==(OrderId, OrderId) = default;`. It compares every member.
</details>

**Step 5: `Order` defaults.** Add default member initializers so that `Order{}` is a **Limit** order with quantity 1 and everything else 0 / Buy. Then write `make_sell(...)` using **designated initializers**. Finally, check the `sizeof(Order) == 24` test passes (it needs Step 1).
<details><summary>Hint</summary>

`OrdType type = OrdType::Limit;` inside the struct. In `make_sell`: `return Order{.id = id, .price = px, .qty = q, .side = Side::Sell};`, keeping the declaration order.
</details>

**Step 6: `Position` class.** `on_fill(side, qty)` adds the quantity for a buy and subtracts it for a sell (use `sign`). `net()` returns the current position. The data is `private` and can only change through `on_fill`.

**Step 7: braces vs parentheses.** Make `make_three_ones()` return `{1, 1, 1}` and `make_three_and_one()` return `{3, 1}`, using `std::vector<int>` with `(3, 1)` or `{3, 1}`. Pick the right one for each.

**Step 8: compile-time check.** Uncomment the `static_assert` lines at the bottom (like on Day 04).

### Part B: break it on purpose (write what happened at the bottom of `impl.cpp`)
1. Add `enum Signal { Buy, Hold };` above `Side` **while Side is still a plain enum**. What error? Now do it again after Step 1. What changes?
2. Write `int x = Side::Buy;` after Step 1. What error?
3. Write `std::int32_t q{2.5};` and then `std::int32_t q2 = 2.5;`. Which one compiles?
4. In `make_sell`, put `.qty` before `.price`. What error?
5. Write `void send(Price p, Qty q) {}` and call `send(Qty{10}, Price{295035});`. Does it compile? Why is that scary? How does `OrderId` avoid this?

---

## 4. Output Questions
Predict first, then run `exercises.cpp`.

**Q1.** Plain enum values
```cpp
enum Level { Low, Mid = 5, High };
std::cout << Low << ' ' << Mid << ' ' << High << '\n';
```
<details><summary>Answer</summary>

`0 5 6`. Counting starts at 0, and after `Mid = 5` it continues at 6. A plain enum prints as a number because it silently converts to `int`.
</details>

**Q2.** Braces vs parentheses
```cpp
std::vector<int> a(3, 7);
std::vector<int> b{3, 7};
std::cout << a.size() << ' ' << b.size() << ' ' << a[0] << ' ' << b[0] << '\n';
```
<details><summary>Answer</summary>

`3 2 7 3`. `a` is three 7s. `b` is the list {3, 7}.
</details>

**Q3.** Sizes
```cpp
enum class A { X };
enum class B : std::uint8_t { X };
enum C { X };
std::cout << sizeof(A) << ' ' << sizeof(B) << ' ' << sizeof(C) << '\n';
```
<details><summary>Answer</summary>

`4 1 4` on GCC. An `enum class` defaults to `int`. `B` asked for 1 byte. Plain `C` lets the compiler choose, which is `int`-sized here.
</details>

**Q4.** Designated initializers + defaults
```cpp
struct Quote { int bid = 100; int ask = 101; int size = 1; };
Quote q{.ask = 105};
std::cout << q.bid << ' ' << q.ask << ' ' << q.size << '\n';
```
<details><summary>Answer</summary>

`100 105 1`. Only `ask` was given. The others use their default member initializers.
</details>

**Q5.** The most vexing parse
```cpp
struct Book { Book() { std::cout << "built "; } };
Book b1;
Book b2();
Book b3{};
std::cout << '\n';
```
<details><summary>Answer</summary>

`built built`. Only **two** objects are made. `Book b2();` declares a function and creates nothing. GCC may even warn: "empty parentheses were disambiguated as a function declaration".
</details>

**Q6.** `auto` with braces
```cpp
auto x{5};
auto y = {5};
std::cout << std::is_same_v<decltype(x), int> << ' '
          << std::is_same_v<decltype(y), int> << '\n';
```
<details><summary>Answer</summary>

`1 0`. `x` is `int`. `y` is `std::initializer_list<int>`.
</details>

### Will it compile?
| # | Code | Answer |
|---|------|--------|
| C1 | `int a{3.7};` | <details><summary>?</summary>❌ Narrowing from `double` to `int` inside braces.</details> |
| C2 | `enum class S { B }; int x = S::B;` | <details><summary>?</summary>❌ No implicit conversion. Use `static_cast<int>(S::B)`.</details> |
| C3 | `char c{65};` | <details><summary>?</summary>✅ 65 is a constant that fits in a `char`, so it's not narrowing. `c` is `'A'`.</details> |
| C4 | `struct P { int x; int y; }; P p{.y = 1, .x = 2};` | <details><summary>?</summary>❌ Designated initializers must follow the declaration order.</details> |
| C5 | `class Q { int bid; }; Q q{1};` | <details><summary>?</summary>❌ `bid` is private (class default), so `Q` is not an aggregate and has no constructor taking an int.</details> |

---

## 5. Debugging
Each snippet has a bug. Find it before reading the answer.

**B1.** Sum of fills
```cpp
int total_qty(const std::vector<int>& fills) {
    int total;
    for (int f : fills) total += f;
    return total;
}
```
<details><summary>Fix</summary>

`total` is never initialized, so it starts as junk (UB). Write `int total{};` or `int total = 0;`. With `-O2 -Wall`, GCC usually warns "may be used uninitialized".
</details>

**B2.** Missing case
```cpp
enum class OrdType : std::uint8_t { Market = 1, Limit = 2, Stop = 3 };
int priority(OrdType t) {
    switch (t) {
        case OrdType::Market: return 0;
        case OrdType::Limit:  return 1;
    }
}
```
<details><summary>Fix</summary>

`Stop` isn't handled, and the function falls off the end without returning. Calling `priority(OrdType::Stop)` is UB. `-Wall` warns about both. Add `case OrdType::Stop: return 2;` and a final `return -1;` after the switch.
</details>

**B3.** Swapped arguments
```cpp
using Price = std::int64_t;
using Qty   = std::int64_t;
void send_order(Price px, Qty qty);

Qty   q  = 10;
Price px = 295035;
send_order(q, px);
```
<details><summary>Fix</summary>

The arguments are swapped, and the compiler can't see it because both aliases are the same type. Call `send_order(px, q)`. The real fix is strong types (`struct Price { std::int64_t v; };`) so the swap becomes a compile error.
</details>

**B4.** Reading a byte from the network
```cpp
OrdType parse(std::uint8_t byte) { return static_cast<OrdType>(byte); }
```
<details><summary>Fix</summary>

No check. A byte of 0 or 200 becomes an `OrdType` with no name, and the code downstream (switches, to_string) will misbehave. Validate first, as `from_wire` does in impl.cpp.
</details>

---

## 6. Performance (the HFT angle)
| Topic | What to know |
|-------|--------------|
| **Pick small underlying types** | `enum class Side : std::uint8_t` instead of `int` makes `Order` 24 bytes instead of 32. A cache line is 64 bytes, so that's 2.67 orders per line instead of 2. Over millions of orders in a backtest, fewer cache lines means fewer cache misses. (Day 12 goes deep on padding.) |
| **Strong types are free** | `OrderId` is exactly a `uint64_t` in memory and in registers. You get safety at zero run-time cost. Check it with `static_assert(sizeof(OrderId) == 8);`. |
| **Aliases are free** | `using` exists only for the compiler. |
| **`switch` on a dense enum** | It becomes a jump table: O(1), one indirect jump. Keep enum values small and close together (1, 2, 3, not 1, 100, 5000) so the compiler can do this. |
| **Zeroing has a cost** | `std::array<char, 1 << 20> buf{};` writes 1 MB of zeros. `std::vector<int> v(n);` also zeroes all `n` ints. On a hot path, use `v.reserve(n)` (memory only, no zeroing) and `push_back`, or skip zeroing a buffer you'll fully overwrite anyway. |
| **But never skip init for correctness** | For small objects (an `Order`), zeroing costs a few instructions. A garbage quantity in a live order costs a lot more. Default: always initialize. |

---

## 7. Interview Questions
<details><summary>1. What's the difference between enum and enum class?</summary><code>enum class</code> is scoped (names need <code>Side::</code>), doesn't convert to <code>int</code> implicitly, and defaults to <code>int</code> as its underlying type. A plain <code>enum</code> leaks its names into the surrounding scope and converts silently.</details>
<details><summary>2. How do you get the number out of an enum class?</summary><code>static_cast&lt;int&gt;(e)</code>, or more generally <code>static_cast&lt;std::underlying_type_t&lt;E&gt;&gt;(e)</code>. C++23 adds <code>std::to_underlying</code>.</details>
<details><summary>3. Why give an enum an underlying type?</summary>To control its size (memory, cache, packed protocol messages) and to allow forward declaration (<code>enum class Side : std::uint8_t;</code>).</details>
<details><summary>4. What's the difference between struct and class?</summary>Only the defaults: struct members and bases are public by default, class members and bases are private by default. Everything else is identical.</details>
<details><summary>5. typedef vs using?</summary>They do the same thing for simple aliases. <code>using</code> is easier to read and can be templated (alias templates). <code>typedef</code> can't.</details>
<details><summary>6. Does a type alias create a new type?</summary>No. It's another name for the same type, so mixing two aliases of the same type compiles. Use a strong type (a wrapper struct) for a new type.</details>
<details><summary>7. What is narrowing, and which initialization forms prevent it?</summary>A conversion that can lose information (double→int, a large int→char). Brace initialization (<code>{}</code> and <code>= {}</code>) makes it a compile error.</details>
<details><summary>8. What value does an uninitialized local int have?</summary>An indeterminate value. Reading it is undefined behaviour. A global or static int is zero-initialized.</details>
<details><summary>9. What is value initialization?</summary>Initializing with empty <code>{}</code> or <code>()</code>, e.g. <code>int x{};</code>. Built-in types become zero. Class types call their default constructor (and are zeroed first if that constructor isn't user-provided).</details>
<details><summary>10. What is an aggregate?</summary>An array, or a class/struct with no user-declared constructors, no private or protected non-static data members, no virtual functions and no virtual, private or protected base classes. You can initialize it with braces, member by member.</details>
<details><summary>11. What are designated initializers?</summary>C++20 syntax to initialize aggregate members by name: <code>Order{.price = 1, .qty = 2}</code>. They must follow the declaration order. Skipped members use their defaults.</details>
<details><summary>12. What is the most vexing parse?</summary><code>T x();</code> is parsed as a declaration of a function returning <code>T</code>, not as an object. Use <code>T x{};</code> or <code>T x;</code>.</details>
<details><summary>13. Why do std::vector&lt;int&gt;(3, 1) and std::vector&lt;int&gt;{3, 1} differ?</summary>Braces prefer the <code>std::initializer_list</code> constructor, giving two elements {3, 1}. Parentheses call the (count, value) constructor, giving {1, 1, 1}.</details>
<details><summary>14. What type is auto x{5}; and auto y = {5};?</summary><code>x</code> is <code>int</code>. <code>y</code> is <code>std::initializer_list&lt;int&gt;</code>.</details>
<details><summary>15. Why avoid default: in a switch over an enum class?</summary>Without <code>default:</code>, <code>-Wall</code> warns you when a new enum value isn't handled. A <code>default:</code> hides that warning.</details>

---

## 8. Hard Questions (optional for now: come back after Day 15)
<details><summary>H1. Design the Side/OrdType/TimeInForce fields of an order message that is sent over the network to an exchange. What do you choose and why?</summary>

`enum class X : std::uint8_t` with **explicit values** that match the exchange spec (e.g. FIX `'1'`/`'2'`), so the in-memory value *is* the wire value and conversion is a plain copy. Never `static_cast` an incoming byte directly. Validate it with a switch or a lookup table, and reject unknown values (a malformed or malicious packet must not become an "order type"). Use `static_assert` on `sizeof` to lock the message layout, so a teammate who changes a field to `int` gets a compile error instead of a corrupted message.
</details>

<details><summary>H2. Why does C++ have so many initialization forms, and what's a sane team rule?</summary>

History. `=` and `()` came from C and C++98. Braces were added in C++11 to give one "uniform" syntax that also blocks narrowing and fixes the most vexing parse. But braces have their own trap: they prefer `initializer_list` constructors. A sane rule: use `{}` by default, use `()` for containers when you mean (count, value) or (size), use `auto x = expr;` with `auto`, and use designated initializers for structs with more than 2–3 fields.
</details>

<details><summary>H3. A strong type `struct Qty { int32_t v; };` is "free". Prove it and name one situation where it might not be.</summary>

Prove it: compare the assembly (`g++ -O2 -S`) of a function taking `int32_t` and one taking `Qty`. They're identical: same register, same instructions. `static_assert(sizeof(Qty) == 4)` confirms the size. A possible cost: on some ABIs (the platform's rules for calling functions), a struct is passed differently from a plain int. Example: a non-trivially-copyable struct (one with a user-written copy constructor or destructor) is passed through memory instead of a register. Keep strong types trivial (no custom copy constructor or destructor) and the cost stays zero.
</details>

<details><summary>H4. `Order o;` (no braces) inside a hot loop: is skipping the zeroing a good optimization?</summary>

Usually no. For a 24-byte struct, zeroing is 2–3 store instructions, and the compiler removes them anyway when you overwrite every field right after (dead-store elimination). Leaving members uninitialized risks reading garbage, which is UB. The place where skipping init really matters is **large buffers** (MBs), where you should use `reserve` or deliberately uninitialized storage that you fully overwrite. Measure before you "optimize".
</details>

---

## 9. Mini Project: typed events for your backtester
Your backtester reads a CSV of events like:
```
T,295035,10        ← trade: price, qty
B,295030,25        ← new buy order
S,295040,5         ← new sell order
```
1. Make `enum class EventType : std::uint8_t { Trade, NewBuy, NewSell };` and `struct Event` (an aggregate with an `EventType`, a `Price` and a `Qty`, using **default member initializers**).
2. Write `bool parse_type(char c, EventType& out)`, which accepts only `'T'`, `'B'`, `'S'`.
3. Write `bool parse_line(std::string_view line, Event& out)`, which splits on commas, uses `parse_type` and builds the event with **designated initializers**. Return `false` on a bad line instead of crashing.
4. Add `static_assert(sizeof(Event) <= 16);`. Reorder the members if it fails.
5. Feed in 5 lines (one deliberately broken, e.g. `X,1,1`) and print a count per event type using `to_string(EventType)` with a `switch` (no `default:`).
6. Bonus: replace `Price`/`Qty` aliases with strong types and see which lines of your code stop compiling. Those are the places a swap was possible.

---

## Checklist
- [ ] watched video 1 (+ video 2 if there's time)
- [ ] read sections 1–2
- [ ] `impl.cpp`: Steps 1–7 pass, Step 8 `static_assert`s compile
- [ ] Part B: 5 breakages written down
- [ ] exercises: Q1–Q6, C1–C5, B1–B4 predicted before running
- [ ] mini project: CSV events parsed into typed `Event`s
