# Tiny Pointers

This repo contains a partial implementation of the ideas from: [Tiny Pointers](https://dl.acm.org/doi/10.1145/3700594) (_Michael A. Bender, Alex Conway, Martín Farach-Colton, William Kuszmaul, and Guido Tagliavini. 2024. ACM Trans. Algorithms Just Accepted (October 2024)_).

Also inspired:

[Optimal Bounds for Open Addressing Without Reordering](https://arxiv.org/abs/2501.02305)

[Undergraduate Upends a 40-Year-Old Data Science Conjecture](https://www.quantamagazine.org/undergraduate-upends-a-40-year-old-data-science-conjecture-20250210/)

[Lightning Talk](https://youtu.be/ArQNyOU1hyE?si=yIC2NhfSiBWTXlJH)

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
    ┌─────────────────┘                 └──────────┐                                         
    ▼                                              ▼                                         
┌───────┬─────────────────┬─────────────────┐ ┌─────────┬─────────────────┬─────────────────┐
│"bravo"│  left_pointer   │  right_pointer  │ │"foxtrot"│  left_pointer   │  right_pointer  │
└───────┴─────────────────┴─────────────────┘ └─────────┴─────────────────┴─────────────────┘
                 │                 │                                                         
    ┌────────────┘                 └───────────────┐                                         
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

    /** \brief Creates a new dereference table, and returns a pointer to an
     * array with `n` slots, each of size `q` bits. We call this array the
     * store. The dereference table will be capable of supporting up to (1 − d n
     * concurrent allocations at at time. We require that d = O(1/q).
     */
    using CreateFn = StatusOr<Ptr>(SlotCount n, BitsPerSlot q, Delta d);

    /** \brief Given a key `x`, allocates a slot in the store to `x`, and
     * returns a bit string `p`, which we call a tiny pointer.
     */
    virtual StatusOr<TinyPointer> Allocate(Key x) noexcept = 0;

    /** \brief Given a key `x` and a tiny pointer `p`, the procedure returns the
     * index of the slot allocated to `x` in the store. If `p` is not a valid
     * tiny pointer for `x` (i.e., `p` was not returned by a call to
     * Allocate(`x`)), then the procedure may return an arbitrary index in the
     * store.
     */
    virtual SlotIndex Dereference(Key x, TinyPointer p) noexcept = 0;

    /** \brief Given a key `x` and a tiny pointer `p`, the procedure deallocates
     * slot Dereference(`x`, `p`) from `x`. The user is only permitted to call
     * this function on pairs (`x`, `p`) where `p` is a valid tiny pointer for
     * `x` (i.e., `p` was returned by the most recent call to Allocate(`x`)).
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

## Simple Deference Table (SDT)

- bucket size = `log^4(n)`
- load factor = `1 - 1/log(n)`
- tiny pointer size = `O(log(log(n)))` (actually `~ceil(4 * log(log(n)))`)

## Load Balancing Table (LBT)

- bucket size = `δ^−2 * log(δ^−1)`
- if `δ = 1 / log(log(n))`, then bucket size = `log(log(log(n))) / (log(log(n)) * log(log(n)))`
- tiny pointer size = `O(log(log(log(n))))` (!!)
- problem: high failure rate (no more w.h.p.)

## Power-of-Two-Choices Table (P2T)

- bucket size = `log(n)`
- tiny pointer size = `1 + log(n)`
- insert/allocate algorithm: hash to two locations, pick the least loaded one
- this gives much better failure rate, at a cost of much lower load factor

## Final Construction (fixed-size Tiny Pointers)

- Create both an LBT and a P2T
- First try inserting to LBT; that will fail at rate of `1/δ`; use P2T on failure
- This finally achieves;
  - `O(log(log(log(n))))` tiny pointer size
  - allocations succeed w.h.p.
  - good load factor (`1 - 1/log(log(n))`)

## Example

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

## Some other concepts mentioned

### Using bit-wise instructions to implement fast rank/select (for free lists)

```c++
/** \brief Returns the number of 1's in `bit_set` at or before position `index`.
 */
inline u64 bit_rank(u64 bit_set, u64 index) noexcept
{
    return __builtin_popcountll(bit_set & ((u64{1} << index) - 1));
}

/** \brief Returns the position of the `rank`-th 1-bit within `bitset`.
 *
 * Example:
 *
 * bit_set =
 * 0000000000000000000000000000010101101110010010111000110001111001
 *
 * rank = 8
 * mask = (2 << 8) - 1 =
 * 0000000000000000000000000000000000000000000000000000000111111111
 *                              ┌───────────────┘│││││││││┃┃┃┃┃┃┃┃┃
 *                              │ ┌──────────────┘││││││││┃┃┃┃┃┃┃┃┃
 *                              │ │ ┌─────────────┘│││││││┃┃┃┃┃┃┃┃┃
 *                              │ │ │┌─────────────┘││││││┃┃┃┃┃┃┃┃┃
 *                              │ │ ││ ┌────────────┘│││││┃┃┃┃┃┃┃┃┃
 *                              │ │ ││ │┌────────────┘││││┃┃┃┃┃┃┃┃┃
 *                              │ │ ││ ││┌────────────┘│││┃┃┃┃┃┃┃┃┃
 *                              │ │ ││ │││  ┌──────────┘││┃┃┃┃┃┃┃┃┃
 *                              │ │ ││ │││  │  ┌────────┘│┃┃┃┃┃┃┃┃┃
 *                              │ │ ││ │││  │  │ ┌───────┘┃┃┃┃┃┃┃┃┃
 *                              │ │ ││ │││  │  │ │┏━━━━━━━┛┃┃┃┃┃┃┃┃
 *                              │ │ ││ │││  │  │ │┃┏━━━━━━━┛┃┃┃┃┃┃┃
 *                              │ │ ││ │││  │  │ │┃┃   ┏━━━━┛┃┃┃┃┃┃
 *                              │ │ ││ │││  │  │ │┃┃   ┃┏━━━━┛┃┃┃┃┃
 *                              │ │ ││ │││  │  │ │┃┃   ┃┃   ┏━┛┃┃┃┃
 *                              │ │ ││ │││  │  │ │┃┃   ┃┃   ┃┏━┛┃┃┃
 *                              │ │ ││ │││  │  │ │┃┃   ┃┃   ┃┃┏━┛┃┃
 *                              │ │ ││ │││  │  │ │┃┃   ┃┃   ┃┃┃┏━┛┃
 *                              ▽ ▽ ▽▽ ▽▽▽  ▽  ▽ ▽▼▼   ▼▼   ▼▼▼▼  ▼
 *                         (..0010101101110010010111000110001111001) (bit_set)
 * 0000000000000000000000000000000000000000000000011000110001111001  (PDEP result)
 * |◀───────────────────────────────────────────▶│
 * CLZ = 47                                      │
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~│~~~~~~~~~~~~~~~~~
 * 6666555555555544444444443333333333222222222211111111110000000000  (bit index 10's)
 * 3210987654321098765432109876543210987654321098765432109876543210  (bit index 1's)
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~│~~~~~~~~~~~~~~~~~
 *                                               │ (63 - 47) = 16 =  (select result)
 */
inline u64 bit_select(u64 bit_set, u64 rank) noexcept
{
    return 63 - __builtin_clzll(_pdep_u64((u64{2} << rank) - 1, bit_set));
}
```

## How to Build/Run

## Prerequisites

- Linux
- Python 3.10 (or newer)
- Pip
- CMake 

## Tools Setup

### Install Pipx

```shell
pip install --upgrade pipx
```

### Install batt-cli

For more info, see: [https://gitlab.com/batteriesincluded/batt-cli](https://gitlab.com/batteriesincluded/batt-cli).

```
pip install batt-cli --index-url https://gitlab.com/api/v4/projects/64628567/packages/pypi/simple
cor-setup
cor setup-conan
```

## Build

In project dir:

```shell
cor build
```

## Run Tests

In project dir:

```shell
cor test --only
```

## Build and Run Tests (single-step alternative method)

```shell
cor test
```
