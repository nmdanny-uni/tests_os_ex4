cmake_minimum_required(VERSION 3.1)

project(ex4_testers VERSION 1.0 LANGUAGES C CXX)

#######################################
### SETTING UP GOOGLE TEST ###

# Download and unpack googletest at configure time
configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
    message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
    message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
        ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
        EXCLUDE_FROM_ALL)

# The gtest/gtest_main targets carry header search path
# dependencies automatically when using CMake 2.8.11 or
# later. Otherwise we have to add them here ourselves.
if (CMAKE_VERSION VERSION_LESS 2.8.11)
    include_directories("${gtest_SOURCE_DIR}/include")
endif()


#######################################

if(NOT vm_source_files)
    message(FATAL_ERROR "You did not set the list of source files in your CMakeLists.txt, make sure you read the README")
endif()



set(test_sources SimpleTest.cpp Common.h)

set(test_compile_options -Wall -Wextra -g)

if(INCLUDE_PRIVATE_TESTS)
    list(APPEND test_sources private/BitUtilsTests.cpp)
endif()

add_executable(ex4TestsNormalConstants ${test_sources})
target_include_directories(ex4TestsNormalConstants PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../)
target_link_libraries(ex4TestsNormalConstants PRIVATE VirtualMemory gtest_main)
set_property(TARGET ex4TestsNormalConstants PROPERTY CXX_STANDARD 11)
target_compile_options(ex4TestsNormalConstants PUBLIC ${test_compile_options})

# The difference between these configurations is that the one below links to 'TestVirtualMemory'
# and the above links to 'VirtualMemory' library.

add_executable(ex4TestsSmallConstants ${test_sources})
target_include_directories(ex4TestsSmallConstants PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../)
target_link_libraries(ex4TestsSmallConstants PRIVATE TestVirtualMemory gtest_main)
set_property(TARGET ex4TestsSmallConstants PROPERTY CXX_STANDARD 11)
target_compile_options(ex4TestsSmallConstants PUBLIC ${test_compile_options})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
