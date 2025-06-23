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
#include <time.h>
#include "rtl_thread.h"

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

// ANSI color codes
#define RTL_COLOR_RESET  "\033[0m"
#define RTL_COLOR_YELLOW "\033[33m"
#define RTL_COLOR_RED    "\033[31m"
#define RTL_COLOR_GREEN  "\033[32m"

// Global mutex for thread safety
static rtl_mutex_t rtl_log_mutex;
static rtl_atomic_int_t rtl_log_mutex_initialized = 0;

static void rtl_log_mutex_init(void)
{
  if (!rtl_atomic_compare_exchange_bool(&rtl_log_mutex_initialized, 0, 1)) {
    return;  // Already initialized
  }
  rtl_mutex_init(&rtl_log_mutex);
}

static void rtl_log_mutex_lock(void)
{
  rtl_log_mutex_init();
  rtl_mutex_lock(&rtl_log_mutex);
}

static void rtl_log_mutex_unlock(void)
{
  rtl_mutex_unlock(&rtl_log_mutex);
}

// Log file functionality
static FILE* rtl_get_log_file(void)
{
  static FILE* log_file = NULL;
  static rtl_atomic_int_t initialized = 0;

  if (!rtl_atomic_load(&initialized)) {
    const time_t now = time(NULL);
    const struct tm* tm_info = localtime(&now);
    char filename[64];
    strftime(filename, sizeof(filename), "logs_%d-%m-%Y_%H-%M-%S.txt", tm_info);
    log_file = fopen(filename, "w");
    rtl_atomic_store(&initialized, 1);
  }

  return log_file;
}

// Time stamp functionality
static const char* rtl_get_time_stamp(void)
{
  static char stamp[16];
  const time_t now = time(NULL);
  const struct tm* tm_info = localtime(&now);
  strftime(stamp, sizeof(stamp), "%H:%M:%S", tm_info);
  return stamp;
}

#define __log_printf(lvl, file, line, func, fmt, ...)                                              \
  do {                                                                                             \
    rtl_log_mutex_lock();                                                                          \
    printf("[%-s|%-s] [%-30s:%5u] (%-30s) " fmt, lvl, rtl_get_time_stamp(), file, line, func,      \
      ##__VA_ARGS__);                                                                              \
    FILE* log_f = rtl_get_log_file();                                                              \
    if (log_f) {                                                                                   \
      fprintf(log_f, "[%-s|%-s] [%-30s:%5u] (%-30s) " fmt, lvl, rtl_get_time_stamp(), file, line,  \
        func, ##__VA_ARGS__);                                                                      \
      fflush(log_f);                                                                               \
    }                                                                                              \
    rtl_log_mutex_unlock();                                                                        \
  } while (0)

#define __log_printf_color(lvl, color, file, line, func, fmt, ...)                                 \
  do {                                                                                             \
    rtl_log_mutex_lock();                                                                          \
    printf("%s[%-s|%-s]%s [%-30s:%5u] (%-30s) " fmt, color, lvl, rtl_get_time_stamp(),             \
      RTL_COLOR_RESET, file, line, func, ##__VA_ARGS__);                                           \
    FILE* log_f = rtl_get_log_file();                                                              \
    if (log_f) {                                                                                   \
      fprintf(log_f, "[%-s|%-s] [%-30s:%5u] (%-30s) " fmt, lvl, rtl_get_time_stamp(), file, line,  \
        func, ##__VA_ARGS__);                                                                      \
      fflush(log_f);                                                                               \
    }                                                                                              \
    rtl_log_mutex_unlock();                                                                        \
  } while (0)

#if RTL_DEBUG_LEVEL >= 4
#define rtl_log_i(_fmt, ...)                                                                       \
  __log_printf("INF", RTL_FILENAME, __LINE__, __FUNCTION__, _fmt, ##__VA_ARGS__)
#else
#define rtl_log_i(_fmt, ...)                                                                       \
  do {                                                                                             \
  } while (0)
#endif

#if RTL_DEBUG_LEVEL >= 3
#define rtl_log_d(_fmt, ...)                                                                       \
  __log_printf_color(                                                                              \
    "DBG", RTL_COLOR_GREEN, RTL_FILENAME, __LINE__, __FUNCTION__, _fmt, ##__VA_ARGS__)
#else
#define rtl_log_d(_fmt, ...)                                                                       \
  do {                                                                                             \
  } while (0)
#endif

#if RTL_DEBUG_LEVEL >= 2
#define rtl_log_w(_fmt, ...)                                                                       \
  __log_printf_color(                                                                              \
    "WRN", RTL_COLOR_YELLOW, RTL_FILENAME, __LINE__, __FUNCTION__, _fmt, ##__VA_ARGS__)
#else
#define rtl_log_w(_fmt, ...)                                                                       \
  do {                                                                                             \
  } while (0)
#endif

#if RTL_DEBUG_LEVEL >= 1
#define rtl_log_e(_fmt, ...)                                                                       \
  __log_printf_color(                                                                              \
    "ERR", RTL_COLOR_RED, RTL_FILENAME, __LINE__, __FUNCTION__, _fmt, ##__VA_ARGS__)
#else
#define rtl_log_e(_fmt, ...)                                                                       \
  do {                                                                                             \
  } while (0)
#endif
