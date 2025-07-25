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
#include <time.h>

// Time stamp functionality
static const char* rtl_get_time_stamp(void)
{
  static char stamp[16];
  const time_t now = time(NULL);
  const struct tm* tm_info = localtime(&now);
  strftime(stamp, sizeof(stamp), "%H:%M:%S", tm_info);
  return stamp;
}

static const char* rtl_filename(const char* filename)
{
#ifdef _WIN32
  const char* p = strrchr(filename, '\\');
#else
  const char* p = strrchr(filename, '/');
#endif
  if (p) {
    return p + 1;
  }

#ifdef _WIN32
  p = strrchr(filename, '/');
  if (p) {
    return p + 1;
  }
#endif

  return filename;
}

#define RTL_FILENAME rtl_filename(__FILE__)

// ANSI color codes
#define RTL_COLOR_RESET  "\033[00m"
#define RTL_COLOR_YELLOW "\033[33m"
#define RTL_COLOR_RED    "\033[31m"
#define RTL_COLOR_GREEN  "\033[32m"
#define RTL_COLOR_WHITE  "\033[37m"

// Common log format string
#define RTL_LOG_FORMAT "[%-s|%-s] [%-16s:%4u] (%s) "

// Unified fprintf-based logging macro for colored format
#define _rtl_printf_color(color, lvl, file, line, func, fmt, ...)                                  \
  fprintf(stdout, "%s" RTL_LOG_FORMAT "%s" fmt "\n", color, lvl, rtl_get_time_stamp(), file, line, \
    func, RTL_COLOR_RESET, ##__VA_ARGS__)

#if RTL_DEBUG_LEVEL >= 4
#define rtl_log_inf(_fmt, ...)                                                                     \
  _rtl_printf_color(                                                                               \
    RTL_COLOR_WHITE, "INF", RTL_FILENAME, __LINE__, __FUNCTION__, _fmt, ##__VA_ARGS__)
#else
#define rtl_log_inf(_fmt, ...)                                                                     \
  do {                                                                                             \
  } while (0)
#endif

#if RTL_DEBUG_LEVEL >= 3
#define rtl_log_dbg(_fmt, ...)                                                                     \
  _rtl_printf_color(                                                                               \
    RTL_COLOR_GREEN, "DBG", RTL_FILENAME, __LINE__, __FUNCTION__, _fmt, ##__VA_ARGS__)
#else
#define rtl_log_dbg(_fmt, ...)                                                                     \
  do {                                                                                             \
  } while (0)
#endif

#if RTL_DEBUG_LEVEL >= 2
#define rtl_log_wrn(_fmt, ...)                                                                     \
  _rtl_printf_color(                                                                               \
    RTL_COLOR_YELLOW, "WRN", RTL_FILENAME, __LINE__, __FUNCTION__, _fmt, ##__VA_ARGS__)
#else
#define rtl_log_wrn(_fmt, ...)                                                                     \
  do {                                                                                             \
  } while (0)
#endif

#if RTL_DEBUG_LEVEL >= 1
#define rtl_log_err(_fmt, ...)                                                                     \
  _rtl_printf_color(RTL_COLOR_RED, "ERR", RTL_FILENAME, __LINE__, __FUNCTION__, _fmt, ##__VA_ARGS__)
#else
#define rtl_log_err(_fmt, ...)                                                                     \
  do {                                                                                             \
  } while (0)
#endif
