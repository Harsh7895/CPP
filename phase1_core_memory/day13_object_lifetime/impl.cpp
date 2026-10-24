// Day 13: object lifetime: construction/destruction order, temporaries, lifetime extension
// Build:  g++ -std=c++20 -O2 -Wall -Wextra -pedantic impl.cpp -o impl
// Run:    ./impl
//
// Do the steps in order (README section 3 explains each one).
// The tests in main() FAIL until you fill in every TODO.
// Each test compares the EVENT LOG with what C++ promises. Predict every one first!

#include <cassert>
#include <cstdio>
#include <string>
#include <vector>

// ═════════════════════════════════════════════════════════════════════════════
// GIVEN: a global event log. Tracers write "+name", "copy(name)", "-name" into it.
// ═════════════════════════════════════════════════════════════════════════════
struct EventLog {
    std::string text;
    void add(const std::string& e) {
        if (!text.empty()) text += ' ';
        text += e;
    }
    std::string take() {          // return the log and clear it
        std::string t = text;
        text.clear();
        return t;
    }
};
EventLog g_log;

// ─────────────────────────────────────────────────────────────────────────────
// STEP 1: the lifetime tracer.
//   Tracer(name):      log "+name",        ++alive
//   Tracer(copy):      log "copy(name)",   ++alive   (the copy keeps the same name)
//   ~Tracer():         log "-name",        --alive
// (A "copy constructor" is the function that runs when you make a copy. Full story: Day 20.)
// ─────────────────────────────────────────────────────────────────────────────
struct Tracer {
    static inline int alive = 0;
    std::string name;

    explicit Tracer(const char* n) : name(n) {
        // TODO
    }
    Tracer(const Tracer& other) : name(other.name) {
        // TODO
    }
    ~Tracer() {
        // TODO
    }
    Tracer& operator=(const Tracer&) = delete;   // not needed today
};

// ─────────────────────────────────────────────────────────────────────────────
// STEP 2: factories. Write them as described. The tests check how many copies happen.
//   make_tracer:  return a TEMPORARY:  return Tracer(n);
//   make_named:   create a named local, then return it
// ─────────────────────────────────────────────────────────────────────────────
Tracer make_tracer(const char* n) {
    // TODO
    (void)n;
    return Tracer("?");
}

Tracer make_named(const char* n) {
    // TODO
    (void)n;
    Tracer t("?");
    return t;
}

// ─────────────────────────────────────────────────────────────────────────────
// STEP 3: a class with three Tracer members: config, book, risk (in THAT declaration
// order). Write the constructor's init list so they're named "config", "book", "risk".
// Try writing the init list in a different order: what does -Wreorder say?
// ─────────────────────────────────────────────────────────────────────────────
struct Strategy {
    // TODO: declare three members: config, book, risk (type Tracer)
    // TODO: Strategy() : ... { g_log.add("body"); }
    Strategy() { g_log.add("body"); }
};

// ─────────────────────────────────────────────────────────────────────────────
// STEP 4: the dangling range-for, fixed.
// Book::levels() returns a REFERENCE into the book. `for (int x : make_book().levels())`
// dangles in C++20 (the Book temporary dies before the loop runs: BUG1 in exercises).
// Implement sum_levels() SAFELY.
// ─────────────────────────────────────────────────────────────────────────────
struct Book {
    std::vector<int> lv{10, 20, 30};
    const std::vector<int>& levels() const { return lv; }
};
Book make_book() { return Book{}; }

int sum_levels() {
    // TODO
    return 0;
}

int main() {
    // Step 1: scope order
    {
        Tracer x("x");
        Tracer y("y");
    }
    assert(g_log.take() == "+x +y -y -x");                 // reverse order of creation
    assert(Tracer::alive == 0);

    // a temporary dies at the end of its full expression (the ';')
    {
        Tracer("tmp");
        g_log.add("after");
    }
    assert(g_log.take() == "+tmp -tmp after");

    // ...unless a const& binds directly to it: lifetime extension
    {
        const Tracer& r = Tracer("ext");
        g_log.add("after");
        assert(Tracer::alive == 1 && r.name == "ext");
    }
    assert(g_log.take() == "+ext after -ext");

    // an explicit copy
    {
        Tracer a("a");
        Tracer b(a);
    }
    assert(g_log.take() == "+a copy(a) -a -a");

    // arrays: destroyed in reverse
    {
        Tracer arr[3]{Tracer("0"), Tracer("1"), Tracer("2")};
    }
    assert(g_log.take() == "+0 +1 +2 -2 -1 -0");

    // Step 2: C++17 guaranteed elision (temporary) / NRVO (named, g++ does it)
    {
        Tracer m = make_tracer("m");
        g_log.add("after");
    }
    assert(g_log.take() == "+m after -m");                 // no copy at all
    {
        Tracer n = make_named("n");
        g_log.add("after");
    }
    assert(g_log.take() == "+n after -n");                 // NRVO: g++ skips the copy

    // Step 3: members built in DECLARATION order, before the body; destroyed in reverse
    {
        Strategy s;
        g_log.add("use");
    }
    assert(g_log.take() == "+config +book +risk body use -risk -book -config");
    assert(Tracer::alive == 0);

    // Step 4
    assert(sum_levels() == 60);

    std::printf("All Day 13 tests passed\n");
}

/* ── Part B observations (fill in) ────────────────────────────────────────────
 B.1 Build with  -fno-elide-constructors . Which Step 2 test fails now, and why only that one?
 B.2 In Strategy, write the init list as  : risk("risk"), book("book"), config("config").
     The -Wreorder warning text:            Did the log order change?
 B.3 Replace  const Tracer& r = Tracer("ext");  with  const Tracer& r = make_named("ext");
     Still extended? (yes/no, why?)
*/
