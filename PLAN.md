# 90-Day C++ Plan — Quant Developer / HFT Track

Time budget: **1–1.5 hrs/day**, 1–2 topics per day.
Every day follows the 9-section format (Theory → Internals → Implementation → Output Qs → Debugging → Performance → Interview Qs → Hard Qs → Mini Project).
Full syllabus: [skill.md](skill.md). Progress: [PROGRESS.md](PROGRESS.md).

Folder layout: `<phase folder>/dayNN_<short_topic>/` — created on the day it is taught.

---

## Phase 1 — Core Language & Memory → `phase1_core_memory/`

| Day | Topic | Build from scratch |
|----|-------|--------------------|
| 01 | Compilation pipeline: preprocess → compile → assemble → link; static vs dynamic linking | Multi-file program, inspect each stage's output |
| 02 | Translation units, declarations vs definitions, ODR, headers, include guards, `#pragma once` | Break & fix ODR / linker errors on purpose |
| 03 | Namespaces, scope, storage duration, linkage, `static`, `extern` | Counter module with internal vs external linkage |
| 04 | `const`, `constexpr`, `consteval`, `constinit`, `volatile`, `mutable` | Compile-time lookup table |
| 05 | `enum` vs `enum class`, struct vs class, `using`/aliases, all initialization forms | Initialization-forms cheat-sheet program |
| 06 | Raw pointers, pointer arithmetic, pointer-to-pointer, const pointer variants, `nullptr` vs `NULL` | Manual 2D dynamic array |
| 07 | References (lvalue/rvalue), function pointers, pointer-to-member | Callback-based event dispatcher (function ptrs) |
| 08 | Dangling/wild pointers, ownership, non-owning pointers, invalidation, strict aliasing | Bug hunt: 10 pointer UB cases |
| 09 | Process memory layout: stack, heap, static, text/data/BSS | Print addresses of every segment, explain them |
| 10 | Virtual memory: pages, page tables, TLB, page faults, `mmap` | Measure page-fault cost (touch vs pre-fault) |
| 11 | `malloc/calloc/realloc/free` vs `new/delete`, `new[]/delete[]`, placement new | Tiny bump allocator using placement new |
| 12 | Alignment, padding, `sizeof`/`alignof`, object representation, `std::align`, fragmentation | Struct reordering to minimize padding |
| 13 | Object lifetime: creation/destruction, temporaries, lifetime extension | Lifetime tracer class (logs ctor/dtor) |
| 14 | Init order: static vs dynamic init, static-init-order fiasco, destruction order, explicit dtor calls, `std::launder` | Reproduce + fix static-init-order fiasco |
| 15 | **REVIEW + MOCK INTERVIEW** (Days 1–14) | Timed quiz + redo weakest implementation |

## Phase 2 — OOP, RAII, Move Semantics, Smart Pointers → `phase2_oop_raii_move/`

| Day | Topic | Build from scratch |
|----|-------|--------------------|
| 16 | Encapsulation, abstraction, inheritance, composition/aggregation/association | Model an Order/Instrument hierarchy |
| 17 | Virtual functions, pure virtual, abstract classes, `override`/`final`, overloading vs overriding vs hiding | Strategy interface for a backtester |
| 18 | vtable/vptr, object layout, virtual destructor, object slicing | Hand-rolled vtable in C-style C++ |
| 19 | Multiple inheritance, diamond problem, virtual inheritance | Inspect layouts with `sizeof` + offsets |
| 20 | Constructors: default/param/copy, delegating, `explicit`, initializer lists, member init order | Class with every ctor kind, traced |
| 21 | Destructors, `=default`/`=delete`, constructor exception safety | Exception-safe two-resource class |
| 22 | RAII, Rule of 0/3/5, resource ownership | `FileHandle`, `ScopedTimer` RAII wrappers |
| 23 | Value categories: lvalue, rvalue, xvalue, prvalue, glvalue | Overload-resolution probe program |
| 24 | Move semantics, `std::move`, move ctor/assign, self-assignment, moved-from state, `noexcept` move | `Buffer` class with Rule of 5 |
| 25 | Copy elision, RVO/NRVO, `std::forward`, perfect forwarding | `make_unique`-style factory with forwarding |
| 26 | `unique_ptr`, `make_unique`, custom deleters | **MyUniquePtr** |
| 27 | `shared_ptr`, control block, `make_shared`, reference counting | **MySharedPtr** |
| 28 | `weak_ptr`, cycles, `enable_shared_from_this`, aliasing ctor | **MyWeakPtr** (extend MySharedPtr) |
| 29 | Casts: `static/dynamic/const/reinterpret_cast`, RTTI, `typeid`, CRTP vs virtual | CRTP vs virtual dispatch benchmark |
| 30 | **REVIEW + MOCK INTERVIEW** (Days 16–29) | Re-implement MySharedPtr from memory, timed |

## Phase 3 — STL, Data Structures, Templates → `phase3_stl_templates/`

| Day | Topic | Build from scratch |
|----|-------|--------------------|
| 31 | STL containers overview, complexity table, container adaptors, choosing a container | Container-choice benchmark |
| 32 | vector deep dive: layout, size vs capacity, growth, reserve/resize/shrink, iterator invalidation | Growth-strategy experiment |
| 33 | **MyVector** part 1: raw storage, push_back, reallocation with `move_if_noexcept` | **MyVector** |
| 34 | **MyVector** part 2: emplace_back, insert, erase, exception safety; bench vs `std::vector` | **MyVector** (complete) |
| 35 | string: C strings, SSO, growth, COW history, `string_view` | **MyString** (with SSO) |
| 36 | list, forward_list, deque internals; stack/queue adaptors | **MyLinkedList**, **MyStack**, **MyQueue** |
| 37 | Iterators: categories, traits, C++20 iterator concepts, custom iterators | Iterators for MyVector + MyLinkedList |
| 38 | Hash tables: hash functions, collisions, chaining vs open addressing, probing, load factor, rehash, hash attacks | Hash-quality experiment |
| 39 | **MyHashMap** (separate chaining, `unordered_map`-like) | **MyHashMap**, **MyHashSet** |
| 40 | Open-addressing flat hash map (linear probing / Robin Hood); bench vs `unordered_map` | Flat hash map |
| 41 | BST, AVL, rotations, tree height | **MyBST**, **MyAVLTree** |
| 42 | Red-Black tree, map/set internals, `lower_bound`/`upper_bound` | **MyRBTree** (insert + lookup) |
| 43 | Heaps: binary heap, heapify, decrease-key, d-ary heap, `priority_queue` internals | **MyPriorityQueue** |
| 44 | STL algorithms: sort family, `nth_element`, binary-search family, erase-remove, partition, rotate | Re-implement 8 algorithms |
| 45 | Sorting: quick/merge/heap/insertion, introsort, pivot selection, cache behavior | Introsort |
| 46 | Templates 1: function/class templates, deduction, full/partial specialization, NTTP | Fixed-capacity `StaticVector<T, N>` |
| 47 | Templates 2: variadic, parameter packs, fold expressions, SFINAE, `enable_if`, type_traits | Type-safe `print(...)` + own traits |
| 48 | Templates 3: concepts, `requires`, CRTP, compile-time programming | Concept-constrained containers |
| 49 | Lambdas: captures, init capture, generic/mutable lambdas, lifetime, `std::function`, `std::invoke` | **MyFunction** (type erasure + SBO) |
| 50 | **REVIEW + MOCK INTERVIEW** (Days 31–49) | Implement vector + hashmap under time pressure |

## Phase 4 — Modern C++ & Error Handling → `phase4_modern_cpp/`

| Day | Topic | Build from scratch |
|----|-------|--------------------|
| 51 | `auto`, `decltype`, `decltype(auto)`, structured bindings, `if constexpr`, inline vars, attributes | Refactor old code to modern C++ |
| 52 | `optional`, `variant`, `any`, `expected`, `span` | **MyOptional** |
| 53 | Ranges & views, lazy evaluation, pipelines, projections, `chrono` | Range pipeline over tick data |
| 54 | Exceptions, stack unwinding, `noexcept`, basic/strong/no-throw guarantees, `error_code`, `expected` | Strong-guarantee `MyVector::insert` |
| 55 | LRU + LFU cache | **MyLRUCache**, **MyLFUCache** |

## Phase 5 — Concurrency → `phase5_concurrency/`

| Day | Topic | Build from scratch |
|----|-------|--------------------|
| 56 | Process vs thread, `std::thread`, join/detach, `thread_local`, `jthread`, `stop_token` | Parallel sum with N threads |
| 57 | `mutex`, `recursive/timed/shared_mutex`, `lock_guard`, `unique_lock`, `scoped_lock` | Thread-safe bank account |
| 58 | Deadlock, livelock, starvation, lock ordering, hierarchical mutex | **HierarchicalMutex** |
| 59 | `condition_variable`, predicates, spurious wakeups, notify_one/all, producer-consumer | **ProducerConsumer** |
| 60 | Thread-safe & bounded blocking queues | **ThreadSafeQueue**, **BlockingQueue** |
| 61 | `future`, `promise`, `packaged_task`, `async`, `shared_future` | Async pipeline |
| 62 | Thread pool design: workers, task queue, shutdown | **ThreadPool** |
| 63 | Thread pool 2: `submit()` returning futures, work stealing, graceful cancellation | **ThreadPool** (futures + stealing) |
| 64 | Atomics: `std::atomic`, `atomic_flag`, `fetch_add`, `exchange`, CAS | **SpinLock** |
| 65 | Memory model 1: data race vs race condition, sequenced-before, happens-before, synchronizes-with, seq_cst | Litmus tests |
| 66 | Memory model 2: relaxed, acquire/release, fences, compiler vs CPU reordering | Message-passing with acq/rel |
| 67 | Lock-free: CAS loops, ABA problem, lock-free stack | **LockFreeStack** |
| 68 | SPSC ring buffer, cache-line padding, false sharing | **SPSCQueue** / **MyRingBuffer** |
| 69 | MPSC/MPMC queues, memory reclamation: hazard pointers, epochs, RCU (concepts) | **MPSCQueue** |
| 70 | Concurrent hash map (lock striping), thread-safe LRU | **ConcurrentHashMap** |
| 71 | Async logger + rate limiter (token bucket) | **AsyncLogger**, **RateLimiter** |
| 72 | **REVIEW + MOCK INTERVIEW** (Days 56–71) | Design a thread pool / bounded queue live |

## Phase 6 — Performance & Systems → `phase6_performance_systems/`

| Day | Topic | Build from scratch |
|----|-------|--------------------|
| 73 | CPU cache: L1/L2/L3, cache lines, locality, associativity, prefetching | Row vs column traversal, AoS vs SoA bench |
| 74 | Cache coherence, MESI, false/true sharing, NUMA | False-sharing benchmark |
| 75 | Pipeline, OoO/speculative execution, ILP, branch prediction, branchless code | Sorted vs unsorted branch bench |
| 76 | SIMD: auto-vectorization, SSE/AVX2 intrinsics, reductions | SIMD sum / min-max |
| 77 | Toolchain: GCC/Clang/MSVC, -O levels, LTO, PGO, inlining, CMake, static/shared libs, `nm`/`objdump` | CMake project + read assembly |
| 78 | Profiling & benchmarking: Google Benchmark, p50/p95/p99/p99.9, perf, flame graphs, `rdtsc` | Latency histogram tool |
| 79 | Debugging & UB: GDB, ASan/UBSan/TSan, UB catalog | Find 10 planted bugs with tools |
| 80 | Allocators: `std::allocator`, `allocator_traits`, custom allocators, PMR | **ArenaAllocator** |
| 81 | Memory pool, object pool, free lists, fragmentation | **MemoryPool**, **ObjectPool**, **FreeListAllocator** |
| 82 | Zero-copy & low latency: preallocation, no-alloc hot path, batching, cache-aware layout | Allocation-free hot path refactor |
| 83 | Networking: sockets, TCP/UDP, blocking vs non-blocking, select/poll/epoll, `TCP_NODELAY` | **TCP Server** (echo), **UDP Server** |
| 84 | Serialization: binary vs text, endianness, fixed vs variable-length, zero-copy parsing | **BinaryProtocolParser** |

## Phase 7 — HFT Capstone → `phase7_hft_capstone/`

| Day | Topic | Build from scratch |
|----|-------|--------------------|
| 85 | Limit order book: price-time priority, data-structure choices | **LimitOrderBook** part 1 |
| 86 | Order book: O(1) add/cancel/modify, cache-friendly price levels, benchmark | **LimitOrderBook** part 2 |
| 87 | Matching engine | **MatchingEngine** |
| 88 | Market data feed handler: parser → SPSC queue → book | **MarketDataParser**, **FeedHandler** |
| 89 | Low-latency engine: event-driven design, CPU affinity, busy polling, timestamping, jitter, kernel bypass | **LowLatencyTradingEngine** (integrate + measure) |
| 90 | **FINAL REVIEW**: C++ interview traps + full mock interview | — |

---

## Backlog (after Day 90, or extra weekend sessions)
Coroutines · Modules · Design patterns (Singleton/Factory/Observer/Visitor) · ABI & name mangling · File I/O & `mmap` · Linux processes/IPC/signals · MyDeque · MyTrie · MyBloomFilter · MySkipList · MyVariant · TimerWheel · EventLoop · ConnectionPool · PubSub · OrderGateway · LockFreeQueue (MPMC) · HTTP Server
