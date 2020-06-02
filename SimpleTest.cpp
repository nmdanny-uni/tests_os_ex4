#include "MemoryConstants.h"
#include "PhysicalMemory.h"
#include "VirtualMemory.h"
#include "Common.h"

#include <gtest/gtest.h>
#include <cstdio>
#include <cassert>
#include <map>
#include <random>


#ifdef TEST_CONSTANTS

TEST(FlowTests, FlowTest)
{
    fullyInitialize();
    VMinitialize();
    setLogging(true);

    Trace trace;
    ASSERT_EQ(VMwrite(13, 3), 1) << "VMwrite(13, 3) should succeed";

    // See Flow example pdf(pages 3-14)
    PhysicalAddressToValueMap expectedAddrToValMap {
        {0, 1},
        {3, 2},
        {5, 3},
        {6, 4},
        {9, 3} // contains the value we've just written, 3
    };

    PhysicalAddressToValueMap  gottenAddrToValMap = readGottenPhysicalAddressToValueMap(expectedAddrToValMap);
    ASSERT_EQ(expectedAddrToValMap, gottenAddrToValMap) << "After doing VMwrite(13, 3), Physical memory contents are different than expected";


    word_t gotten;
    ASSERT_EQ(VMread(13, &gotten), 1) << "VMread(13, &gotten) should succeed";
    ASSERT_EQ(gotten, 3) << "Should've read 13 from gotten";

    gottenAddrToValMap = readGottenPhysicalAddressToValueMap(expectedAddrToValMap);
    ASSERT_EQ(expectedAddrToValMap, gottenAddrToValMap) << "Reading a value that was just written shouldn't cause page tables to change.";

    ASSERT_TRUE(LinesContainedInTrace(trace, {
        "PMrestore(4, 6)"
    })) << "See PDF page 13 on why PMrestore is expected after reading value 13";


    // the physical location 14 shouldn't have been changed
    word_t valueAtPhysAddr14;
    PMread(14, &valueAtPhysAddr14);
    ASSERT_EQ(valueAtPhysAddr14, 0) << "Nothing in thee've touched physical address 14";
    // the virtual address 6 will map to physical address 14 (see PDF why this is true)
    PMwrite(14, 1337);

    ASSERT_EQ(VMread(6, &gotten), 1) << "VMread(6, &gotten) should succeed";
    ASSERT_EQ(gotten, 1337) << "VMread(6, &gotten) should've yielded 1337, see PMwrite in test code above";
    // see pdf pages 15-16
    expectedAddrToValMap = {
        {0, 1},
        {2, 5}, // added during VMread(6)
        {3, 2},
        {5, 3},
        {6, 4},
        {9, 3},
        {11, 6},  // added during VMread(6)
        {13, 7}, // added during VMread(6)
        {14, 1337} // added during VMread(6)
    };


    gottenAddrToValMap = readGottenPhysicalAddressToValueMap(expectedAddrToValMap);
    ASSERT_EQ(expectedAddrToValMap, gottenAddrToValMap) << "After doing VMread(6, &gotten), physical memory contents are different than expected.";

    ASSERT_TRUE(LinesContainedInTrace(trace, {
        "PMrestore(7, 3)"
    })) << "see PDF page 16 on why PMrestore is expected after reading virtual address 6";


    // the physical location 15 shouldn't have been changed
    word_t valueAtPhysAddr15;
    PMread(15, &valueAtPhysAddr15);
    ASSERT_EQ(valueAtPhysAddr15, 0) << "Nothing in thee've touched physical address 14";
    // the virtual address 31 will map to physical address 15 (see PDF why this is true)
    PMwrite(15, 7331);


    ASSERT_EQ(VMread(31, &gotten), 1) << "VMread(31, &gotten) should succeed";

    expectedAddrToValMap = {
        {0, 1},
        {1, 4},
        {2, 5},
        {3, 0},
        {4, 0},
        {5, 7},
        {6, 0},
        {7, 2},
        {8, 0},
        {9, 3},
        {10, 0},
        {11, 6},
        {12, 0},
        {13, 0},
        // {14, ?}, 14 contains VM[30], which wasn't saved anywhere
        {15, 7331},
    };

    gottenAddrToValMap = readGottenPhysicalAddressToValueMap(expectedAddrToValMap);
    ASSERT_EQ(expectedAddrToValMap, gottenAddrToValMap) << "After doing VMread(31), physical memory contents are different than expected.";

    ASSERT_TRUE(LinesContainedInTrace(trace, {
        "PMevict(4, 6)", // see pdf page 20
        "PMevict(7, 3)",  // see pdf page 26-27          now page number 3 contains VM[6]=1337, and VM[7]=unknown
        "PMrestore(7, 15)" // see pdf page 26-27
    })) << "See PDF why these evicts were expected";

    // the following isn't in the PDF, but should still work

    // page 3 at offset 0 includes 1337, that is virtual address 0b00110 = 6
    ASSERT_EQ(VMread(6, &gotten), 1) << "should be able to read value";
    ASSERT_EQ(gotten, 1337) << "Page 3 at offset 0, that is, virtual address 6, should include 1337.";
}

#else


/** The simplest test when using the default constants
 *  We are starting with completely empty RAM, we write a single value and then read it.
 *  Since the table is completely empty, we expect that the page tables used during translation
 *  (there are 4) will be created at the first 4 frames (technically the 1st frame already exists
 *                                                       and doesn't need to be created)
 * */
TEST(SimpleTests, Can_Read_And_Write_Memory_Once)
{
    fullyInitialize();
    VMinitialize();
    setLogging(true);
    uint64_t addr = 0b10001011101101110011;
    ASSERT_EQ(VMwrite(addr, 1337), 1) << "write should succeed";

    // The offsets(for the page tables) of the above virtual address are
    // 8, 11, 11, 7, 3
    // Therefore, since we're starting with a completely empty table(see 'fullyInitialize' impl)
    // every page table found will be according to the second criteria in the PDF(an unused frame),
    // and not the first one(as all frames containing empty tables are exactly those we just created
    // during the current address translation, thus they can't be used)
    //
    // So, we expect the following to occur:
    // write(8, 1)     <- table at physical addr 0, offset is 8, next table at frame index 1
    // write(27, 2)    <- table at physical addr 16, offset is 11, next table at frame index 2
    // write(43, 3)    <- table at physical addr 32, offset is 11, next table at frame index 2
    // write(55, 4)    <- table at physical addr 48, offset is 7, next table at frame index 3
    // write(67, 1337) <- table at physical addr 64, offset is 3, write 1337 in this address.

    PhysicalAddressToValueMap expectedAddrToVal {
        {8, 1}, {27, 2}, {43, 3}, {55, 4}, {67, 1337}
    };

    auto gottenAddrToValMap = readGottenPhysicalAddressToValueMap(expectedAddrToVal );
    ASSERT_EQ(expectedAddrToVal , gottenAddrToValMap) << "After doing VMwrite(addr, 1337), physical memory contents are different than expected";

    word_t res;
    ASSERT_EQ(VMread(addr, &res), 1) << "read should succeed";
    ASSERT_EQ(res, 1337) << "wrong value was read";
}


#endif


// Params: test name, from, to, increment, start from blank slate
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

    if (increment == 0 || from > VIRTUAL_MEMORY_SIZE || to > VIRTUAL_MEMORY_SIZE)
    {
        GTEST_SKIP() << "Unable to run this test configuration as the parameters are too big for the given memory constants";
    }


    std::default_random_engine eng = getRandomEngine();
    std::uniform_int_distribution<word_t> dist(1, 0x7FFFFFFF);
    std::map<uint64_t, word_t> ixToVal;


    setLogging(false);
    if (blankSlate) {
        fullyInitialize();
        VMinitialize();
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

    for (uint64_t i = from; i < to; i += increment) {
        word_t value;
        /* printf("reading from %llu %d\n", (long long int) i, value); */
        ASSERT_EQ(VMread(i, &value), 1) << "read should succeed";
        ASSERT_EQ(value, ixToVal.at(i)) << "wrong value was read";
    }
}

std::vector<Params> values = {
    // 'true' for blank slate(RAM only contains 0)
    {"MostBasic", 0, NUM_FRAMES, 1, true},
    {"MoreFrames", 0, 14 * NUM_FRAMES, 1, true},
    // without blank slate, that is, initial memory might be random
    // apart from root table which is initialized to be 0
    {"MostBasic", 0, NUM_FRAMES, 1, false},
    {"MoreFrames", 0, 14 * NUM_FRAMES, 1, false},

    // more variations
    {"ManyAddresses", 0, VIRTUAL_MEMORY_SIZE, PAGE_SIZE, false},
    {"ManyAddresses", 0, VIRTUAL_MEMORY_SIZE, PAGE_SIZE/2, false},
    {"ManyAddresses", 0, VIRTUAL_MEMORY_SIZE, PAGE_SIZE/4, false},
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




// debugging aid, enable this so that after every change, we'll
// check the entire table and see if it's valid.
const bool PERFORM_INTERMEDIATE_CHECKS = false;

TEST(SimpleTests, Can_Read_Then_Write_Memory_Original)
{
    fullyInitialize();
    VMinitialize();
    setLogging(true);


    for (uint64_t i = 0; i < (2 * NUM_FRAMES); ++i) {

        if (5 * i * PAGE_SIZE >= VIRTUAL_MEMORY_SIZE)
        {
            continue;
        }

        std::cout << "Writing to " << 5 * i * PAGE_SIZE << " the value " << i << std::endl;
        ASSERT_EQ(VMwrite(5 * i * PAGE_SIZE, i), 1) << "write should succeed";
        word_t value;
        ASSERT_EQ(VMread(5 * i * PAGE_SIZE, &value), 1) << "immediate read should succeed";
        ASSERT_EQ(uint64_t(value), i) << "immediate read: wrong value read";

        if (PERFORM_INTERMEDIATE_CHECKS)
        {
            setLogging(false);
            for (uint64_t j = 0; j <= i; ++j)
            {
                if (5 * j * PAGE_SIZE >= VIRTUAL_MEMORY_SIZE)
                {
                    continue;
                }
                word_t checkVal;
                ASSERT_EQ(VMread(5 * j * PAGE_SIZE, &checkVal), 1) << "read should succeed";
                if (uint64_t(checkVal) != j)
                {
                    std::cout << "Mismatch for j = " << j << ", expected VM = " << 5 * j * PAGE_SIZE
                              << " to have value " << j
                              << ", got " << checkVal << " instead - re-reading. " << std::endl;
                    setLogging(true);
                    ASSERT_EQ(VMread(5 * j * PAGE_SIZE, &checkVal), 1) << "read should succeed";
                    ASSERT_EQ(uint64_t(checkVal), j) << "immediate reads, re-scan: wrong value was read";
                    setLogging(false);
                    FAIL() << "immediate read failed, only succeeded after re-scan";

                }
            }
            setLogging(true);
        }
    }


    for (uint64_t i = 0; i < (2 * NUM_FRAMES); ++i) {
        if (5 * i * PAGE_SIZE >= VIRTUAL_MEMORY_SIZE)
        {
            continue;
        }
        word_t value;
        ASSERT_EQ(VMread(5 * i * PAGE_SIZE, &value), 1) << "read should succeed";
        std::cout << "Read from " << 5 * i * PAGE_SIZE << " the value " << value << ", the expected value is " << i << std::endl;
        ASSERT_EQ(uint64_t(value), i) << "wrong value was read";
    }
}



TEST(SimpleTests, RandomTest)
{

}