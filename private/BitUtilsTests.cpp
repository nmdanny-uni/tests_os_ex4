/// These are private implementation tests, they are not part of the public test suite,
/// So please ignore this file.

#include "VirtualMemory.h"
#include "BitUtils.h"

#include <gtest/gtest.h>
#include <cstdio>
#include <cassert>

TEST(BitUtilsTests, GrabBitsTest)
{
    uint64_t word = 0b10011011010101;

    ASSERT_EQ(displayBits(word, 14),
              "10011011010101");

    ASSERT_EQ(displayBits(word),
              displayBits(grabBitsMSB(word, 50, 14)));

    ASSERT_EQ(displayBits(grabBitsMSB(word, 61, 3)),
              displayBits(0b101));


    ASSERT_EQ(displayBits(grabBitsMSB(word, 59, 1)),
              displayBits(0b1));


    ASSERT_EQ(displayBits(grabBitsMSB(word, 59, 2)),
              displayBits(0b10));

    ASSERT_EQ(displayBits(grabBitsMSB(word, 59, 3)),
              displayBits(0b101));

    ASSERT_EQ(displayBits(grabBitsMSB(word, 59, 5)),
              displayBits(0b10101));


    ASSERT_EQ(displayBits(0b10101, 5), "10101");


    ASSERT_EQ(countBits(word), 14);
    ASSERT_EQ(countBits(0b01010000001), 10);
    ASSERT_EQ(countBits(0b00010000001), 8);
    ASSERT_EQ(countBits(0b10000001), 8);
    ASSERT_EQ(countBits(0b01010101), 7);
    ASSERT_EQ(countBits(0b00010101), 5);

}
