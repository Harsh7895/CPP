// odr_other.cpp: the second translation unit for exercises.cpp. Don't peek before predicting!

// Q2: same two lines as exercises.cpp (as if both included one header)
const int kLimit = 100;
inline const int kInlineLimit = 100;
const int* other_klimit_addr() { return &kLimit; }
const int* other_kinline_addr() { return &kInlineLimit; }

// Q3: a DIFFERENT body for the same inline function (ODR violation, no diagnostic required)
inline int fee_bps() { return 2; }
int other_fee() { return fee_bps(); }

// Q5
int counter = 7;
int bump() { return ++counter; }

#ifdef BUG3
static int g_orders = 0;
void record_order_other() { ++g_orders; }
int orders_seen_by_other() { return g_orders; }
#endif
