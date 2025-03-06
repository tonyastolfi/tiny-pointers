#pragma once

#include "imports.hpp"

namespace tiny_pointers {

/** \brief Returns an integer in the range [0, out_range) via linear scaling of `in_val`:
 *
 *  - when in_val == 0, return 0
 *  - when in_val == ~0, return out_range - 1
 *  - for other values of in_val, linearly interpolate between these bounds
 */
inline u64 scale_u64(u64 in_val, u64 out_range)
{
    const i32 pre_shift = log2_ceil(out_range) + 1;
    const i32 post_shift = 64 - pre_shift;

    return ((in_val >> pre_shift) * out_range) >> post_shift;
}

}  //namespace tiny_pointers
