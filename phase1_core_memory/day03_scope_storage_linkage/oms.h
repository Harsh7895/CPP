// oms.h: order-management counters (Day 03)
#pragma once

#include <cstdint>
#include <string>

namespace oms {

struct Venue {
    std::string name;
    int id;
};

// EXTERNAL linkage, static storage duration. Declared here, defined once in oms.cpp.
extern int session_id;

// Order id = (session_id << 32) | sequence. The sequence counter must be INVISIBLE outside oms.cpp.
std::uint64_t next_order_id();
void reset_order_ids(std::uint64_t start = 1);   // next sequence number becomes `start`
std::uint64_t ids_issued();                      // total since program start (NOT cleared by reset)

// Function-local static: constructed lazily on the FIRST call, exactly once.
const Venue& primary_venue();                    // {"NSE", 1}
int venue_init_count();                          // how many times the Venue was constructed

// Thread storage duration: every thread has its own counter. Returns the new value.
int bump_thread_counter();

// Static data member: ONE counter shared by all Order objects (not stored inside each object).
class Order {
public:
    Order() {
        // TODO
    }
    Order(const Order&) {
        // TODO: a copy is a new live object too
    }
    Order& operator=(const Order&) = default;
    ~Order() {
        // TODO
    }

    static int live() { return live_; }          // static member function: no `this`

private:
    static inline int live_ = 0;                 // C++17: defined right here (Day 02: inline variable)
};

}  // namespace oms
