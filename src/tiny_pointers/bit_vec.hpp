#pragma once

#include "imports.hpp"

#include <batteries/checked_cast.hpp>

#include <vector>

namespace tiny_pointers {

inline void bit_copy(const u64* p_src, usize src_shift,  //
                     u64* p_dst, usize dst_shift,        //
                     usize n_to_copy)
{
    while (n_to_copy) {
        const usize bits = std::min(n_to_copy, 64 - std::max(src_shift, dst_shift));
        const u64 mask = (u64{1} << bits) - 1;

        *p_dst &= ~(mask << dst_shift);
        *p_dst |= ((*p_src >> src_shift) & mask) << dst_shift;

        src_shift += bits;
        dst_shift += bits;
        n_to_copy -= bits;

        if (src_shift == 64) {
            src_shift = 0;
            ++p_src;
        }
        if (dst_shift == 64) {
            dst_shift = 0;
            ++p_dst;
        }
    }
}

class BitVec
{
   public:
    using Self = BitVec;

    BitVec() = default;

    explicit BitVec(usize n) noexcept : bit_size_{n}, words_((n + 63) / 64, 0)
    {
    }

    explicit BitVec(i32 n) noexcept : BitVec{BATT_CHECKED_CAST(usize, n)}
    {
    }

    BitVec(usize n, u64 data) noexcept : bit_size_{n}, words_((n + 63) / 64)
    {
        BATT_CHECK_LE(n, 64);
        if (n) {
            this->words_[0] = data & ((u64{1} << n) - 1);
        }
    }
    BitVec(i32 n, u64 data) noexcept : BitVec{BATT_CHECKED_CAST(usize, n), data}
    {
    }

    BitVec(usize n, std::string_view s) noexcept : BitVec{n}
    {
        *this = s;
    }

    BitVec(i32 n, std::string_view s) noexcept : BitVec{BATT_CHECKED_CAST(usize, n), s}
    {
    }

    explicit BitVec(std::string_view s) noexcept : BitVec{s.size() * 8, s}
    {
    }

    //+++++++++++-+-+--+----- --- -- -  -  -   -

    Self& operator=(std::string_view s) noexcept
    {
        const usize n_to_copy = std::min(s.size(), this->words_.size() * sizeof(u64));
        std::memcpy(this->words_.data(), s.data(), n_to_copy);
        return *this;
    }

    bool operator[](usize i) const noexcept
    {
        return (this->words_[i / 64] & (u64{1} << (i % 64))) != 0;
    }

    usize size() const noexcept
    {
        return this->bit_size_;
    }

    Self& set(usize i, bool b = true) noexcept
    {
        if (b) {
            this->words_[i / 64] |= u64{1} << (i % 64);
        } else {
            this->words_[i / 64] &= ~(u64{1} << (i % 64));
        }
        return *this;
    }

    BitVec get_range(usize begin, usize end) const noexcept
    {
        const usize n_to_copy = end - begin;

        BitVec dst(n_to_copy);

        bit_copy(this->words_.data() + begin / 64, begin % 64,  //
                 dst.words_.data(), 0,                          //
                 n_to_copy);

        return dst;
    }

    Self& set_range(usize begin, const BitVec& src) noexcept
    {
        bit_copy(src.words_.data(), 0,                          //
                 this->words_.data() + begin / 64, begin % 64,  //
                 src.size());

        return *this;
    }

    u64 int_value() const noexcept
    {
        return this->words_[0] & ((u64{1} << this->size()) - 1);
    }

    std::string_view as_str() const noexcept
    {
        return std::string_view{(const char*)this->words_.data(), this->bit_size_ / 8};
    }

   private:
    usize bit_size_ = 0;
    SmallVec<u64, 1> words_;
};

inline std::ostream& operator<<(std::ostream& out, const BitVec& t)
{
    for (usize i = 0; i < t.size(); ++i) {
        out << (t[t.size() - i - 1] ? '1' : '0');
    }
    return out;
}

}  //namespace tiny_pointers
