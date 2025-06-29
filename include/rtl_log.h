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

// Log file functionality
static FILE* rtl_get_log_file(void)
{
  static FILE* log_file = NULL;
  static int initialized = 0;

  if (!initialized) {
    const time_t now = time(NULL);
    const struct tm* tm_info = localtime(&now);
    char filename[64];
    strftime(filename, sizeof(filename), "logs_%d-%m-%Y_%H-%M-%S.txt", tm_info);
    log_file = fopen(filename, "w");
    initialized = 1;
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

// Common log format string
#define RTL_LOG_FORMAT "[%-s|%-s] [%-16s:%4u] (%-16s) "

// Unified fprintf-based logging macro for standard format
#define __log_fprintf_std(stream, lvl, file, line, func, fmt, ...)                                 \
  fprintf(stream, RTL_LOG_FORMAT fmt, lvl, rtl_get_time_stamp(), file, line, func, ##__VA_ARGS__)

// Unified fprintf-based logging macro for colored format
#define __log_fprintf_color(stream, color, lvl, file, line, func, fmt, ...)                        \
  fprintf(stream, "%s" RTL_LOG_FORMAT "%s" fmt, color, lvl, rtl_get_time_stamp(), file, line,      \
    func, RTL_COLOR_RESET, ##__VA_ARGS__)

#define __log_printf(lvl, file, line, func, fmt, ...)                                              \
  do {                                                                                             \
    __log_fprintf_std(stdout, lvl, file, line, func, fmt, ##__VA_ARGS__);                          \
    FILE* log_f = rtl_get_log_file();                                                              \
    if (log_f) {                                                                                   \
      __log_fprintf_std(log_f, lvl, file, line, func, fmt, ##__VA_ARGS__);                         \
      fflush(log_f);                                                                               \
    }                                                                                              \
  } while (0)

#define __log_printf_color(lvl, color, file, line, func, fmt, ...)                                 \
  do {                                                                                             \
    __log_fprintf_color(stdout, color, lvl, file, line, func, fmt, ##__VA_ARGS__);                 \
    FILE* log_f = rtl_get_log_file();                                                              \
    if (log_f) {                                                                                   \
      __log_fprintf_std(log_f, lvl, file, line, func, fmt, ##__VA_ARGS__);                         \
      fflush(log_f);                                                                               \
    }                                                                                              \
  } while (0)

#if RTL_DEBUG_LEVEL >= 4
#define rtl_log_inf(_fmt, ...)                                                                     \
  __log_printf("INF", RTL_FILENAME, __LINE__, __FUNCTION__, _fmt, ##__VA_ARGS__)
#else
#define rtl_log_inf(_fmt, ...)                                                                     \
  do {                                                                                             \
  } while (0)
#endif

#if RTL_DEBUG_LEVEL >= 3
#define rtl_log_dbg(_fmt, ...)                                                                     \
  __log_printf_color(                                                                              \
    "DBG", RTL_COLOR_GREEN, RTL_FILENAME, __LINE__, __FUNCTION__, _fmt, ##__VA_ARGS__)
#else
#define rtl_log_dbg(_fmt, ...)                                                                     \
  do {                                                                                             \
  } while (0)
#endif

#if RTL_DEBUG_LEVEL >= 2
#define rtl_log_wrn(_fmt, ...)                                                                     \
  __log_printf_color(                                                                              \
    "WRN", RTL_COLOR_YELLOW, RTL_FILENAME, __LINE__, __FUNCTION__, _fmt, ##__VA_ARGS__)
#else
#define rtl_log_wrn(_fmt, ...)                                                                     \
  do {                                                                                             \
  } while (0)
#endif

#if RTL_DEBUG_LEVEL >= 1
#define rtl_log_err(_fmt, ...)                                                                     \
  __log_printf_color(                                                                              \
    "ERR", RTL_COLOR_RED, RTL_FILENAME, __LINE__, __FUNCTION__, _fmt, ##__VA_ARGS__)
#else
#define rtl_log_err(_fmt, ...)                                                                     \
  do {                                                                                             \
  } while (0)
#endif
