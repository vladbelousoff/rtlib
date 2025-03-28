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

#include <stdlib.h>
#include <string.h>

void* rtl_malloc(
#ifdef RTL_DEBUG_BUILD
  const char* filename, unsigned long line,
#endif
  unsigned long size)
{
#ifdef RTL_DEBUG_BUILD
  // It's not a memory leak, it's just a trick to add a bit more
  // info about allocated memory so...
  // ReSharper disable once CppDFAMemoryLeak
  char* data = malloc(sizeof(struct rtl_memory_header) + size);
  struct rtl_memory_header* header = (struct rtl_memory_header*)data;
  header->filename = filename;
  header->line = line;
  header->size = size;
  // Mark the memory with 0x77 to be able to debug uninitialized memory
  memset(&data[sizeof(struct rtl_memory_header)], 0x77, size);
  // Return only the needed piece and hide the header
  return &data[sizeof(struct rtl_memory_header)];
#else
  return malloc(size);
#endif
}

void rtl_free(void* data)
{
#ifdef RTL_DEBUG_BUILD
  // Find the header with meta information
  void* header = (char*)data - sizeof(struct rtl_memory_header);
  // Now we can free the real allocated piece
  free(header);
#else
  free(data);
#endif
}
