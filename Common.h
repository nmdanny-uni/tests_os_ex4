#pragma once

#include "MemoryConstants.h"
#include "PhysicalMemory.h"
#include "VirtualMemory.h"

#ifdef USE_SPEEDLOG
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#endif

#include <gtest/gtest.h>
#include <cstdio>
#include <cassert>
#include <map>
#include <random>
#include <regex>
#include <string>
#include <unordered_map>

/** This function is mostly for my convenience when debugging,
 *  you can use it if you have some way to enable/disable print statements at runtime.
 * @param doLog Should logging be enabled or disabled?
 */
void setLogging(bool doLog)
{
	(void)doLog;
#ifdef USE_SPEEDLOG
    if (!doLog) {
        spdlog::set_level(spdlog::level::off);
    } else {
        spdlog::set_level(spdlog::level::info);
    }
#endif
}

/** Maps between expected physical addresses to their values in memory. */
using PhysicalAddressToValueMap = std::unordered_map<uint64_t, word_t>;

/**
 * Reads all physical memory addresses in memory who are present as keys in 'map'
 * @param map Used for its keys, indicates which keys the output map will contain.
 * @return Maps read physical memory addresses to their actual values in RAM
 */
PhysicalAddressToValueMap readGottenPhysicalAddressToValueMap(const PhysicalAddressToValueMap& map)
{
    PhysicalAddressToValueMap gotten;
    for (const auto& kvp: map)
    {
        word_t gottenValue;
        PMread(kvp.first, &gottenValue);
        gotten[kvp.first] = gottenValue;
    }
    return gotten;
}

::testing::AssertionResult LinesContainedInTrace(Trace& trace, std::initializer_list<std::string> lines)
{
    std::string traceOut = trace.GetContents();
    std::stringstream regexMaker;
    int i=0;
    for (const auto& line: lines)
    {
        auto nextPos = traceOut.find(line);
        if (nextPos == std::string::npos) {
            return ::testing::AssertionFailure()
                << "Expected to encounter \"" << line << "\" after checking " << i << " elements, didn't find it.";
        }
        ++i;
        traceOut = traceOut.substr(nextPos);
    }
    return ::testing::AssertionSuccess();
}

// by default, use the same seed so that test results will be consistent with several runs.
const bool USE_DETERMINED_SEED = true;

/** Returns a random engine using either a known seed, or a seed determined at startup
 * @param useDeterminedSeed Use known seed?
 * @return Engine
 */
std::default_random_engine getRandomEngine(bool useDeterminedSeed = USE_DETERMINED_SEED)
{
   if (useDeterminedSeed)
   {
       std::seed_seq seed {
           static_cast<long unsigned int>(1337),
       };
       std::default_random_engine eng {seed};
       return eng;
   } else
   {
       std::random_device rd;
       std::seed_seq seed {
            rd()
       };
       std::default_random_engine eng {seed};
       return eng;
   }
}
