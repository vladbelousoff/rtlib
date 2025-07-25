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
#include <stdlib.h>
#include "rtl_log.h"
#endif

#include "rtl_memory.h"

/**
 * @brief Assert macro that logs an error and aborts if condition is false.
 *        Disabled when RTL_DEBUG_BUILD is defined.
 * @param condition The condition to check
 * @param message Optional error message (can be a format string with args)
 */
#ifdef RTL_DEBUG_BUILD
#define rtl_assert(condition, message, ...)                                                        \
  do {                                                                                             \
    if (!(condition)) {                                                                            \
      rtl_log_err("Assertion failed: %s - " message, #condition, ##__VA_ARGS__);                   \
      exit(EXIT_FAILURE);                                                                          \
    }                                                                                              \
  } while (0)
#else
#define rtl_assert(condition, message, ...)                                                        \
  do {                                                                                             \
    (void)(condition);                                                                             \
  } while (0)
#endif

/**
 * @brief Initializes the runtime library subsystems.
 *        Must be called once at the start of the application.
 * @param malloc_func Custom malloc function (NULL to use standard malloc)
 * @param free_func Custom free function (NULL to use standard free)
 */
void rtl_init(rtl_malloc_func_t malloc_func, rtl_free_func_t free_func);

/**
 * @brief Cleans up the runtime library subsystems.
 *        Should be called once at the end of the application.
 */
void rtl_cleanup();
