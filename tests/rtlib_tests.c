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

#include "rtl.h"
#include "rtl_memory.h"

#include "unity.h"

// Test setup and teardown
void setUp(void)
{
  rtl_init();
}

void tearDown(void)
{
  rtl_cleanup();
}

// Test memory allocation
void test_memory_allocation(void)
{
  char* data = rtl_malloc(10);
  TEST_ASSERT_NOT_NULL(data);

#ifdef RTL_DEBUG_BUILD
  const struct rtl_memory_header* header =
    (struct rtl_memory_header*)(data - sizeof(struct rtl_memory_header));
  TEST_ASSERT_EQUAL(10, header->size);
  // Note: We can't easily test the line number as it would be different in the converted test
#endif

  rtl_free(data);
}

// Test allocating zero bytes
void test_zero_allocation(void)
{
  char* data = rtl_malloc(0);
  TEST_ASSERT_NOT_NULL(data);
  rtl_free(data);
}

// Test multiple allocations and frees
void test_multiple_allocations(void)
{
  char* data1 = rtl_malloc(5);
  char* data2 = rtl_malloc(10);
  char* data3 = rtl_malloc(15);

  TEST_ASSERT_NOT_NULL(data1);
  TEST_ASSERT_NOT_NULL(data2);
  TEST_ASSERT_NOT_NULL(data3);

  rtl_free(data2);
  rtl_free(data1);
  rtl_free(data3);
}

// Test memory allocation with different sizes
void test_different_sizes(void)
{
  char* small_data = rtl_malloc(1);
  char* large_data = rtl_malloc(1000);

  TEST_ASSERT_NOT_NULL(small_data);
  TEST_ASSERT_NOT_NULL(large_data);

  rtl_free(small_data);
  rtl_free(large_data);
}

// Test that freed memory can be reallocated
void test_reallocation_after_free(void)
{
  char* data1 = rtl_malloc(50);
  TEST_ASSERT_NOT_NULL(data1);
  rtl_free(data1);

  char* data2 = rtl_malloc(50);
  TEST_ASSERT_NOT_NULL(data2);
  rtl_free(data2);
}

// Test memory allocation stress test
void test_memory_stress(void)
{
  char* data[100];
  int i;

  // Allocate 100 blocks
  for (i = 0; i < 100; i++) {
    data[i] = rtl_malloc(i + 1);
    TEST_ASSERT_NOT_NULL(data[i]);
  }

  // Free all blocks
  for (i = 0; i < 100; i++) {
    rtl_free(data[i]);
  }
}

int main(void)
{
  UNITY_BEGIN();

  RUN_TEST(test_memory_allocation);
  RUN_TEST(test_zero_allocation);
  RUN_TEST(test_multiple_allocations);
  RUN_TEST(test_different_sizes);
  RUN_TEST(test_reallocation_after_free);
  RUN_TEST(test_memory_stress);

  return UNITY_END();
}
