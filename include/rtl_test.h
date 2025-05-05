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
#include <stdlib.h>
#include <string.h>

/**
 * @brief Test context structure to maintain test state
 */
struct rtl_test_context
{
  int tests_run;
  int tests_failed;
  const char* current_test_name;
};

/**
 * @brief Global test context
 */
extern struct rtl_test_context g_rtl_test_ctx;

/**
 * @brief Initialize test context
 */
void rtl_test_init();

/**
 * @brief Print test summary and return appropriate exit code
 * @return 0 if all tests passed, 1 if any failed
 */
int rtl_test_summary();

/**
 * @brief Define a test function
 */
#define RTL_TEST_FUNCTION(name) int name()

/**
 * @brief Run a test function
 * @param func The test function to run
 * @return 0 if test passed, line number if failed
 */
#define RTL_RUN_TEST(func)                                                                         \
  do {                                                                                             \
    g_rtl_test_ctx.current_test_name = #func;                                                      \
    g_rtl_test_ctx.tests_run++;                                                                    \
    int result = func();                                                                           \
    if (result != 0) {                                                                             \
      g_rtl_test_ctx.tests_failed++;                                                               \
      fprintf(stderr, "TEST FAILED: %s at line %d\n", #func, result);                              \
    } else {                                                                                       \
      printf("TEST PASSED: %s\n", #func);                                                          \
    }                                                                                              \
  } while (0)

/**
 * @brief Assert that two values are equal
 */
#define RTL_TEST_EQUAL(a, b)                                                                       \
  if (!(a == b)) {                                                                                 \
    fprintf(stderr, "%s:%d - FAILED: %s != %s\n", __FILE__, __LINE__, #a, #b);                     \
    return __LINE__;                                                                               \
  }

/**
 * @brief Assert that a condition is true
 */
#define RTL_TEST_TRUE(condition)                                                                   \
  if (!(condition)) {                                                                              \
    fprintf(stderr, "%s:%d - FAILED: %s is not true\n", __FILE__, __LINE__, #condition);           \
    return __LINE__;                                                                               \
  }

/**
 * @brief Assert that a condition is false
 */
#define RTL_TEST_FALSE(condition)                                                                  \
  if (condition) {                                                                                 \
    fprintf(stderr, "%s:%d - FAILED: %s is not false\n", __FILE__, __LINE__, #condition);          \
    return __LINE__;                                                                               \
  }

/**
 * @brief Assert that a pointer is not NULL
 */
#define RTL_TEST_NOT_NULL(ptr)                                                                     \
  if ((ptr) == NULL) {                                                                             \
    fprintf(stderr, "%s:%d - FAILED: %s is NULL\n", __FILE__, __LINE__, #ptr);                     \
    return __LINE__;                                                                               \
  }

/**
 * @brief Assert that two strings are equal
 */
#define RTL_TEST_STR_EQUAL(a, b)                                                                   \
  if (strcmp((a), (b)) != 0) {                                                                     \
    fprintf(stderr, "%s:%d - FAILED: string %s != %s\n", __FILE__, __LINE__, #a, #b);              \
    return __LINE__;                                                                               \
  }
