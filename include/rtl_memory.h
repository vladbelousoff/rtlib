// MIT License
//
// Copyright (c) 2025 Vladislav Belousov
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#ifdef RTL_DEBUG_BUILD
#include "rtl_list.h"
#endif

#define RTL_NARGS(...)                 RTL_NARGS_(__VA_ARGS__, 3, 2, 1, 0)
#define RTL_NARGS_(a, b, c, N, ...)    N
#define RTL_CONCAT(a, b)               RTL_CONCAT_(a, b)
#define RTL_CONCAT_(a, b)              a##b

#define rtl_malloc(...)                RTL_CONCAT(rtl_malloc_, RTL_NARGS(__VA_ARGS__))(__VA_ARGS__)
#define rtl_malloc_1(size)             __rtl_malloc(__FILE__, __LINE__, size)
#define rtl_malloc_3(file, line, size) __rtl_malloc(file, line, size)

/**
 * @brief Structure to store source code location information.
 */
struct rtl_source_location
{
  const char* file;   /**< Source file name */
  unsigned long line; /**< Source line number */
};

#ifdef RTL_DEBUG_BUILD
/**
 * @brief Header structure prepended to memory allocations in debug builds.
 *        Contains metadata for tracking memory leaks.
 */
struct rtl_memory_header
{
  struct rtl_list_entry link;
  struct rtl_source_location source_location;
  unsigned long size;
};
#endif

/**
 * @brief Internal memory allocation function.
 * @param file Source file name where the allocation was requested.
 * @param line Source line number where the allocation was requested.
 * @param size The number of bytes to allocate.
 * @return A pointer to the allocated memory, or NULL on failure.
 * @note Users should typically use the rtl_malloc() macro instead.
 */
void* __rtl_malloc(const char* file, unsigned long line, unsigned long size);

/**
 * @brief Frees memory previously allocated by rtl_malloc().
 * @param data Pointer to the memory block to free.
 */
void rtl_free(void* data);

/**
 * @brief Initializes the rtl memory management subsystem.
 *        Must be called before any rtl_malloc() or rtl_free() calls.
 *        In debug builds, initializes the allocation tracking list.
 */
void rtl_memory_init();

/**
 * @brief Cleans up the rtl memory management subsystem.
 *        Should be called at program termination.
 *        In debug builds, checks for memory leaks and reports them to stderr.
 */
void rtl_memory_cleanup();
