// risk.h: pre-trade checks (Day 03)
#pragma once

namespace risk {

inline constexpr int kMaxOrderQty = 10'000;

bool check_qty(int qty);    // true if 0 < qty <= kMaxOrderQty; every call is counted
int checks_done();

}  // namespace risk
