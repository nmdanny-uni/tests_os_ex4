#include "VirtualMemory.h"

#include <gtest/gtest.h>
#include <cstdio>
#include <cassert>


TEST(SimpleTests, Can_Read_Then_Write_Memory)
{
    VMinitialize();
    for (uint64_t i = 0; i < (2 * NUM_FRAMES); ++i) {
        printf("writing to %llu\n", (long long int) i);
        VMwrite(5 * i * PAGE_SIZE, i);
    }

    for (uint64_t i = 0; i < (2 * NUM_FRAMES); ++i) {
        word_t value;
        VMread(5 * i * PAGE_SIZE, &value);
        printf("reading from %llu %d\n", (long long int) i, value);
        EXPECT_EQ(uint64_t(value), i);
    }
}
