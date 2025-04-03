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

/**
 * @brief Structure to store source code location information.
 */
struct rtl_source_location
{
  const char* file;   /**< Source file name */
  unsigned long line; /**< Source line number */
};

/**
 * @brief Creates a source location structure with current file and line.
 * @return A rtl_source_location structure initialized with current location.
 */
#define RTL_SOURCE_LOCATION ((struct rtl_source_location){ __FILE__, __LINE__ })

#ifdef RTL_DEBUG_BUILD
#define rtl_malloc(size) __rtl_malloc(RTL_SOURCE_LOCATION, size)
#else
#define rtl_malloc(size) __rtl_malloc(size)
#endif

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
 * @param source_location Source location information (only in debug builds).
 * @param size The number of bytes to allocate.
 * @return A pointer to the allocated memory, or NULL on failure.
 * @note Users should typically use the rtl_malloc() macro instead.
 */
void* __rtl_malloc(
#ifdef RTL_DEBUG_BUILD
  struct rtl_source_location source_location,
#endif
  unsigned long size);

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
