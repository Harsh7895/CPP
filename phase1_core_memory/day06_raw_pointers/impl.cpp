// Day 06: raw pointers, pointer arithmetic, pointer-to-pointer, const pointers, 2D arrays
// Build:  g++ -std=c++20 -O2 -Wall -Wextra -pedantic impl.cpp -o impl
// Run:    ./impl
//
// Do the steps in order (README section 3 explains each one).
// The tests in main() FAIL until you fill in every TODO.
// Rule for today: use POINTERS, not std::vector / std::array / references.

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>

// ─────────────────────────────────────────────────────────────────────────────
// STEP 1: swap two prices through pointers
// ─────────────────────────────────────────────────────────────────────────────
void swap_prices(std::int64_t* a, std::int64_t* b) {
    // TODO
    (void)a;
    (void)b;
}

// ─────────────────────────────────────────────────────────────────────────────
// STEP 2: walk [first, last) with ++ and return the first price > limit.
// Return `last` if there is none. No indexes, only pointers.
// ─────────────────────────────────────────────────────────────────────────────
const std::int64_t* find_first_above(const std::int64_t* first,
                                     const std::int64_t* last,
                                     std::int64_t limit) {
    // TODO
    (void)first;
    (void)last;
    (void)limit;
    return nullptr;
}

// ─────────────────────────────────────────────────────────────────────────────
// STEP 3: a market-data cursor (pointer to pointer)
// If *cursor == end: return false and change nothing.
// Else: write the current price into *out, move the CALLER's cursor forward by one,
// return true.
// ─────────────────────────────────────────────────────────────────────────────
bool next_tick(const std::int64_t** cursor, const std::int64_t* end, std::int64_t* out) {
    // TODO
    (void)cursor;
    (void)end;
    (void)out;
    return false;
}

// ─────────────────────────────────────────────────────────────────────────────
// STEP 4: jagged 2D grid = array of row pointers. Every cell starts at 0.0.
// ─────────────────────────────────────────────────────────────────────────────
double** make_jagged(std::size_t rows, std::size_t cols) {
    // TODO: new double*[rows], then new double[cols]{} for each row
    (void)rows;
    (void)cols;
    return nullptr;
}

void free_jagged(double** grid, std::size_t rows) {
    // TODO: delete[] each row, then delete[] the spine
    (void)grid;
    (void)rows;
}

// ─────────────────────────────────────────────────────────────────────────────
// STEP 5: contiguous matrix = ONE block, index = r * cols + c (row-major)
// ─────────────────────────────────────────────────────────────────────────────
class Matrix {
public:
    Matrix(std::size_t rows, std::size_t cols) {
        // TODO: store the sizes and allocate rows*cols doubles, all 0.0
        (void)rows;
        (void)cols;
    }

    ~Matrix() {
        // TODO: free the block (which delete?)
    }

    // Copying a raw-pointer owner needs the Rule of 3/5 (Day 22). Forbidden for now.
    Matrix(const Matrix&) = delete;
    Matrix& operator=(const Matrix&) = delete;

    std::size_t rows() const { return rows_; }
    std::size_t cols() const { return cols_; }

    double get(std::size_t r, std::size_t c) const {
        // TODO
        (void)r;
        (void)c;
        return -1.0;
    }

    void set(std::size_t r, std::size_t c, double v) {
        // TODO
        (void)r;
        (void)c;
        (void)v;
    }

    double* row(std::size_t r) {             // pointer to the first cell of row r
        // TODO
        (void)r;
        return nullptr;
    }
    const double* row(std::size_t r) const { // same, read-only version
        // TODO
        (void)r;
        return nullptr;
    }

    const double* data() const { return data_; }

private:
    std::size_t rows_ = 0;
    std::size_t cols_ = 0;
    double* data_ = nullptr;
};

// ─────────────────────────────────────────────────────────────────────────────
// STEP 6: sums. row_sum walks with a pointer; col_sum jumps by cols() (a stride).
// ─────────────────────────────────────────────────────────────────────────────
double row_sum(const double* row, std::size_t n) {
    // TODO
    (void)row;
    (void)n;
    return -1.0;
}

double col_sum(const Matrix* m, std::size_t c) {
    // TODO: use m->row(0) and pointer jumps of m->cols(), or m->get(r, c)
    (void)m;
    (void)c;
    return -1.0;
}

int main() {
    // Step 1
    std::int64_t a = 1, b = 2;
    swap_prices(&a, &b);
    assert(a == 2 && b == 1);

    // Step 2
    const std::int64_t px[] = {100, 101, 99, 105, 102};
    assert(find_first_above(px, px + 5, 101) == px + 3);
    assert(find_first_above(px, px + 5, 99) == px + 0);
    assert(find_first_above(px, px + 5, 200) == px + 5);   // none -> last
    assert(find_first_above(px, px, 0) == px);             // empty range -> last

    // Step 3
    const std::int64_t* cur = px;
    std::int64_t v = 0;
    assert(next_tick(&cur, px + 5, &v) && v == 100 && cur == px + 1);
    int n = 1;
    while (next_tick(&cur, px + 5, &v)) ++n;
    assert(n == 5 && v == 102 && cur == px + 5);
    v = -7;
    assert(!next_tick(&cur, px + 5, &v) && v == -7 && cur == px + 5);   // untouched

    // Step 4
    double** g = make_jagged(3, 4);
    assert(g != nullptr);
    assert(g[0][0] == 0.0 && g[2][3] == 0.0);
    g[2][3] = 7.5;
    g[0][1] = 1.25;
    assert(g[2][3] == 7.5 && g[0][1] == 1.25 && g[1][1] == 0.0);
    free_jagged(g, 3);
    g = nullptr;

    // Step 5
    Matrix m(3, 4);
    assert(m.rows() == 3 && m.cols() == 4);
    assert(m.data() != nullptr);
    assert(m.get(0, 0) == 0.0 && m.get(2, 3) == 0.0);
    m.set(1, 2, 5.0);
    assert(m.get(1, 2) == 5.0);
    double* r1 = m.row(1);
    assert(r1 != nullptr && r1[2] == 5.0);
    assert(m.row(2) - m.row(1) == 4);                      // rows are back to back
    assert(m.data() + (1 * 4 + 2) == &r1[2]);              // r * cols + c
    const Matrix* cm = &m;
    assert(cm->row(1) == r1);                              // the const version

    // Step 6
    const double vals[] = {1.5, 2.5, 3.0};
    assert(row_sum(vals, 3) == 7.0);
    assert(row_sum(vals, 0) == 0.0);
    m.set(0, 2, 1.0);
    m.set(2, 2, 2.0);
    assert(col_sum(&m, 2) == 8.0);                         // 1 + 5 + 2
    assert(col_sum(&m, 0) == 0.0);
    assert(row_sum(m.row(1), m.cols()) == 5.0);

    std::cout << "All Day 06 tests passed\n";
}

/* ── Part B observations (fill in) ────────────────────────────────────────────
 B.1 *row = 0; inside row_sum (const double*):
 B.2 delete data_; instead of delete[] data_;  compiles? warns? why still wrong?
 B.3 send(NULL) error message:
*/
