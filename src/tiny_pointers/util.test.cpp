#include <tiny_pointers/util.hpp>
//
#include <tiny_pointers/util.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace {

using namespace batt::int_types;
using tiny_pointers::scale_u64;

TEST(UtilTest, ScaleU64)
{
    for (usize out_range : {1, 7, 8, 100, 999, 1024, 123456}) {
        EXPECT_EQ(scale_u64(0, out_range), 0);
        EXPECT_EQ(scale_u64(~u64{0}, out_range), out_range - 1);
        EXPECT_EQ(scale_u64(u64{1} << 63, out_range), out_range / 2);
        EXPECT_EQ(scale_u64(u64{1} << 62, out_range), out_range / 4);
    }
}

}  //namespace
