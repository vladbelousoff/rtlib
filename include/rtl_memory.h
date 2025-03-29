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

#ifdef RTL_DEBUG_BUILD
#define rtl_malloc(size) __rtl_malloc(__FILE__, __LINE__, size)
#else
#define rtl_malloc(size) __rtl_malloc(size)
#endif

#ifdef RTL_DEBUG_BUILD
struct rtl_memory_header
{
  struct rtl_list_entry link;
  const char* file;
  unsigned long line;
  unsigned long size;
};
#endif

void* __rtl_malloc(
#ifdef RTL_DEBUG_BUILD
  const char* file, unsigned long line,
#endif
  unsigned long size);

void rtl_free(void* data);

void rtl_memory_init();
void rtl_memory_cleanup();
