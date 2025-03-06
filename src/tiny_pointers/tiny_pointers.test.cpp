#include <tiny_pointers/tiny_pointers.hpp>
//
#include <tiny_pointers/tiny_pointers.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <bitset>

namespace {

using namespace batt::int_types;
using tiny_pointers::BitVec;

TEST(TinyPointersTest, Test)
{
    BitVec x(10);

    x.set(1);
    x.set(2);
    x.set(5);

    std::cout << BATT_INSPECT(x) << std::endl;
    std::cout << "      " << std::bitset<10>{x.int_value()} << std::endl;
    std::cout << BATT_INSPECT(x.get_range(1, 6)) << std::endl;

    BitVec y(16);

    std::cout << BATT_INSPECT(y) << std::endl;

    y.set_range(4, x.get_range(1, 6));

    std::cout << BATT_INSPECT(y) << std::endl;
}

}  // namespace
