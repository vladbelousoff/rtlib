# MIT License
#
# Copyright (c) 2025 Vladislav Belousov
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

cmake_minimum_required(VERSION 3.20)

project(rtlib LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Collect source files
file(GLOB_RECURSE SOURCE_FILES src/*.c)
file(GLOB_RECURSE HEADER_FILES include/*.h)

# Create static or shared library
add_library(rtlib STATIC ${SOURCE_FILES} ${HEADER_FILES})

# Create an alias target for consistent usage in other projects
add_library(rtlib::rtlib ALIAS rtlib)

# Specify include directories
target_include_directories(rtlib PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Add special define for the debug configuration
target_compile_definitions(rtlib PUBLIC
        $<$<CONFIG:Debug>:RTL_DEBUG_BUILD>
)

# Testing executable (only built if this is the main project)
if (CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    # Enable testing
    enable_testing()
    
    # Include FetchContent for Unity
    include(FetchContent)
    
    # Fetch and configure Unity
    FetchContent_Declare(
        unity
        GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
        GIT_TAG v2.5.2
    )
    FetchContent_MakeAvailable(unity)
    
    # Create test executable
    add_executable(rtlib_tests tests/rtlib_tests.c)
    target_link_libraries(rtlib_tests PRIVATE rtlib unity)
    
    # Add the test
    add_test(
        NAME rtlib_tests
        COMMAND rtlib_tests
    )
endif ()