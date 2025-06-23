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

#include "rtl_log.h"

#ifdef RTL_DEBUG_BUILD
/**
 * @internal
 * @brief Head of the linked list used to track memory allocations in debug builds.
 */
static rtl_list_entry_t rtl_memory_allocations;
#endif

void* __rtl_malloc(const char* file, unsigned long line, unsigned long size)
{
#ifdef RTL_DEBUG_BUILD
  // It's not a memory leak, it's just a trick to add a bit more
  // info about allocated memory so...
  // ReSharper disable once CppDFAMemoryLeak
  char* data = malloc(sizeof(rtl_memory_header_t) + size);
  rtl_memory_header_t* header = (rtl_memory_header_t*)data;
  header->source_location.file = file;
  header->source_location.line = line;
  header->size = size;
  rtl_list_add_tail(&rtl_memory_allocations, &header->link);
  // Mark the memory with 0x77 to be able to debug uninitialized memory
  memset(&data[sizeof(rtl_memory_header_t)], 0x77, size);
  // Return only the needed piece and hide the header
  return &data[sizeof(rtl_memory_header_t)];
#else
  return malloc(size);
#endif
}

void rtl_free(void* data)
{
#ifdef RTL_DEBUG_BUILD
  // Find the header with meta information
  rtl_memory_header_t* header =
    (rtl_memory_header_t*)((char*)data - sizeof(rtl_memory_header_t));
  rtl_list_remove(&header->link);
  // Now we can free the real allocated piece
  free(header);
#else
  free(data);
#endif
}

void rtl_memory_init()
{
#ifdef RTL_DEBUG_BUILD
  rtl_list_init(&rtl_memory_allocations);
#endif
  rtl_log_d("Memory initialized\n");
}

void rtl_memory_cleanup()
{
#ifdef RTL_DEBUG_BUILD
  rtl_list_entry_t* entry;
  rtl_list_entry_t* safe;
  rtl_list_for_each_safe(entry, safe, &rtl_memory_allocations)
  {
    rtl_memory_header_t* header = rtl_list_record(entry, rtl_memory_header_t, link);
    rtl_log_e("Leaked memory, file: %s, line: %lu, size: %lu\n", header->source_location.file,
      header->source_location.line, header->size);
    rtl_list_remove(&header->link);
    free(header);
  }
#endif
  rtl_log_d("Memory cleaned up\n");
}
