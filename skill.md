C++
│
├── 01. C++ FUNDAMENTALS
│ ├── compilation pipeline
│ ├── preprocessing
│ ├── compilation
│ ├── assembly
│ ├── linking
│ ├── static linking
│ ├── dynamic linking
│ ├── translation units
│ ├── declarations vs definitions
│ ├── header files
│ ├── include guards
│ ├── #pragma once
│ ├── namespaces
│ ├── scope
│ ├── storage duration
│ ├── lifetime
│ ├── linkage
│ ├── const
│ ├── constexpr
│ ├── consteval
│ ├── constinit
│ ├── static
│ ├── extern
│ ├── volatile
│ ├── mutable
│ ├── type aliases
│ ├── using
│ ├── enum
│ ├── enum class
│ ├── structs
│ ├── classes
│ └── initialization forms
│
├── 02. POINTERS & REFERENCES
│ ├── raw pointers
│ ├── pointer arithmetic
│ ├── pointer to pointer
│ ├── pointer to function
│ ├── pointer to member
│ ├── references
│ ├── lvalue references
│ ├── rvalue references
│ ├── const pointer
│ ├── pointer to const
│ ├── const pointer to const
│ ├── nullptr
│ ├── NULL vs nullptr
│ ├── dangling pointers
│ ├── wild pointers
│ ├── ownership
│ ├── non-owning pointers
│ ├── pointer lifetime
│ ├── pointer invalidation
│ └── strict aliasing
│
├── 03. MEMORY MODEL
│ ├── stack
│ ├── heap
│ ├── static storage
│ ├── global memory
│ ├── code/text segment
│ ├── data segment
│ ├── BSS
│ ├── virtual memory
│ ├── virtual address
│ ├── physical address
│ ├── pages
│ ├── page tables
│ ├── TLB
│ ├── page faults
│ ├── memory mapping
│ ├── mmap
│ ├── malloc
│ ├── calloc
│ ├── realloc
│ ├── free
│ ├── new
│ ├── delete
│ ├── new[]
│ ├── delete[]
│ ├── placement new
│ ├── custom allocation
│ ├── alignment
│ ├── padding
│ ├── object representation
│ ├── sizeof
│ ├── alignof
│ ├── std::align
│ └── memory fragmentation
│
├── 04. OBJECT LIFETIME
│ ├── object creation
│ ├── object destruction
│ ├── lifetime rules
│ ├── temporary objects
│ ├── temporary lifetime extension
│ ├── initialization order
│ ├── static initialization
│ ├── dynamic initialization
│ ├── destruction order
│ ├── placement new
│ ├── explicit destructor calls
│ ├── object reuse
│ ├── std::launder
│ └── lifetime-related undefined behavior
│
├── 05. OOP & CLASS DESIGN
│ ├── encapsulation
│ ├── abstraction
│ ├── inheritance
│ ├── composition
│ ├── aggregation
│ ├── association
│ ├── polymorphism
│ ├── compile-time polymorphism
│ ├── runtime polymorphism
│ ├── virtual functions
│ ├── pure virtual functions
│ ├── abstract classes
│ ├── virtual destructor
│ ├── vtable
│ ├── vptr
│ ├── object layout
│ ├── multiple inheritance
│ ├── diamond problem
│ ├── virtual inheritance
│ ├── method hiding
│ ├── overriding
│ ├── overloading
│ ├── final
│ ├── override
│ └── CRTP
│
├── 06. CONSTRUCTORS & DESTRUCTORS
│ ├── default constructor
│ ├── parameterized constructor
│ ├── copy constructor
│ ├── move constructor
│ ├── destructor
│ ├── delegating constructors
│ ├── explicit constructors
│ ├── initializer lists
│ ├── member initialization order
│ ├── defaulted functions
│ ├── deleted functions
│ ├── constructor exception safety
│ ├── RAII
│ ├── Rule of 0
│ ├── Rule of 3
│ ├── Rule of 5
│ └── resource ownership
│
├── 07. COPY & MOVE SEMANTICS
│ ├── lvalue
│ ├── rvalue
│ ├── xvalue
│ ├── prvalue
│ ├── glvalue
│ ├── copy semantics
│ ├── move semantics
│ ├── std::move
│ ├── std::forward
│ ├── copy elision
│ ├── RVO
│ ├── NRVO
│ ├── move assignment
│ ├── self assignment
│ ├── moved-from objects
│ ├── noexcept move constructor
│ └── perfect forwarding
│
├── 08. SMART POINTERS
│ ├── unique*ptr
│ ├── shared_ptr
│ ├── weak_ptr
│ ├── make_unique
│ ├── make_shared
│ ├── ownership semantics
│ ├── reference counting
│ ├── control block
│ ├── strong reference
│ ├── weak reference
│ ├── cyclic references
│ ├── custom deleters
│ ├── aliasing constructor
│ ├── enable_shared_from_this
│ ├── atomic shared_ptr
│ └── smart pointer implementation
│
├── 09. STL CONTAINERS
│ ├── array
│ ├── vector
│ ├── deque
│ ├── list
│ ├── forward_list
│ ├── stack
│ ├── queue
│ ├── priority_queue
│ ├── set
│ ├── multiset
│ ├── map
│ ├── multimap
│ ├── unordered_set
│ ├── unordered_multiset
│ ├── unordered_map
│ ├── unordered_multimap
│ ├── string
│ ├── string_view
│ └── container adaptors
│
├── 10. VECTOR — DEEP IMPLEMENTATION
│ ├── dynamic array
│ ├── size vs capacity
│ ├── growth strategy
│ ├── reserve
│ ├── resize
│ ├── shrink_to_fit
│ ├── push_back
│ ├── emplace_back
│ ├── insert
│ ├── erase
│ ├── reallocation
│ ├── iterator invalidation
│ ├── exception safety
│ ├── allocator usage
│ ├── move vs copy during growth
│ ├── contiguous memory
│ ├── cache locality
│ └── IMPLEMENT YOUR OWN VECTOR
│
├── 11. STRING — DEEP IMPLEMENTATION
│ ├── C strings
│ ├── std::string
│ ├── string capacity
│ ├── SSO
│ ├── string growth
│ ├── copy-on-write history
│ ├── string_view
│ ├── null termination
│ ├── iterator invalidation
│ ├── allocation behavior
│ └── IMPLEMENT YOUR OWN STRING
│
├── 12. HASH TABLES
│ ├── hashing
│ ├── hash functions
│ ├── collision
│ ├── separate chaining
│ ├── open addressing
│ ├── linear probing
│ ├── quadratic probing
│ ├── double hashing
│ ├── load factor
│ ├── rehashing
│ ├── bucket count
│ ├── iterator invalidation
│ ├── worst-case complexity
│ ├── hash attacks
│ └── IMPLEMENT YOUR OWN UNORDERED_MAP
│
├── 13. TREE-BASED CONTAINERS
│ ├── BST
│ ├── AVL tree
│ ├── Red-Black tree
│ ├── rotations
│ ├── balancing
│ ├── tree height
│ ├── ordered traversal
│ ├── lower_bound
│ ├── upper_bound
│ ├── map internals
│ ├── set internals
│ └── IMPLEMENT YOUR OWN ORDERED MAP
│
├── 14. HEAPS
│ ├── binary heap
│ ├── min heap
│ ├── max heap
│ ├── heapify
│ ├── push
│ ├── pop
│ ├── decrease key
│ ├── priority_queue internals
│ ├── d-ary heap
│ └── IMPLEMENT YOUR OWN PRIORITY_QUEUE
│
├── 15. ITERATORS
│ ├── iterator concept
│ ├── input iterator
│ ├── output iterator
│ ├── forward iterator
│ ├── bidirectional iterator
│ ├── random access iterator
│ ├── contiguous iterator
│ ├── iterator traits
│ ├── custom iterators
│ ├── iterator invalidation
│ └── IMPLEMENT YOUR OWN ITERATOR
│
├── 16. STL ALGORITHMS
│ ├── sort
│ ├── stable_sort
│ ├── partial_sort
│ ├── nth_element
│ ├── binary_search
│ ├── lower_bound
│ ├── upper_bound
│ ├── equal_range
│ ├── find
│ ├── find_if
│ ├── count
│ ├── accumulate
│ ├── reduce
│ ├── transform
│ ├── copy
│ ├── move
│ ├── remove
│ ├── erase-remove idiom
│ ├── partition
│ ├── rotate
│ ├── reverse
│ ├── min/max
│ └── permutation algorithms
│
├── 17. SORTING IMPLEMENTATION
│ ├── bubble sort
│ ├── insertion sort
│ ├── selection sort
│ ├── merge sort
│ ├── quicksort
│ ├── heapsort
│ ├── introsort
│ ├── stable sorting
│ ├── partition schemes
│ ├── pivot selection
│ ├── worst-case behavior
│ ├── recursion depth
│ ├── cache behavior
│ └── IMPLEMENT YOUR OWN SORT
│
├── 18. TEMPLATES
│ ├── function templates
│ ├── class templates
│ ├── template specialization
│ ├── partial specialization
│ ├── template deduction
│ ├── non-type template parameters
│ ├── variadic templates
│ ├── parameter packs
│ ├── fold expressions
│ ├── SFINAE
│ ├── enable_if
│ ├── type_traits
│ ├── concepts
│ ├── requires
│ ├── CRTP
│ ├── template metaprogramming
│ └── compile-time programming
│
├── 19. LAMBDA & FUNCTIONAL C++
│ ├── lambda syntax
│ ├── capture by value
│ ├── capture by reference
│ ├── init capture
│ ├── generic lambdas
│ ├── mutable lambdas
│ ├── lambda lifetime
│ ├── std::function
│ ├── function objects
│ ├── functors
│ ├── std::bind
│ ├── std::invoke
│ └── callable objects
│
├── 20. MODERN C++
│ ├── auto
│ ├── decltype
│ ├── decltype(auto)
│ ├── structured bindings
│ ├── nullptr
│ ├── enum class
│ ├── range-based for
│ ├── constexpr
│ ├── consteval
│ ├── constinit
│ ├── if constexpr
│ ├── inline variables
│ ├── [[nodiscard]]
│ ├── [[maybe_unused]]
│ ├── attributes
│ ├── modules
│ ├── concepts
│ ├── ranges
│ ├── span
│ ├── optional
│ ├── variant
│ ├── any
│ ├── expected
│ ├── filesystem
│ └── chrono
│
├── 21. RANGES
│ ├── ranges::begin
│ ├── ranges::end
│ ├── views
│ ├── filter
│ ├── transform
│ ├── take
│ ├── drop
│ ├── reverse
│ ├── lazy evaluation
│ ├── range pipelines
│ ├── projections
│ └── custom views
│
├── 22. ERROR HANDLING
│ ├── exceptions
│ ├── exception hierarchy
│ ├── throw
│ ├── catch
│ ├── noexcept
│ ├── exception specifications
│ ├── stack unwinding
│ ├── exception safety
│ ├── basic guarantee
│ ├── strong guarantee
│ ├── no-throw guarantee
│ ├── std::error_code
│ ├── std::expected
│ └── error handling design
│
├── 23. CONCURRENCY FUNDAMENTALS
│ ├── process vs thread
│ ├── concurrency vs parallelism
│ ├── std::thread
│ ├── thread creation
│ ├── thread joining
│ ├── detach
│ ├── thread lifetime
│ ├── thread_local
│ ├── hardware_concurrency
│ ├── std::jthread
│ ├── stop_token
│ └── thread safety
│
├── 24. MUTEX & SYNCHRONIZATION
│ ├── mutex
│ ├── recursive_mutex
│ ├── timed_mutex
│ ├── shared_mutex
│ ├── lock_guard
│ ├── unique_lock
│ ├── scoped_lock
│ ├── try_lock
│ ├── deadlock
│ ├── livelock
│ ├── starvation
│ ├── lock ordering
│ ├── hierarchical mutex
│ └── synchronization design
│
├── 25. CONDITION VARIABLES
│ ├── condition_variable
│ ├── wait
│ ├── wait_for
│ ├── wait_until
│ ├── predicates
│ ├── spurious wakeups
│ ├── notify_one
│ ├── notify_all
│ ├── producer-consumer
│ ├── bounded queue
│ └── IMPLEMENT THREAD-SAFE QUEUE
│
├── 26. ATOMICS
│ ├── std::atomic
│ ├── atomic operations
│ ├── atomic_flag
│ ├── compare_exchange
│ ├── CAS
│ ├── fetch_add
│ ├── fetch_sub
│ ├── exchange
│ ├── test-and-set
│ ├── atomic pointers
│ ├── atomic shared_ptr
│ └── lock-free vs wait-free
│
├── 27. C++ MEMORY MODEL
│ ├── data races
│ ├── race conditions
│ ├── happens-before
│ ├── synchronizes-with
│ ├── sequenced-before
│ ├── memory_order_relaxed
│ ├── memory_order_acquire
│ ├── memory_order_release
│ ├── acquire-release
│ ├── memory_order_seq_cst
│ ├── fences
│ ├── visibility
│ ├── reordering
│ └── compiler vs CPU reordering
│
├── 28. LOCK-FREE PROGRAMMING
│ ├── CAS loops
│ ├── ABA problem
│ ├── lock-free stack
│ ├── lock-free queue
│ ├── SPSC queue
│ ├── MPSC queue
│ ├── MPMC queue
│ ├── memory reclamation
│ ├── hazard pointers
│ ├── epoch-based reclamation
│ ├── RCU concepts
│ └── false sharing
│
├── 29. THREAD POOLS
│ ├── worker threads
│ ├── task queue
│ ├── work stealing
│ ├── task scheduling
│ ├── futures
│ ├── promises
│ ├── packaged_task
│ ├── std::async
│ ├── thread pool shutdown
│ ├── graceful cancellation
│ └── IMPLEMENT YOUR OWN THREAD POOL
│
├── 30. ASYNC PROGRAMMING
│ ├── future
│ ├── promise
│ ├── packaged_task
│ ├── async
│ ├── shared_future
│ ├── continuations
│ ├── cancellation
│ └── async task design
│
├── 31. COROUTINES
│ ├── coroutine basics
│ ├── co_await
│ ├── co_yield
│ ├── co_return
│ ├── promise_type
│ ├── coroutine frame
│ ├── suspension
│ ├── generators
│ └── coroutine performance
│
├── 32. CACHE & CPU ARCHITECTURE
│ ├── CPU cache
│ ├── L1
│ ├── L2
│ ├── L3
│ ├── cache lines
│ ├── cache hit
│ ├── cache miss
│ ├── spatial locality
│ ├── temporal locality
│ ├── cache associativity
│ ├── prefetching
│ ├── hardware prefetcher
│ ├── cache coherence
│ ├── MESI
│ ├── false sharing
│ ├── true sharing
│ └── NUMA
│
├── 33. CPU PERFORMANCE
│ ├── instruction pipeline
│ ├── superscalar execution
│ ├── out-of-order execution
│ ├── speculative execution
│ ├── branch prediction
│ ├── branch misprediction
│ ├── instruction-level parallelism
│ ├── CPU cycles
│ ├── CPI
│ ├── latency vs throughput
│ ├── CPU affinity
│ ├── hyperthreading
│ └── context switching
│
├── 34. BRANCH PREDICTION
│ ├── conditional branches
│ ├── predictable branches
│ ├── unpredictable branches
│ ├── branchless programming
│ ├── lookup-table techniques
│ └── measuring branch misprediction
│
├── 35. SIMD
│ ├── SIMD concept
│ ├── vectorization
│ ├── SSE
│ ├── AVX
│ ├── AVX2
│ ├── AVX-512
│ ├── compiler auto-vectorization
│ ├── alignment
│ ├── intrinsics
│ ├── SIMD reductions
│ └── SIMD performance measurement
│
├── 36. ALLOCATORS & MEMORY POOLS
│ ├── std::allocator
│ ├── allocator_traits
│ ├── custom allocators
│ ├── arena allocation
│ ├── pool allocation
│ ├── slab allocation
│ ├── object pools
│ ├── freelists
│ ├── monotonic allocation
│ ├── allocation overhead
│ ├── fragmentation
│ └── IMPLEMENT MEMORY POOL
│
├── 37. ZERO-COPY & LOW-LATENCY C++
│ ├── zero-copy
│ ├── move instead of copy
│ ├── object reuse
│ ├── memory pools
│ ├── preallocation
│ ├── contiguous structures
│ ├── avoiding dynamic allocation
│ ├── cache-aware structures
│ ├── branch reduction
│ ├── batching
│ ├── ring buffers
│ └── latency measurement
│
├── 38. DATA STRUCTURES — IMPLEMENT FROM SCRATCH
│ ├── dynamic array
│ ├── linked list
│ ├── doubly linked list
│ ├── stack
│ ├── queue
│ ├── circular queue
│ ├── deque
│ ├── hash table
│ ├── BST
│ ├── AVL
│ ├── Red-Black tree
│ ├── heap
│ ├── trie
│ ├── LRU cache
│ ├── LFU cache
│ ├── bloom filter
│ ├── skip list
│ └── ring buffer
│
├── 39. IMPORTANT SYSTEM DESIGN IMPLEMENTATIONS
│ ├── LRU cache
│ ├── thread-safe LRU cache
│ ├── LFU cache
│ ├── bounded blocking queue
│ ├── thread pool
│ ├── logger
│ ├── async logger
│ ├── rate limiter
│ ├── connection pool
│ ├── memory pool
│ ├── object pool
│ ├── event bus
│ ├── publish-subscribe system
│ ├── scheduler
│ ├── timer wheel
│ └── producer-consumer pipeline
│
├── 40. NETWORKING IN C++
│ ├── sockets
│ ├── TCP
│ ├── UDP
│ ├── blocking sockets
│ ├── non-blocking sockets
│ ├── select
│ ├── poll
│ ├── epoll
│ ├── event-driven I/O
│ ├── socket buffers
│ ├── serialization
│ ├── deserialization
│ ├── endianness
│ ├── network byte order
│ ├── TCP_NODELAY
│ ├── connection lifecycle
│ └── IMPLEMENT TCP SERVER
│
├── 41. SERIALIZATION
│ ├── binary serialization
│ ├── text serialization
│ ├── POD serialization
│ ├── alignment issues
│ ├── endianness
│ ├── zero-copy parsing
│ ├── fixed-width messages
│ ├── variable-length messages
│ └── serialization performance
│
├── 42. FILE I/O
│ ├── ifstream
│ ├── ofstream
│ ├── fstream
│ ├── buffered I/O
│ ├── unbuffered I/O
│ ├── mmap
│ ├── file descriptors
│ ├── sequential I/O
│ ├── random I/O
│ ├── direct I/O
│ └── asynchronous I/O concepts
│
├── 43. LINUX / SYSTEM PROGRAMMING
│ ├── processes
│ ├── fork
│ ├── exec
│ ├── wait
│ ├── signals
│ ├── pipes
│ ├── shared memory
│ ├── IPC
│ ├── file descriptors
│ ├── system calls
│ ├── context switches
│ ├── process scheduling
│ └── thread scheduling
│
├── 44. BUILD SYSTEM & TOOLCHAIN
│ ├── GCC
│ ├── Clang
│ ├── MSVC
│ ├── compiler flags
│ ├── optimization flags
│ ├── -O0
│ ├── -O1
│ ├── -O2
│ ├── -O3
│ ├── LTO
│ ├── debug vs release
│ ├── CMake
│ ├── static libraries
│ ├── shared libraries
│ ├── object files
│ ├── symbol tables
│ ├── nm
│ ├── objdump
│ ├── readelf
│ └── linker errors
│
├── 45. DEBUGGING
│ ├── GDB
│ ├── breakpoints
│ ├── watchpoints
│ ├── stack traces
│ ├── core dumps
│ ├── debugging optimized code
│ ├── AddressSanitizer
│ ├── UndefinedBehaviorSanitizer
│ ├── ThreadSanitizer
│ ├── LeakSanitizer
│ └── static analysis
│
├── 46. UNDEFINED BEHAVIOR
│ ├── dangling pointer
│ ├── use-after-free
│ ├── double free
│ ├── buffer overflow
│ ├── signed integer overflow
│ ├── invalid iterator
│ ├── data race
│ ├── uninitialized memory
│ ├── strict aliasing violations
│ ├── lifetime violations
│ ├── invalid casts
│ └── UB optimization consequences
│
├── 47. TYPE SYSTEM
│ ├── implicit conversions
│ ├── explicit conversions
│ ├── static_cast
│ ├── dynamic_cast
│ ├── const_cast
│ ├── reinterpret_cast
│ ├── user-defined conversions
│ ├── conversion operators
│ ├── type deduction
│ ├── decltype
│ ├── type_traits
│ └── std::is*\* traits
│
├── 48. CASTS & POLYMORPHISM
│ ├── static_cast
│ ├── dynamic_cast
│ ├── const_cast
│ ├── reinterpret_cast
│ ├── RTTI
│ ├── typeid
│ ├── vtable
│ ├── vptr
│ └── polymorphic destruction
│
├── 49. COMPILER OPTIMIZATION
│ ├── dead-code elimination
│ ├── constant folding
│ ├── constant propagation
│ ├── inlining
│ ├── loop unrolling
│ ├── vectorization
│ ├── common subexpression elimination
│ ├── copy elision
│ ├── link-time optimization
│ ├── PGO
│ └── compiler-generated code
│
├── 50. PROFILING & BENCHMARKING
│ ├── microbenchmarks
│ ├── Google Benchmark
│ ├── wall-clock time
│ ├── CPU time
│ ├── latency
│ ├── throughput
│ ├── percentiles
│ ├── p50
│ ├── p95
│ ├── p99
│ ├── p99.9
│ ├── flame graphs
│ ├── perf
│ ├── cachegrind
│ ├── VTune
│ └── profiling methodology
│
├── 51. DESIGN PATTERNS IN C++
│ ├── Singleton
│ ├── Factory
│ ├── Abstract Factory
│ ├── Builder
│ ├── Adapter
│ ├── Decorator
│ ├── Observer
│ ├── Strategy
│ ├── Command
│ ├── State
│ ├── Visitor
│ ├── Template Method
│ ├── dependency injection
│ └── type erasure
│
├── 52. TYPE ERASURE
│ ├── std::function
│ ├── std::any
│ ├── virtual dispatch
│ ├── type-erased wrappers
│ ├── small object optimization
│ └── IMPLEMENT TYPE-ERASED CONTAINER
│
├── 53. ABI & BINARY COMPATIBILITY
│ ├── ABI
│ ├── API
│ ├── name mangling
│ ├── calling conventions
│ ├── object layout
│ ├── vtables
│ ├── symbol visibility
│ ├── shared library ABI
│ └── binary compatibility
│
├── 54. ADVANCED STL INTERNALS
│ ├── vector implementation
│ ├── string implementation
│ ├── deque implementation
│ ├── list implementation
│ ├── map implementation
│ ├── unordered_map implementation
│ ├── set implementation
│ ├── priority_queue implementation
│ ├── allocator integration
│ ├── iterator implementation
│ ├── iterator invalidation
│ ├── complexity guarantees
│ └── exception guarantees
│
├── 55. C++ INTERVIEW TRAPS
│ ├── sizeof empty class
│ ├── sizeof derived class
│ ├── object slicing
│ ├── virtual destructor
│ ├── dangling references
│ ├── temporary lifetime
│ ├── iterator invalidation
│ ├── vector reallocation
│ ├── map vs unordered_map
│ ├── emplace vs push
│ ├── reserve vs resize
│ ├── copy vs move
│ ├── std::move does not move
│ ├── pass-by-value vs reference
│ ├── const correctness
│ ├── static initialization order
│ ├── lambda capture lifetime
│ ├── shared_ptr cycles
│ ├── deadlocks
│ ├── race conditions
│ ├── atomic vs volatile
│ └── undefined behavior
│
├── 56. QUANT / HFT C++ ⭐⭐⭐⭐⭐
│ ├── low-latency programming
│ ├── deterministic latency
│ ├── market data parsing
│ ├── binary protocols
│ ├── order book
│ ├── limit order book
│ ├── price-time priority
│ ├── order matching engine
│ ├── market data feed handler
│ ├── feed handler architecture
│ ├── order gateway
│ ├── event-driven architecture
│ ├── lock-free queues
│ ├── ring buffers
│ ├── memory pools
│ ├── object pools
│ ├── cache-aware data structures
│ ├── CPU affinity
│ ├── NUMA
│ ├── kernel bypass concepts
│ ├── busy polling
│ ├── timestamping
│ ├── latency measurement
│ ├── jitter
│ ├── batching
│ ├── zero-copy networking
│ └── FIX / binary protocol concepts
│
└── 57. IMPLEMENTATION PROJECTS
├── MyVector
├── MyString
├── MyUniquePtr
├── MySharedPtr
├── MyOptional
├── MyVariant
├── MyFunction
├── MyUnorderedMap
├── MyMap
├── MyPriorityQueue
├── MyThread
├── ThreadSafeQueue
├── ThreadPool
├── LockFreeQueue
├── RingBuffer
├── MemoryPool
├── ObjectPool
├── LRUCache
├── LFUCache
├── RateLimiter
├── AsyncLogger
├── TCP Server
├── HTTP Server
├── EventLoop
├── TimerWheel
├── OrderBook
├── MatchingEngine
├── MarketDataParser
└── Low-Latency Trading Engine

LEARN → EXPLAIN → IMPLEMENT → DEBUG → BENCHMARK → INTERVIEW QUESTIONS
For example, when your AI gives you Vector:
Day: std::vector

1. What problem does vector solve?
2. Internal memory layout
3. size vs capacity
4. growth strategy
5. reserve()
6. resize()
7. push_back()
8. emplace_back()
9. insert()
10. erase()
11. iterator invalidation
12. exception guarantees
13. move vs copy during reallocation
14. allocator
15. cache locality
16. complexity of every operation

IMPLEMENT:
MyVector<T>

INTERVIEW:
Why is vector faster than list?
Why is vector contiguous?
Why does push_back sometimes take O(n)?
Why does reserve improve performance?
What happens to iterators after reallocation?
Why does vector prefer move when noexcept?
Why can vector not store incomplete types?
How would you implement vector yourself?

My highest-priority areas

For your Quant Developer / HFT target, I would rank the syllabus:

Tier S — Must know extremely well

1. Pointers / references
2. Object lifetime
3. RAII
4. Copy / move semantics
5. Smart pointers
6. STL containers
7. Vector internals
8. Hash table internals
9. Map / Red-Black tree
10. Templates
11. Memory management
12. Cache / cache locality
13. Multithreading
14. Mutexes
15. Condition variables
16. Atomics
17. C++ memory model
18. Acquire / release
19. CAS
20. Lock-free programming
21. Thread pools
22. Producer-consumer
23. CPU architecture
24. Performance optimization
25. Profiling
26. Low-latency programming
    Tier A — Very important
27. Virtual functions / vtable
28. Multiple inheritance
29. Exception safety
30. STL algorithms
31. Iterators
32. Allocators
33. Memory pools
34. Ring buffers
35. LRU cache
36. Networking
37. TCP / UDP
38. epoll
39. Serialization
40. CMake
41. GDB
42. Sanitizers
43. Compiler optimization
44. SIMD
45. Branch prediction
46. NUMA
47. Zero-copy
    Tier B — Know conceptually
48. Coroutines
49. Modules
50. Advanced ranges
51. Template metaprogramming
52. ABI
53. Type erasure
54. Visitor pattern
55. Advanced allocator machinery
56. Hazard pointers
57. Epoch-based reclamation
58. RCU

The implementation list I'd make mandatory

This is particularly important for me.

By the end, I should have personally implemented these:

DATA STRUCTURES
────────────────────────────────

1.  MyVector
2.  MyString
3.  MyDeque
4.  MyLinkedList
5.  MyStack
6.  MyQueue
7.  MyPriorityQueue
8.  MyHashMap
9.  MyHashSet
10. MyBST
11. MyAVLTree
12. MyRBTree
13. MyTrie
14. MyLRUCache
15. MyLFUCache
16. MyBloomFilter
17. MySkipList
18. MyRingBuffer

MEMORY
────────────────────────────────

19. MyUniquePtr
20. MySharedPtr
21. MyWeakPtr
22. MemoryPool
23. ObjectPool
24. ArenaAllocator
25. FreeListAllocator

CONCURRENCY
────────────────────────────────

26. ThreadSafeQueue
27. BlockingQueue
28. ThreadPool
29. ProducerConsumer
30. SPSCQueue
31. MPSCQueue
32. LockFreeStack
33. LockFreeQueue
34. ConcurrentHashMap
35. AsyncLogger

SYSTEMS
────────────────────────────────

36. TCP Server
37. UDP Server
38. EventLoop
39. TimerWheel
40. ConnectionPool
41. RateLimiter
42. PubSub

HFT
────────────────────────────────

43. LimitOrderBook
44. MatchingEngine
45. MarketDataParser
46. BinaryProtocolParser
47. OrderGateway
48. FeedHandler
49. LockFreeMarketDataQueue
50. LowLatencyTradingEngine

Since I'am already doing DSA and working on a C++ backtester, I would make your AI generate your daily material in this exact hierarchy:

DAY N
│
├── 1. THEORY
│ 5–10 concepts
│
├── 2. INTERNALS
│ How does it actually work?
│
├── 3. IMPLEMENTATION
│ Implement the important component
│
├── 4. OUTPUT QUESTIONS
│ Predict the output
│
├── 5. DEBUGGING
│ Find the bug / UB
│
├── 6. PERFORMANCE
│ Complexity + cache + allocation + latency
│
├── 7. INTERVIEW QUESTIONS
│ 10–20 questions
│
├── 8. HARD QUESTIONS
│ 3–5 senior-level questions
│
└── 9. MINI PROJECT
Apply the concept

I can give at most 1hr to 1.5hrs per day, so I would like to focus on 1–2 topics per day, and I would like to have a daily plan for the next 3 months.
breakdown the topics in folder and then day and topic vise , do not create too many folders, keep it simple and easy to follow. Each day should have a clear focus on a specific topic or concept, with a mix of theory, implementation, and practical exercises.

And don't give me only theoretical answers. For every major topic, make it ask you “implement this from scratch”, because questions like “Implement a vector,” “Design a thread pool,” “Implement an LRU cache,” “Build a bounded concurrent queue,” “How would you reduce latency here?” are much more valuable for your target than memorizing definitions.
