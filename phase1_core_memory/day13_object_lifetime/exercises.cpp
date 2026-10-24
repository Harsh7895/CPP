// Day 13: Exercises. PREDICT the output BEFORE running!
//
// Output questions:   g++ -std=c++20 -O2 -Wall -Wextra exercises.cpp -o ex && ./ex
//                     (g++ prints a -Wreorder warning for Q3: that's part of the question)
// Bugs:               add -DBUG1, -DBUG2 or -DBUG3 (one at a time)
//                     They're dangling bugs (UB): the output may be garbage, or look fine.

#include <cstdio>
#include <string>
#include <string_view>
#include <vector>

struct T {
    const char* n;
    explicit T(const char* s) : n(s) { std::printf("+%s ", n); }
    T(const T& o) : n(o.n) { std::printf("copy(%s) ", n); }
    ~T() { std::printf("-%s ", n); }
};

T make_named(const char* s) { T t(s); return t; }

// ── Q1: temporaries in one expression ───────────────────────────────────────
int use(const T&, const T&) { std::printf("use "); return 0; }

void q1() {
    std::printf("Q1: ");
    use(T("a"), T("b"));
    std::printf("| next\n");
}

// ── Q2: lifetime extension through a by-value return ────────────────────────
void q2() {
    std::printf("Q2: ");
    {
        const T& r = make_named("r");
        std::printf("after ");
        (void)r;
    }
    std::printf("\n");
}

// ── Q3: member order vs init-list order ─────────────────────────────────────
struct Pair {
    T a;
    T b;
    Pair() : b("b"), a("a") { std::printf("body "); }   // init list written b, a
};

void q3() {
    std::printf("Q3: ");
    { Pair p; }
    std::printf("\n");
}

// ── Q4: a temporary inside a bigger object ──────────────────────────────────
struct Holder {
    T t;
    explicit Holder(const char* s) : t(s) {}
};

void q4() {
    std::printf("Q4: ");
    {
        Holder h = Holder("h");     // how many T objects are created?
        std::printf("after ");
    }
    std::printf("\n");
}

// ── Bugs ────────────────────────────────────────────────────────────────────
struct Book {
    std::vector<int> lv{10, 20, 30};
    const std::vector<int>& levels() const { return lv; }
};
Book make_book() { return Book{}; }

const T& pass(const T& t) { return t; }

void bugs() {
#ifdef BUG1
    std::printf("BUG1: ");
    int shown = 0;
    for (int v : make_book().levels()) {      // C++20: the Book dies BEFORE the loop body
        std::printf("%d ", v);
        if (++shown == 5) { std::printf("... (stopped)"); break; }
    }
    std::printf("\n");
#endif
#ifdef BUG2
    std::printf("BUG2: ");
    const T& r = pass(T("p"));                 // extension does NOT go through a function
    std::printf("after, r.n = %s\n", r.n);
#endif
#ifdef BUG3
    std::string_view sym = std::string("ESZ6-quarterly-future-contract");  // view of a temporary
    std::string other(40, 'Z');                // reuse the freed heap block
    std::printf("BUG3: sym = %.30s\n", sym.data());
#endif
}

int main() {
    q1();
    q2();
    q3();
    q4();
    bugs();
}
