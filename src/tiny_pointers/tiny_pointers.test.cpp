#include <tiny_pointers/tiny_pointers.hpp>
//
#include <tiny_pointers/tiny_pointers.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <tiny_pointers/data.hpp>

#include <bitset>
#include <cmath>
#include <random>

namespace {

using namespace batt::int_types;
using tiny_pointers::BitsPerSlot;
using tiny_pointers::BitVec;
using tiny_pointers::Key;
using tiny_pointers::random_key;
using tiny_pointers::SimpleDereferenceTable;
using tiny_pointers::SlotCount;
using tiny_pointers::Status;
using tiny_pointers::StatusOr;
using tiny_pointers::TinyPointer;
using tiny_pointers::Value;

//==#==========+==+=+=++=+++++++++++-+-+--+----- --- -- -  -  -   -
//
TEST(TinyPointersTest, SimpleDereferenceTable)
{
    SimpleDereferenceTable sdt{SlotCount{(usize)1e7}, BitsPerSlot{8 * 40}};

    EXPECT_EQ(sdt.size(), 0);

    std::cerr << BATT_INSPECT(sdt.load_factor()) << std::endl
              << BATT_INSPECT(sdt.n_slots()) << std::endl
              << BATT_INSPECT(sdt.capacity()) << std::endl
              << BATT_INSPECT(usize{1} << sdt.tiny_pointer_size()) << std::endl
              << BATT_INSPECT(sdt.slots_per_bucket()) << std::endl
              << BATT_INSPECT(sdt.bucket_count()) << std::endl
              << BATT_INSPECT(std::log2(sdt.n_slots())) << std::endl
              << BATT_INSPECT(sdt.log_n()) << std::endl
              << BATT_INSPECT(sdt.tiny_pointer_size()) << std::endl
              << BATT_INSPECT(4 * std::log2(std::log2(sdt.n_slots()))) << std::endl
        //
        ;
}

//==#==========+==+=+=++=+++++++++++-+-+--+----- --- -- -  -  -   -
//
TEST(TinyPointersTest, SimpleDereferenceTable_LoadFactor)
{
    std::default_random_engine rng{std::random_device{}()};
    std::vector<usize> size_reached;
    usize n_slots = 0;
    usize capacity = 0;
    double load_factor = 0;

    for (usize i = 0; i < 10; ++i) {
        SimpleDereferenceTable sdt{SlotCount{(usize)1e7}, BitsPerSlot{8 * 40}};
        n_slots = sdt.n_slots();
        capacity = sdt.capacity();
        load_factor = sdt.load_factor();

        for (usize j = 0; j < sdt.bucket_count(); ++j) {
            BitVec head = sdt.get_free_head(j);
            ASSERT_EQ(head.int_value(), 0);
            for (usize s = 0; s < sdt.slots_per_bucket(); ++s) {
                ASSERT_EQ(sdt.get_free_next(j, s).int_value(), s + 1);
            }
        }

        for (usize k = 0; k < sdt.n_slots(); ++k) {
            if (!sdt.Allocate(random_key(rng)).ok()) {
                size_reached.push_back(sdt.size());
                break;
            }
        }
    }
    std::sort(size_reached.begin(), size_reached.end());

    usize p50 = size_reached[size_reached.size() / 2];

    double total = 0;
    for (double n : size_reached) {
        total += n;
    }
    double avg_load_factor = (total / (double)size_reached.size()) / (double)n_slots;
    std::cerr << BATT_INSPECT(avg_load_factor) << BATT_INSPECT(load_factor) << std::endl;

    EXPECT_LT(p50, n_slots);
    EXPECT_GT(p50, capacity);
}

}  // namespace
