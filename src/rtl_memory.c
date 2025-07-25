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

#include "rtl_memory.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef RTL_DEBUG_BUILD
#include "rtl_log.h"
#endif

#ifdef RTL_DEBUG_BUILD
/**
 * @internal
 * @brief Head of the linked list used to track memory allocations in debug builds.
 */
static rtl_list_entry_t rtl_memory_allocations;
#endif

/**
 * @internal
 * @brief Wrapper function for standard malloc to match our function pointer signature.
 */
static void* default_malloc_wrapper(unsigned long size)
{
  return malloc(size);
}

/**
 * @internal
 * @brief Wrapper function for standard free to match our function pointer signature.
 */
static void default_free_wrapper(void* ptr)
{
  free(ptr);
}

/**
 * @internal
 * @brief Global function pointers for custom memory allocators.
 */
static rtl_malloc_func_t g_malloc_func = NULL;
static rtl_free_func_t g_free_func = NULL;

void* _rtl_malloc(const char* file, unsigned long line, unsigned long size)
{
#ifdef RTL_DEBUG_BUILD
  // It's not a memory leak, it's just a trick to add a bit more
  // info about allocated memory so...
  // ReSharper disable once CppDFAMemoryLeak
  char* data = (char*)g_malloc_func(sizeof(rtl_memory_header_t) + size);

  if (data == NULL) {
    return NULL;
  }

  rtl_memory_header_t* header = (rtl_memory_header_t*)data;
  header->source_location.file = file;
  header->source_location.line = line;
  header->size = size;
  rtl_list_add_tail(&rtl_memory_allocations, &header->link);
  // Mark the memory with 0x77 to be able to debug uninitialized memory
  memset(&data[sizeof(rtl_memory_header_t)], 0x77, size);
  // Return only the needed piece and hide the header
  // ReSharper disable once CppDFAMemoryLeak
  return &data[sizeof(rtl_memory_header_t)];
#else
  return g_malloc_func(size);
#endif
}

char* _rtl_strdup(const char* file, unsigned long line, const char* str)
{
  if (str == NULL) {
    return NULL;
  }

  const size_t len = strlen(str) + 1;  // +1 for null terminator
  char* copy = _rtl_malloc(file, line, len);
  if (copy != NULL) {
    strcpy(copy, str);
  }

  return copy;
}

void rtl_free(void* data)
{
  if (data == NULL) {
    return;
  }

#ifdef RTL_DEBUG_BUILD
  // Find the header with meta information
  rtl_memory_header_t* header = (rtl_memory_header_t*)((char*)data - sizeof(rtl_memory_header_t));
  rtl_list_remove(&header->link);
  // Now we can free the real allocated piece
  g_free_func(header);
#else
  g_free_func(data);
#endif
}

void rtl_memory_init(rtl_malloc_func_t malloc_func, rtl_free_func_t free_func)
{
  // Set custom allocators or default to standard malloc/free wrappers
  g_malloc_func = malloc_func ? malloc_func : default_malloc_wrapper;
  g_free_func = free_func ? free_func : default_free_wrapper;

#ifdef RTL_DEBUG_BUILD
  rtl_list_init(&rtl_memory_allocations);
#endif
}

void rtl_memory_cleanup()
{
#ifdef RTL_DEBUG_BUILD
  rtl_list_entry_t* entry;
  rtl_list_entry_t* safe;
  rtl_list_for_each_safe(entry, safe, &rtl_memory_allocations)
  {
    rtl_memory_header_t* header = rtl_list_record(entry, rtl_memory_header_t, link);
    rtl_log_err("Leaked memory, file: %s, line: %lu, size: %lu", header->source_location.file,
      header->source_location.line, header->size);
    rtl_list_remove(&header->link);
    g_free_func(header);
  }
#endif
}
