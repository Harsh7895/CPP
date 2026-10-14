// Day 05: enum class, struct vs class, using aliases, initialization forms
// Build:  g++ -std=c++20 -O2 -Wall -Wextra -pedantic impl.cpp -o impl
// Run:    ./impl
//
// Do the steps in order (README section 3 explains each one).
// The tests in main() FAIL until you fill in every TODO.

#include <cassert>
#include <cstdint>
#include <iostream>
#include <string_view>
#include <type_traits>
#include <vector>

// ─────────────────────────────────────────────────────────────────────────────
// STEP 1: make Side safe and small
// TODO: change this plain enum into an `enum class` stored in ONE byte (std::uint8_t).
// Then fill in sign() and opposite().
//   sign(Side::Buy) == +1,  sign(Side::Sell) == -1
//   opposite(Side::Buy) == Side::Sell
// ─────────────────────────────────────────────────────────────────────────────
enum Side { Buy, Sell };

constexpr int sign(Side s) {
    // TODO
    (void)s;
    return 0;
}

constexpr Side opposite(Side s) {
    // TODO
    return s;
}

// ─────────────────────────────────────────────────────────────────────────────
// STEP 2: enum -> text
// OrdType is already written for you (values match the FIX protocol, tag 40).
// Return "MKT", "LMT" or "STP". Use a switch with NO `default:`.
// ─────────────────────────────────────────────────────────────────────────────
enum class OrdType : std::uint8_t { Market = 1, Limit = 2, Stop = 3 };

constexpr std::string_view to_string(OrdType t) {
    // TODO
    (void)t;
    return "";
}

// ─────────────────────────────────────────────────────────────────────────────
// STEP 3: enum <-> one byte on the wire
//   to_wire(OrdType::Limit) == 2
//   from_wire(2, out)   -> returns true,  out == OrdType::Limit
//   from_wire(9, out)   -> returns false, out is left unchanged
// NEVER just static_cast an unchecked byte into the enum.
// ─────────────────────────────────────────────────────────────────────────────
constexpr std::uint8_t to_wire(OrdType t) {
    // TODO
    (void)t;
    return 0;
}

constexpr bool from_wire(std::uint8_t byte, OrdType& out) {
    // TODO
    (void)byte;
    (void)out;
    return false;
}

// ─────────────────────────────────────────────────────────────────────────────
// STEP 4: aliases vs a strong type
// Price and Qty are just nicknames (they are NOT new types).
// OrderId is a strong type: a real, different type.
// TODO: give `value` a default of 0, and make == compare the values.
// ─────────────────────────────────────────────────────────────────────────────
using Price = std::int64_t;   // fixed-point, 2 decimals: 2950.35 -> 295035
using Qty   = std::int32_t;

struct OrderId {
    std::uint64_t value;      // TODO: default 0

    friend constexpr bool operator==(OrderId a, OrderId b) {
        // TODO (or replace this whole function with "= default;")
        (void)a;
        (void)b;
        return false;
    }
};

// ─────────────────────────────────────────────────────────────────────────────
// STEP 5: an aggregate with default member initializers
// TODO: add defaults so that `Order{}` is:
//   id 0, price 0, qty 1, side Buy, type Limit
// Keep the member ORDER as it is (the size test and designated init depend on it).
// ─────────────────────────────────────────────────────────────────────────────
struct Order {
    OrderId id;
    Price   price;
    Qty     qty;
    Side    side;
    OrdType type;
};

// Build a sell order with DESIGNATED initializers: Order{.id = ..., ...}
// Leave `type` out so it uses its default (Limit).
Order make_sell(OrderId id, Price px, Qty q) {
    // TODO
    (void)id;
    (void)px;
    (void)q;
    return Order{};
}

// ─────────────────────────────────────────────────────────────────────────────
// STEP 6: a class protects a rule
// Rule: the net position only changes through on_fill().
//   buy 10, sell 4  -> net() == 6
// ─────────────────────────────────────────────────────────────────────────────
class Position {
public:
    void on_fill(Side side, Qty qty) {
        // TODO: use sign()
        (void)side;
        (void)qty;
    }

    Qty net() const {
        // TODO
        return -1;
    }

private:
    Qty net_ = 0;
};

// ─────────────────────────────────────────────────────────────────────────────
// STEP 7: braces vs parentheses
// Use std::vector<int>(3, 1) or std::vector<int>{3, 1}. Pick the right one!
// ─────────────────────────────────────────────────────────────────────────────
std::vector<int> make_three_ones() {      // must be {1, 1, 1}
    // TODO
    return {};
}

std::vector<int> make_three_and_one() {   // must be {3, 1}
    // TODO
    return {};
}

// ─────────────────────────────────────────────────────────────────────────────
// Tests
// ─────────────────────────────────────────────────────────────────────────────
int main() {
    // Step 1
    assert(sizeof(Side) == 1);                          // 1 byte
    assert((!std::is_convertible_v<Side, int>));        // no silent conversion to int
    assert(sign(Side::Buy) == 1);
    assert(sign(Side::Sell) == -1);
    assert(opposite(Side::Buy) == Side::Sell);
    assert(opposite(Side::Sell) == Side::Buy);

    // Step 2
    assert(to_string(OrdType::Market) == "MKT");
    assert(to_string(OrdType::Limit) == "LMT");
    assert(to_string(OrdType::Stop) == "STP");

    // Step 3
    assert(to_wire(OrdType::Limit) == 2);
    assert(to_wire(OrdType::Stop) == 3);
    OrdType t = OrdType::Market;
    assert(from_wire(2, t) && t == OrdType::Limit);
    assert(!from_wire(0, t) && t == OrdType::Limit);    // rejected, unchanged
    assert(!from_wire(9, t) && t == OrdType::Limit);
    assert(!from_wire(255, t));

    // Step 4
    assert((!std::is_convertible_v<std::uint64_t, OrderId>));   // 7 can't silently become an OrderId
    assert(sizeof(OrderId) == 8);                                // strong type = zero overhead
    assert(OrderId{7} == OrderId{7});
    assert(!(OrderId{7} == OrderId{8}));
    assert(OrderId{}.value == 0);

    // Step 5
    Order d{};
    assert(d.id == OrderId{0});
    assert(d.price == 0);
    assert(d.qty == 1);
    assert(d.side == Side::Buy);
    assert(d.type == OrdType::Limit);

    Order s = make_sell(OrderId{42}, 295035, 10);
    assert(s.id == OrderId{42});
    assert(s.price == 295035);
    assert(s.qty == 10);
    assert(s.side == Side::Sell);
    assert(s.type == OrdType::Limit);
    assert(sizeof(Order) == 24);                        // needs the 1-byte Side from Step 1

    // Step 6
    Position p;
    assert(p.net() == 0);
    p.on_fill(Side::Buy, 10);
    p.on_fill(Side::Sell, 4);
    assert(p.net() == 6);
    p.on_fill(Side::Sell, 10);
    assert(p.net() == -4);                              // short 4

    // Step 7
    assert((make_three_ones() == std::vector<int>{1, 1, 1}));
    assert((make_three_and_one().size() == 2));
    assert(make_three_and_one()[0] == 3 && make_three_and_one()[1] == 1);

    std::cout << "All Day 05 tests passed\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// STEP 8 (final check): once all the tests pass, UNCOMMENT these lines.
// They prove your functions work at compile time and lock the sizes.
// ─────────────────────────────────────────────────────────────────────────────
// static_assert(sign(Side::Sell) == -1);
// static_assert(to_string(OrdType::Stop) == "STP");
// static_assert(to_wire(OrdType::Market) == 1);
// static_assert(sizeof(Order) == 24);
// static_assert(std::is_aggregate_v<Order>);
// static_assert(!std::is_aggregate_v<Position>);

/* ── Part B observations (fill in) ────────────────────────────────────────────
 B.1 enum Signal { Buy, Hold }; with plain Side, then with enum class Side:
 B.2 int x = Side::Buy; after Step 1:
 B.3 std::int32_t q{2.5};  vs  std::int32_t q2 = 2.5;
 B.4 .qty before .price in make_sell:
 B.5 send(Qty{10}, Price{295035}) with aliases: compiles? why scary?
*/
