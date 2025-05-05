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
#include "rtl_test.h"

#include <stdio.h>

// Test memory allocation
RTL_TEST_FUNCTION(test_memory_allocation)
{
  char* data = rtl_malloc(10);
  RTL_TEST_NOT_NULL(data);

#ifdef RTL_DEBUG_BUILD
  const struct rtl_memory_header* header =
    (struct rtl_memory_header*)(data - sizeof(struct rtl_memory_header));
  RTL_TEST_EQUAL(header->size, 10);
  RTL_TEST_EQUAL(header->source_location.line, __LINE__ - 7);
#endif

  rtl_free(data);
  return 0;
}

// Test allocating zero bytes
RTL_TEST_FUNCTION(test_zero_allocation)
{
  char* data = rtl_malloc(0);
  RTL_TEST_NOT_NULL(data);
  rtl_free(data);
  return 0;
}

// Test multiple allocations and frees
RTL_TEST_FUNCTION(test_multiple_allocations)
{
  char* data1 = rtl_malloc(5);
  char* data2 = rtl_malloc(10);
  char* data3 = rtl_malloc(15);

  RTL_TEST_NOT_NULL(data1);
  RTL_TEST_NOT_NULL(data2);
  RTL_TEST_NOT_NULL(data3);

  rtl_free(data2);
  rtl_free(data1);
  rtl_free(data3);
  return 0;
}

int main(int argc, char** argv)
{
  (void)argc;
  (void)argv;

  // Init the library
  rtl_init();

  // Init test framework
  rtl_test_init();

  // Run tests
  RTL_RUN_TEST(test_memory_allocation);
  RTL_RUN_TEST(test_zero_allocation);
  RTL_RUN_TEST(test_multiple_allocations);

  // Return summary status
  const int summary = rtl_test_summary();

  // Clean up the library
  rtl_cleanup();

  return summary;
}
