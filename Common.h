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
               << " to contain value " << expectedValue << ", but instead it contains " << value;
        };
    }
    return ::testing::AssertionSuccess();
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


