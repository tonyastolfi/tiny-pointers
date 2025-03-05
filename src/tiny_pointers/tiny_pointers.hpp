#pragma once

#include "imports.hpp"

#include <batteries/strong_typedef.hpp>

#include <bm.h>

#include <bitset>
#include <functional>
#include <memory>
#include <string_view>

namespace tiny_pointers {

/** \brief A key is anything that can be hashed.  We will represent this as a byte string.
 */
using Key = std::string_view;

/** \brief Dereference tables store values of `q` bits in size; represent values as bit vectors.
 */
using Value = bm::bvector<>;

/** \brief Tiny pointers are small integers; we will represent them as bit vectosfixed-size bit setas bit
 * vectors.
 */
using TinyPointer = bm::bvector<>;

/** \brief The index of a slot in a DereferenceTable.
 */
BATT_STRONG_TYPEDEF(usize, SlotIndex);

/** \brief A number of slots.
 */
BATT_STRONG_TYPEDEF(usize, SlotCount);

/** \brief The size in bits of a slot.
 */
BATT_STRONG_TYPEDEF(usize, BitsPerSlot);

/** \brief The (little) delta parameter from the paper.
 */
BATT_STRONG_TYPEDEF(double, Delta);

/** \brief Definition of load factor.
 */
inline Delta max_load_factor(Delta delta) noexcept
{
    return Delta{1.0 - delta};
}

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
    using Create = std::function<DereferenceTable::Ptr(SlotCount n, BitsPerSlot q, Delta d)>;

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

}  //namespace tiny_pointers
