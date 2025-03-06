# Tiny Pointers

This repo contains a partial implementation of the ideas from: [Tiny Pointers](https://dl.acm.org/doi/10.1145/3700594) (_Michael A. Bender, Alex Conway, Martín Farach-Colton, William Kuszmaul, and Guido Tagliavini. 2024. ACM Trans. Algorithms Just Accepted (October 2024)_).

## Motivation

Pointers can represent a lot of overhead:

```
                                  ┌──────┐                                                                    
                                  │(root)│                                                                    
                                  └──────┘                                                                    
                                      │                                                                       
                                      ▼                                                                       
                                  ┌──────┬─────────────────┬─────────────────┐                                
                                  │"echo"│  left_pointer   │  right_pointer  │                                
                                  └──────┴─────────────────┴─────────────────┘                                
                                                  │                 │                                         
                 ┌────────────────────────────────┘                 │                                         
                 ▼                                                  ▼                                         
             ┌───────┬─────────────────┬─────────────────┐     ┌─────────┬─────────────────┬─────────────────┐
             │"bravo"│  left_pointer   │  right_pointer  │     │"foxtrot"│  left_pointer   │  right_pointer  │
             └───────┴─────────────────┴─────────────────┘     └─────────┴─────────────────┴─────────────────┘
                              │                 │                                                             
    ┌─────────────────────────┘                 └──┐                                                          
    ▼                                              ▼                                                          
┌───────┬─────────────────┬─────────────────┐ ┌─────────┬─────────────────┬─────────────────┐                 
│"alpha"│  left_pointer   │  right_pointer  │ │"charlie"│  left_pointer   │  right_pointer  │                 
└───────┴─────────────────┴─────────────────┘ └─────────┴─────────────────┴─────────────────┘                 
```

Solution: Tiny Pointers!

Concept: Dereference Table

```
                               
    hash bucket     slot (p)   
                               
         │              │      
         └──────────┐   │      
                    ▼   ▼      
┌──────┬───┬──────┬───┬──┐     
│(root)│ p │~~~~~~│   │00│     
├──────┴┬──┴┬───┬─┤   ├──┤     
│"bravo"│ p │ p │~│   │01│     
├───────┴───┴───┴─┤ 0 ├──┤     
│~~~~~~~~~~~~~~~~~│   │10│     
├─────────────────┤   ├──┤     
│~~~~~~~~~~~~~~~~~│   │11│     
├──────┬───┬───┬──┼───┼──┤     
│"echo"│ p │ p │~~│   │00│     
├──────┴┬──┴┬──┴┬─┤   ├──┤     
│"alpha"│ p │ p │~│   │01│     
├───────┴─┬─┴─┬─┴─┤ 1 ├──┤     
│"charlie"│ p │ p │   │10│     
├─────────┴───┴───┤   ├──┤     
│~~~~~~~~~~~~~~~~~│   │11│     
├─────────┬───┬───┼───┼──┤     
│"foxtrot"│ p │ p │   │00│     
├─────────┴───┴───┤   ├──┤     
│~~~~~~~~~~~~~~~~~│   │01│     
├─────────────────┤ 2 ├──┤     
│~~~~~~~~~~~~~~~~~│   │10│     
├─────────────────┤   ├──┤     
│~~~~~~~~~~~~~~~~~│   │11│     
└─────────────────┴───┴──┘     
```

TinyPointer + Key == Table Slot

Keys in example would be:
- "(root)"
- "(null)" (?)
- "alpha:left"
- "alpha:right"
- "bravo:left"
- "bravo:right"

etc.

```c++
/** \brief Dereference Table as defined in Section 2, Preliminaries.
 */
class DereferenceTable
{
   public:
    using Self = DereferenceTable;
    using Ptr = std::unique_ptr<Self>;

    //+++++++++++-+-+--+----- --- -- -  -  -   -

    DereferenceTable(const DereferenceTable&) = delete;
    DereferenceTable& operator=(const DereferenceTable&) = delete;

    virtual ~DereferenceTable() = default;

    //+++++++++++-+-+--+----- --- -- -  -  -   -
    // From the paper:

    /** \brief Creates a new dereference table, and returns a pointer to an array with `n` slots, each of size
     * `q` bits. We call this array the store. The dereference table will be capable of supporting up to (1 −
     * d n concurrent allocations at at time. We require that d = O(1/q).
     */
    using CreateFn = StatusOr<Ptr>(SlotCount n, BitsPerSlot q, Delta d);

    /** \brief Given a key `x`, allocates a slot in the store to `x`, and returns a bit string `p`, which we
     * call a tiny pointer.
     */
    virtual StatusOr<TinyPointer> Allocate(Key x) noexcept = 0;

    /** \brief Given a key `x` and a tiny pointer `p`, the procedure returns the index of the slot allocated
     * to `x` in the store. If `p` is not a valid tiny pointer for `x` (i.e., `p` was not returned by a call
     * to Allocate(`x`)), then the procedure may return an arbitrary index in the store.
     */
    virtual SlotIndex Dereference(Key x, TinyPointer p) noexcept = 0;

    /** \brief Given a key `x` and a tiny pointer `p`, the procedure deallocates slot Dereference(`x`, `p`)
     * from `x`. The user is only permitted to call this function on pairs (`x`, `p`) where `p` is a valid
     * tiny pointer for `x` (i.e., `p` was returned by the most recent call to Allocate(`x`)).
     */
    virtual void Free(Key x, TinyPointer p) noexcept = 0;

    //+++++++++++-+-+--+----- --- -- -  -  -   -
    // Additional methods:

    /** \brief Sets the value of slot `i` to `v`.
     */
    virtual void Set(SlotIndex i, Value v) noexcept = 0;

    /** \brief Gets the value currently held by slot `i`.
     *
     * If `i` is not a valid slot in this table, panic.
     */
    virtual Value Get(SlotIndex i) noexcept = 0;

    //+++++++++++-+-+--+----- --- -- -  -  -   -
   protected:
    DereferenceTable() = default;
};
```


## Test Output

```
[==========] Running 5 tests from 4 test suites.
[----------] Global test environment set-up.
[----------] 1 test from UtilTest
[ RUN      ] UtilTest.ScaleU64
[       OK ] UtilTest.ScaleU64 (0 ms)
[----------] 1 test from UtilTest (0 ms total)

[----------] 2 tests from TinyPointersTest
[ RUN      ] TinyPointersTest.SimpleDereferenceTable
 sdt.load_factor() == 0.958333
 sdt.n_slots() == 10285056
 sdt.capacity() == 9856512
 usize{1} << sdt.tiny_pointer_size() == 524288
 sdt.slots_per_bucket() == 331776
 sdt.bucket_count() == 31
 std::log2(sdt.n_slots()) == 23.294
 sdt.log_n() == 24
 sdt.tiny_pointer_size() == 19
 4 * std::log2(std::log2(sdt.n_slots())) == 18.1676
[       OK ] TinyPointersTest.SimpleDereferenceTable (171 ms)
[ RUN      ] TinyPointersTest.SimpleDereferenceTable_LoadFactor
Connection to localhost closed.
 avg_load_factor == 0.996421 load_factor == 0.958333
[       OK ] TinyPointersTest.SimpleDereferenceTable_LoadFactor (22064 ms)
[----------] 2 tests from TinyPointersTest (22235 ms total)

[----------] 1 test from DataTest
[ RUN      ] DataTest.Load
 words.size() == 542778 word_set.size() == 249607
[       OK ] DataTest.Load (77 ms)
[----------] 1 test from DataTest (77 ms total)

[----------] 1 test from BitVecTest
[ RUN      ] BitVecTest.Test
[       OK ] BitVecTest.Test (0 ms)
[----------] 1 test from BitVecTest (0 ms total)

[----------] Global test environment tear-down
[==========] 5 tests from 4 test suites ran. (22312 ms total)
[  PASSED  ] 5 tests.

```
