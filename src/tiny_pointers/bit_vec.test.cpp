#include <tiny_pointers/bit_vec.hpp>
//
#include <tiny_pointers/bit_vec.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <bitset>

namespace {

using namespace batt::int_types;
using tiny_pointers::BitVec;

TEST(BitVecTest, Test)
{
    BitVec x(10);

    EXPECT_EQ(x.size(), 10);

    x.set(1);
    x.set(2);
    x.set(5);

    EXPECT_EQ(x[0], false);
    EXPECT_EQ(x[1], true);
    EXPECT_EQ(x[2], true);
    EXPECT_EQ(x[3], false);
    EXPECT_EQ(x[4], false);
    EXPECT_EQ(x[5], true);
    EXPECT_EQ(x[6], false);
    EXPECT_EQ(x[7], false);
    EXPECT_EQ(x[8], false);
    EXPECT_EQ(x[9], false);

    EXPECT_EQ(x.int_value(), 0b100110);
    EXPECT_EQ(x.get_range(1, 6).int_value(), 0b10011);
    EXPECT_EQ(x.get_range(2, 6).int_value(), 0b1001);
    EXPECT_EQ(x.get_range(2, 6).int_value(), 0b1001);
    EXPECT_EQ(x.get_range(3, 6).int_value(), 0b100);

    BitVec y(16);

    EXPECT_EQ(y.int_value(), 0);

    y.set_range(4, x.get_range(1, 6));

    EXPECT_EQ(y.int_value(), 0b100110000);

    BitVec z(7 * 128);

    EXPECT_EQ(z.size(), 7 * 128);

    for (usize i = 0; i < 128; ++i) {
        BitVec e{7, i};
        z.set_range(i * 7, e);
    }
    for (usize i = 0; i < 128; ++i) {
        EXPECT_EQ(z.get_range(i * 7, (i + 1) * 7).int_value(), i);
    }
    for (usize i = 0; i < 128; ++i) {
        BitVec e{7, 127 - i};
        z.set_range(i * 7, e);
    }
    for (usize i = 0; i < 128; ++i) {
        EXPECT_EQ(z.get_range(i * 7, (i + 1) * 7).int_value(), 127 - i);
    }
}

}  //namespace
