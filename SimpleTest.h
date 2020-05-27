#pragma once

#include "MemoryConstants.h"
#include "PhysicalMemory.h"
#include "VirtualMemory.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <gtest/gtest.h>
#include <cstdio>
#include <cassert>
#include <map>
#include <random>


void setLogging(bool doLog)
{
#ifdef USE_SPEEDLOG
    if (!doLog) {
        spdlog::set_level(spdlog::level::off);
    } else {
        spdlog::set_level(spdlog::level::trace);
    }
#endif
}

using PhysicalAddressToValueMap = std::unordered_map<uint64_t, word_t>;

::testing::AssertionResult WroteExpectedValues(const PhysicalAddressToValueMap& map)
{
    for (const auto& kvp: map)
    {
      const uint64_t &physicalAddr = kvp.first;
      const word_t &expectedValue = kvp.second;

      word_t value;
      PMread(physicalAddr, &value);

      if (value != expectedValue) {
        return ::testing::AssertionFailure()
               << "Expected RAM at address " << physicalAddr
               << " to contain value " << expectedValue;
        };
    }
    return ::testing::AssertionSuccess();
}


/** The simplest test
 *  We are starting with completely empty RAM, we write a single value and then read it.
 *  Since the table is completely empty, we expect that the page tables used during translation
 *  (there are 4) will be created at the first 4 frames (technically the 1st frame already exists
 *                                                       and doesn't need to be created)
 * */
TEST(SimpleTests, Can_Read_And_Write_Memory_Once)
{
    fullyInitialize();
    uint64_t addr = 0b10001011101101110011;
    ASSERT_EQ(VMwrite(addr, 1337), 1) << "write should succeed";

    // The offsets(for the page tables) of the above virtual address are
    // 8, 11, 11, 7, 3
    // Therefore, since we're starting with a completely empty table(see 'fullyInitialize' impl)
    // we expect the following to occur:
    // write(8, 1)     <- table at physical addr 0, offset is 8, next table at frame index 1
    // write(27, 2)    <- table at physical addr 16, offset is 11, next table at frame index 2
    // write(43, 3)    <- table at physical addr 32, offset is 11, next table at frame index 2
    // write(55, 4)    <- table at physical addr 48, offset is 7, next table at frame index 3
    // write(67, 1337) <- table at physical addr 64, offset is 3, write 1337 in this address.

    std::unordered_map<uint64_t, word_t> addrToPos {
        {8, 1}, {27, 2}, {43, 3}, {55, 4}, {67, 1337}
    };
    EXPECT_TRUE(WroteExpectedValues(addrToPos));

    word_t res;
    ASSERT_EQ(VMread(addr, &res), 1) << "read should succeed";
    ASSERT_EQ(res, 1337) << "wrong value was read";
}


// Params: test name, "from", "to", "increment", start from blank slate?
using Params = std::tuple<const char*, uint64_t, uint64_t, uint64_t, bool>;

struct ReadWriteTestFixture : public ::testing::TestWithParam<Params>
{};



/** The following test writes random values in a loop,
 *  in the range [from, from + increment, from + 2 * increment, ..., to]
 *
 *  It then checks that the expected values were gotten
 */
TEST_P(ReadWriteTestFixture, Can_Read_Then_Write_Memory_Loop)
{
    const char* testName;
    uint64_t from;
    uint64_t to;
    uint64_t increment;
    bool blankSlate;

    std::tie(testName, from, to, increment, blankSlate) = GetParam();

    std::random_device rd;
    std::seed_seq seed {
        static_cast<long unsigned int>(1337),
        /* rd() */
            };
    std::default_random_engine eng {seed};
    std::uniform_int_distribution<word_t> dist(1337, 0x7FFFFFFF);
    std::map<uint64_t, word_t> ixToVal;


    setLogging(false);
    if (blankSlate) {
        fullyInitialize();
    } else {
        VMinitialize();
    }
    for (uint64_t i = from; i < to; i += increment) {
        word_t genValue = dist(eng);
        ixToVal[i] = genValue;
        /* printf("writing %ud to %llu\n", genValue, (long long int) i); */
        ASSERT_EQ(VMwrite(i, genValue), 1) << "write should succeed";

        word_t value;
        ASSERT_EQ(VMread(i, &value), 1) << "immediate read should succeed";
        ASSERT_EQ(uint64_t(value), genValue) << "immediate read: wrong value read";
    }

    for (uint64_t i = from; i < to; ++i) {
        word_t value;
        /* printf("reading from %llu %d\n", (long long int) i, value); */
        ASSERT_EQ(VMread(i, &value), 1) << "read should succeed";
        ASSERT_EQ(value, ixToVal.at(i)) << "wrong value was read";
    }
}

std::vector<Params> values = {
    {"MostBasic", 0, NUM_FRAMES, 1, true},
    {"MoreFrames", 0, 14 * NUM_FRAMES, 1, true},
    // {"MostBasic", 0, NUM_FRAMES, 1, false},
    // {"MoreFrames", 0, 14 * NUM_FRAMES, 1, false}
};

INSTANTIATE_TEST_SUITE_P(ReadWriteTests, ReadWriteTestFixture,
                         ::testing::ValuesIn(values),
                         [](const testing::TestParamInfo<ReadWriteTestFixture::ParamType>& info) {
                             std::stringstream ss;
                             const char* testName = std::get<0>(info.param);
                             uint64_t from = std::get<1>(info.param);
                             uint64_t to = std::get<2>(info.param);
                             uint64_t increment = std::get<3>(info.param);
                             uint64_t blank = std::get<4>(info.param);
                             ss << testName;
                             if (blank) {
                                 ss << "_blank_slate";
                             }
                             ss << "_from_" << from;
                             ss << "_to_" << to;
                             ss << "_increment_" << increment;
                             return ss.str();
                         }
);




TEST(SimpleTests, Can_Read_Then_Write_Memory_Original)
{
    VMinitialize();
    setLogging(true);
    for (uint64_t i = 0; i < (2 * NUM_FRAMES); ++i) {
        printf("writing to %llu\n", (long long int) i);
        ASSERT_EQ(VMwrite(5 * i * PAGE_SIZE, i), 1) << "write should succeed";
        word_t value;
        ASSERT_EQ(VMread(5 * i * PAGE_SIZE, &value), 1) << "immediate read should succeed";
        ASSERT_EQ(uint64_t(value), i) << "immediate read: wrong value read";
    }

    for (uint64_t i = 0; i < (2 * NUM_FRAMES); ++i) {
        word_t value;
        ASSERT_EQ(VMread(5 * i * PAGE_SIZE, &value), 1) << "read should succeed";
        printf("reading from %llu %d\n", (long long int) i, value);
        ASSERT_EQ(uint64_t(value), i) << "wrong value was read";
    }
}
