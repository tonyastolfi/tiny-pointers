#pragma once

#include <batteries/assert.hpp>
#include <batteries/constants.hpp>
#include <batteries/int_types.hpp>
#include <batteries/math.hpp>
#include <batteries/optional.hpp>
#include <batteries/small_vec.hpp>
#include <batteries/status.hpp>
#include <batteries/stream_util.hpp>
#include <batteries/utility.hpp>

namespace tiny_pointers {

using namespace batt::int_types;
using namespace batt::constants;
using batt::log2_ceil;
using batt::log2_floor;
using batt::Optional;
using batt::SmallVec;
using batt::Status;
using batt::StatusOr;

}  //namespace tiny_pointers
