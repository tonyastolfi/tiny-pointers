#pragma once

#include "bit_vec.hpp"
#include "imports.hpp"
#include "util.hpp"

#include <batteries/strong_typedef.hpp>

#include <xxhash.h>

#include <bitset>
#include <functional>
#include <memory>
#include <random>
#include <string_view>

namespace tiny_pointers {

//=#=#==#==#===============+=+=+=+=++=++++++++++++++-++-+--+-+----+---------------

/** \brief A key is anything that can be hashed.  We will represent this as a byte string.
 */
using Key = std::string_view;

/** \brief Dereference tables store values of `q` bits in size; represent values as bit vectors.
 */
using Value = BitVec;

/** \brief Tiny pointers are small integers; we will represent them as bit vectosfixed-size bit setas bit
 * vectors.
 */
using TinyPointer = BitVec;

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

//=#=#==#==#===============+=+=+=+=++=++++++++++++++-++-+--+-+----+---------------

/** \brief Definition of load factor.
 */
inline Delta load_factor(Delta delta) noexcept
{
    return Delta{1.0 - delta};
}

//=#=#==#==#===============+=+=+=+=++=++++++++++++++-++-+--+-+----+---------------

/** \brief Define a family of seed-able hash functions for the various constructions.
 */
struct HashFn {
    u64 seed_;

    explicit HashFn(u64 seed) noexcept : seed_{seed}
    {
    }

    usize operator()(const std::string_view& s) const noexcept
    {
        return XXH3_64bits_withSeed(s.data(), s.size(), this->seed_);
    }
};

//=#=#==#==#===============+=+=+=+=++=++++++++++++++-++-+--+-+----+---------------

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

//=#=#==#==#===============+=+=+=+=++=++++++++++++++-++-+--+-+----+---------------

/** \brief From Section 3, Warmup:
 *
 * Let `q` ≥ log `n` and `d` = 1/log `n`. There is a dereference table for `q`-bit values that:
 *
 *  1. succeeds on each allocation w.h.p.
 *  2. has load factor 1 − `d`
 *  3. has constant-time operations
 *  4. produces tiny pointers of size O(log log `n`) bits
 */
class SimpleDereferenceTable : public DereferenceTable
{
   public:
    SimpleDereferenceTable(SlotCount n, BitsPerSlot q) noexcept

        // We partition the store into n/b buckets, each of which has b = log^4(n) slots.
        //
        : slots_per_bucket_{log2_ceil(n) * log2_ceil(n) * log2_ceil(n) * log2_ceil(n)}
        , bucket_count_{(n + this->slots_per_bucket_ - 1) / this->slots_per_bucket_}
        , n_slots_{this->slots_per_bucket_ * this->bucket_count_}
        , log_n_{log2_ceil(this->n_slots_)}

        // If the key (x) is allocated the p-th slot in the bucket, then the number p is returned as the tiny
        // pointer for x
        //
        , p_bits_{log2_ceil(this->slots_per_bucket_)}

        // ...for q-bit values...
        //
        , q_bits_per_slot_{q}

        // let delta = 1/log(n)
        //
        , delta_{1.0 / (double)this->log_n_}

        , size_{0}
        , hash_fn_{std::random_device{}()}
        , storage_(this->n_slots_ * this->q_bits_per_slot_)

        // The head of the free list for each bucket.
        //
        , free_list_head_(this->bucket_count_ * this->p_bits_)
    {
        BATT_CHECK_GE(this->n_slots_, n);
        BATT_CHECK_GE(this->q_bits_per_slot_, this->log_n_);

        // Initialize free lists; free heads should all be zero, so they are good.
        //
        for (usize bucket_i = 0; bucket_i < this->bucket_count_; ++bucket_i) {
            for (usize slot_i = 0; slot_i < this->slots_per_bucket_; ++slot_i) {
                this->set_free_next(bucket_i, slot_i, BitVec{this->p_bits_, slot_i + 1});
            }
        }
    }

    /** \brief Returns the maximum fraction of storage slots available for allocation.
     */
    double load_factor() const noexcept
    {
        return 1.0 - this->delta_;
    }

    /** \brief The number of slots in the storage array; not all are available for allocation (see capacity).
     */
    usize n_slots() const noexcept
    {
        return this->n_slots_;
    }

    /** \brief The maximum number of active allocations (w.h.p.).
     */
    usize capacity() const noexcept
    {
        return this->load_factor() * this->n_slots_;
    }

    /** \brief The current number of active allocations.
     */
    usize size() const noexcept
    {
        return this->size_;
    }

    /** \brief The size of TinyPointers returned by this.
     */
    usize tiny_pointer_size() const noexcept
    {
        return this->p_bits_;
    }

    usize slots_per_bucket() const noexcept
    {
        return this->slots_per_bucket_;
    }

    usize log_n() const noexcept
    {
        return this->log_n_;
    }

    usize bucket_count() const noexcept
    {
        return this->bucket_count_;
    }

    //==#==========+==+=+=++=+++++++++++-+-+--+----- --- -- -  -  -   -
    //
    StatusOr<TinyPointer> Allocate(Key x) noexcept override
    {
        // Find the bucket for x.
        //
        const usize bucket_i = this->find_bucket(x);

        // Look at the first free slot for the bucket.
        //
        TinyPointer free_slot = this->get_free_head(bucket_i);
        if (free_slot.int_value() == this->slots_per_bucket_) {
            return {batt::StatusCode::kResourceExhausted};
        }
        BATT_CHECK_LT(free_slot.int_value(), this->slots_per_bucket_);

        // There is a free slot; set the head of the free list to the next
        // free slot and give the first one to the caller.
        //
        TinyPointer next_free = this->get_free_next(bucket_i, free_slot.int_value());
        this->set_free_head(bucket_i, next_free);

        BATT_CHECK_EQ(this->get_free_head(bucket_i).int_value(), next_free.int_value());

        // Success!
        //
        ++this->size_;
        return free_slot;
    }

    //==#==========+==+=+=++=+++++++++++-+-+--+----- --- -- -  -  -   -
    //
    SlotIndex Dereference(Key x, TinyPointer p) noexcept override
    {
        BATT_CHECK_EQ(p.size(), this->p_bits_);

        // Find the bucket for x.
        //
        const usize bucket_i = this->find_bucket(x);
        const usize slot_i = p.int_value();

        return SlotIndex{bucket_i * this->slots_per_bucket_ + slot_i};
    }

    //==#==========+==+=+=++=+++++++++++-+-+--+----- --- -- -  -  -   -
    //
    void Free(Key x, TinyPointer p) noexcept override
    {
        BATT_CHECK_EQ(p.size(), this->p_bits_);

        // Find the bucket for x.
        //
        const usize bucket_i = this->find_bucket(x);

        // Slot `p` will be the new head; set it's next to the current head.
        //
        this->set_free_next(bucket_i, p.int_value(), this->get_free_head(bucket_i));

        // Push `p` onto the free list.
        //
        this->set_free_head(bucket_i, p);

        // Success!
        //
        --this->size_;
    }

    //==#==========+==+=+=++=+++++++++++-+-+--+----- --- -- -  -  -   -
    //
    void Set(SlotIndex i, Value v) noexcept override
    {
        BATT_CHECK_LE(v.size(), this->q_bits_per_slot_);

        const usize pos = i * this->q_bits_per_slot_;

        this->storage_.set_range(pos, v);
    }

    //==#==========+==+=+=++=+++++++++++-+-+--+----- --- -- -  -  -   -
    //
    Value Get(SlotIndex i) noexcept override
    {
        const usize pos = i * this->q_bits_per_slot_;

        return this->storage_.get_range(pos, pos + this->q_bits_per_slot_);
    }

    //+++++++++++-+-+--+----- --- -- -  -  -   -
   private:
    usize find_bucket(const Key& x) const noexcept
    {
        const u64 bucket_i = scale_u64(this->hash_fn_(x), this->bucket_count_);
        BATT_CHECK_LT(bucket_i, this->bucket_count_);

        return bucket_i;
    }

    void set_free_next(usize bucket_i, usize slot_i, const TinyPointer& value) noexcept
    {
        BATT_CHECK_EQ(value.size(), this->p_bits_);

        const usize pos = (bucket_i * this->slots_per_bucket_ + slot_i) * this->q_bits_per_slot_;

        this->storage_.set_range(pos, value);
    }

    TinyPointer get_free_next(usize bucket_i, usize slot_i) const noexcept
    {
        const usize pos = (bucket_i * this->slots_per_bucket_ + slot_i) * this->q_bits_per_slot_;

        return this->storage_.get_range(pos, pos + this->p_bits_);
    }

    void set_free_head(usize bucket_i, const TinyPointer& value) noexcept
    {
        BATT_CHECK_EQ(value.size(), this->p_bits_);

        const usize pos = bucket_i * this->p_bits_;

        this->free_list_head_.set_range(pos, value);
    }

    TinyPointer get_free_head(usize bucket_i) const noexcept
    {
        const usize pos = bucket_i * this->p_bits_;

        return this->free_list_head_.get_range(pos, pos + this->p_bits_);
    }

    //+++++++++++-+-+--+----- --- -- -  -  -   -
    // b - the bucket size
    //
    const i32 slots_per_bucket_;

    // n/b - the number of buckets
    //
    const usize bucket_count_;

    // n - the number of slots
    //
    const SlotCount n_slots_;

    // log(n)
    //
    const i32 log_n_;

    // The TinyPointer size, in bits.
    //
    const i32 p_bits_;

    // q - the value size
    //
    const BitsPerSlot q_bits_per_slot_;

    // 1 - load_factor
    //
    const Delta delta_;

    // The number of active allocations.
    //
    usize size_;
    HashFn hash_fn_;
    BitVec storage_;
    BitVec free_list_head_;
};

}  //namespace tiny_pointers
