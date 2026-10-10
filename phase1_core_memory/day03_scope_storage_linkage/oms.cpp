// oms.cpp: implement from scratch (Day 03)
#include "oms.h"

namespace {
// TODO: put this TU's private state here, at GLOBAL scope inside an anonymous namespace
//       (= internal linkage). Name the "total issued" counter `g_count`.
//       risk.cpp has a global with the SAME name on purpose (Part B.1 shows why that only
//       works with internal linkage).
}  // namespace

namespace oms {

// THE definition of the external-linkage variable declared in oms.h.
int session_id = 0;

std::uint64_t next_order_id() {
    // TODO: (session_id << 32) | sequence, then advance the sequence and g_count.
    return 0;
}

void reset_order_ids(std::uint64_t start) {
    // TODO
    (void)start;
}

std::uint64_t ids_issued() {
    // TODO
    return 0;
}

const Venue& primary_venue() {
    // TODO: a function-local `static const Venue`. Count how many times it is constructed
    //       (hint: initialize it from a helper function that bumps a private counter).
    static const Venue placeholder{"", 0};
    return placeholder;
}

int venue_init_count() {
    // TODO
    return -1;
}

int bump_thread_counter() {
    // TODO: use a `thread_local` variable
    return 0;
}

}  // namespace oms
