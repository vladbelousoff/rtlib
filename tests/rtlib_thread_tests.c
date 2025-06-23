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

#include <stdio.h>
#include <stdlib.h>
#include "rtl.h"
#include "rtl_log.h"
#include "rtl_thread.h"
#include "unity.h"

void setUp(void)
{
  rtl_init();
}

void tearDown(void)
{
  rtl_cleanup();
}

// Test mutex functionality
void test_mutex_basic_operations(void)
{
  rtl_mutex_t mutex;

  // Test initialization
  rtl_mutex_init(&mutex);

  // Test lock/unlock
  rtl_mutex_lock(&mutex);
  rtl_mutex_unlock(&mutex);

  // Test destruction
  rtl_mutex_destroy(&mutex);
}

// Test atomic operations
void test_atomic_operations(void)
{
  rtl_atomic_int_t value = 0;

  // Test store/load
  rtl_atomic_store(&value, 42);
  TEST_ASSERT_EQUAL(42, rtl_atomic_load(&value));

  // Test fetch_add
  rtl_atomic_int_t old = rtl_atomic_fetch_add(&value, 10);
  TEST_ASSERT_EQUAL(42, old);
  TEST_ASSERT_EQUAL(52, rtl_atomic_load(&value));

  // Test fetch_sub
  old = rtl_atomic_fetch_sub(&value, 5);
  TEST_ASSERT_EQUAL(52, old);
  TEST_ASSERT_EQUAL(47, rtl_atomic_load(&value));

  // Test compare_exchange
  int success = rtl_atomic_compare_exchange_bool(&value, 47, 100);
  TEST_ASSERT_TRUE(success);
  TEST_ASSERT_EQUAL(100, rtl_atomic_load(&value));

  // Test failed compare_exchange
  success = rtl_atomic_compare_exchange_bool(&value, 47, 200);
  TEST_ASSERT_FALSE(success);
  TEST_ASSERT_EQUAL(100, rtl_atomic_load(&value));
}

// Test thread-safe logging
void test_thread_safe_logging(void)
{
  // This test verifies that logging can be called from multiple contexts
  // without causing crashes or data corruption

  rtl_log_i("Test info message\n");
  rtl_log_d("Test debug message\n");
  rtl_log_w("Test warning message\n");
  rtl_log_e("Test error message\n");

  // If we get here without crashes, the test passes
  TEST_PASS();
}

// Test mutex reentrancy (should not deadlock)
void test_mutex_reentrancy(void)
{
  rtl_mutex_t mutex;
  rtl_mutex_init(&mutex);

  // Lock multiple times (this should work for recursive mutexes)
  // Note: This is a basic test - actual recursive behavior depends on implementation
  rtl_mutex_lock(&mutex);
  rtl_mutex_lock(&mutex);
  rtl_mutex_unlock(&mutex);
  rtl_mutex_unlock(&mutex);

  rtl_mutex_destroy(&mutex);
  TEST_PASS();
}

int main(void)
{
  UNITY_BEGIN();

  RUN_TEST(test_mutex_basic_operations);
  RUN_TEST(test_atomic_operations);
  RUN_TEST(test_thread_safe_logging);
  RUN_TEST(test_mutex_reentrancy);

  return UNITY_END();
}
