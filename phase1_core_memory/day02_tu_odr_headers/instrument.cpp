// instrument.cpp: implement the registry (Day 02)
#include "instrument.h"

// THE one definition of the variable declared `extern` in instrument.h.
long long g_lookup_count = 0;

// TODO: add storage for the table here: e.g. a fixed array of kMaxInstruments + a count.
//       Mark it `static` so it is private to this TU (Day 03 explains linkage in depth).

bool register_instrument(const Instrument& inst) {
    // TODO: reject duplicate symbols and a full table; otherwise store a copy.
    (void)inst;
    return false;
}

const Instrument* find_instrument(std::string_view symbol) {
    // TODO: increment g_lookup_count on EVERY call (hit or miss), then linear-search the table.
    (void)symbol;
    return nullptr;
}

int registered_count() {
    // TODO
    return -1;
}
