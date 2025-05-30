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

#include <stdio.h>
#include <string.h>

#ifndef RTL_DEBUG_LEVEL
#ifndef RTL_DEBUG_BUILD
#define RTL_DEBUG_LEVEL 1
#else
#define RTL_DEBUG_LEVEL 4
#endif
#endif

#ifdef _WIN32
#define RTL_SEPARATOR '\\'
#else
#define RTL_SEPARATOR '/'
#endif

#define RTL_FILENAME ((char*)(strrchr(__FILE__, RTL_SEPARATOR) + 1))

#define __log_printf(lvl, file, line, func, fmt, ...)                                              \
  printf("[%s] %s:%u (%s) " fmt, lvl, file, line, func, ##__VA_ARGS__)

#if RTL_DEBUG_LEVEL >= 4
#define rtl_log_i(_fmt, ...)                                                                       \
  __log_printf("I", RTL_FILENAME, __LINE__, __FUNCTION__, _fmt, ##__VA_ARGS__)
#else
#define rtl_log_i(_fmt, ...)                                                                       \
  do {                                                                                             \
  } while (0)
#endif

#if RTL_DEBUG_LEVEL >= 3
#define rtl_log_d(_fmt, ...)                                                                       \
  __log_printf("D", RTL_FILENAME, __LINE__, __FUNCTION__, _fmt, ##__VA_ARGS__)
#else
#define rtl_log_d(_fmt, ...)                                                                       \
  do {                                                                                             \
  } while (0)
#endif

#if RTL_DEBUG_LEVEL >= 2
#define rtl_log_w(_fmt, ...)                                                                       \
  __log_printf("W", RTL_FILENAME, __LINE__, __FUNCTION__, _fmt, ##__VA_ARGS__)
#else
#define rtl_log_w(_fmt, ...)                                                                       \
  do {                                                                                             \
  } while (0)
#endif

#if RTL_DEBUG_LEVEL >= 1
#define rtl_log_e(_fmt, ...)                                                                       \
  __log_printf("E", RTL_FILENAME, __LINE__, __FUNCTION__, _fmt, ##__VA_ARGS__)
#else
#define rtl_log_e(_fmt, ...)                                                                       \
  do {                                                                                             \
  } while (0)
#endif
