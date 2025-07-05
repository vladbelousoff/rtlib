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

#include <string.h>

#include "rtl.h"
#include "rtl_list.h"
#include "rtl_log.h"
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
  // ReSharper disable once CppDFANullDereference
  const rtl_memory_header_t* header = (rtl_memory_header_t*)(data - sizeof(rtl_memory_header_t));
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

// Test structure for list tests
typedef struct test_node
{
  int value;
  rtl_list_entry_t list_entry;
} test_node_t;

// Test list initialization
void test_list_init(void)
{
  rtl_list_entry_t head;
  rtl_list_init(&head);

  TEST_ASSERT_TRUE(rtl_list_empty(&head));
  TEST_ASSERT_EQUAL_PTR(&head, head.next);
  TEST_ASSERT_EQUAL_PTR(&head, head.prev);
}

// Test list first function with empty list
void test_list_first_empty(void)
{
  rtl_list_entry_t head;
  rtl_list_init(&head);

  rtl_list_entry_t* first = rtl_list_first(&head);
  TEST_ASSERT_NULL(first);
}

// Test list first function with one element
void test_list_first_single_element(void)
{
  rtl_list_entry_t head;
  test_node_t node;

  rtl_list_init(&head);
  node.value = 42;
  rtl_list_add_head(&head, &node.list_entry);

  rtl_list_entry_t* first = rtl_list_first(&head);
  TEST_ASSERT_NOT_NULL(first);
  TEST_ASSERT_EQUAL_PTR(&node.list_entry, first);

  test_node_t* first_node = rtl_list_record(first, test_node_t, list_entry);
  TEST_ASSERT_EQUAL(42, first_node->value);
}

// Test list first function with multiple elements
void test_list_first_multiple_elements(void)
{
  rtl_list_entry_t head;
  test_node_t node1, node2, node3;

  rtl_list_init(&head);

  node1.value = 1;
  node2.value = 2;
  node3.value = 3;

  rtl_list_add_head(&head, &node1.list_entry);
  rtl_list_add_head(&head, &node2.list_entry);
  rtl_list_add_head(&head, &node3.list_entry);

  rtl_list_entry_t* first = rtl_list_first(&head);
  TEST_ASSERT_NOT_NULL(first);
  TEST_ASSERT_EQUAL_PTR(&node3.list_entry, first);

  test_node_t* first_node = rtl_list_record(first, test_node_t, list_entry);
  TEST_ASSERT_EQUAL(3, first_node->value);
}

// Test list next function with NULL current
void test_list_next_null_current(void)
{
  rtl_list_entry_t head;
  rtl_list_init(&head);

  rtl_list_entry_t* next = rtl_list_next(NULL, &head);
  TEST_ASSERT_NULL(next);
}

// Test list next function with single element
void test_list_next_single_element(void)
{
  rtl_list_entry_t head;
  test_node_t node;

  rtl_list_init(&head);
  node.value = 42;
  rtl_list_add_head(&head, &node.list_entry);

  rtl_list_entry_t* next = rtl_list_next(&node.list_entry, &head);
  TEST_ASSERT_NULL(next);
}

// Test list next function with multiple elements
void test_list_next_multiple_elements(void)
{
  rtl_list_entry_t head;
  test_node_t node1, node2, node3;

  rtl_list_init(&head);

  node1.value = 1;
  node2.value = 2;
  node3.value = 3;

  rtl_list_add_tail(&head, &node1.list_entry);
  rtl_list_add_tail(&head, &node2.list_entry);
  rtl_list_add_tail(&head, &node3.list_entry);

  rtl_list_entry_t* first = rtl_list_first(&head);
  TEST_ASSERT_NOT_NULL(first);

  rtl_list_entry_t* second = rtl_list_next(first, &head);
  TEST_ASSERT_NOT_NULL(second);
  TEST_ASSERT_EQUAL_PTR(&node2.list_entry, second);

  rtl_list_entry_t* third = rtl_list_next(second, &head);
  TEST_ASSERT_NOT_NULL(third);
  TEST_ASSERT_EQUAL_PTR(&node3.list_entry, third);

  rtl_list_entry_t* fourth = rtl_list_next(third, &head);
  TEST_ASSERT_NULL(fourth);
}

// Test iterating through list using first and next
void test_list_iteration_with_first_and_next(void)
{
  rtl_list_entry_t head;
  test_node_t nodes[5];
  int i;

  rtl_list_init(&head);

  // Add nodes with values 0, 1, 2, 3, 4
  for (i = 0; i < 5; i++) {
    nodes[i].value = i;
    rtl_list_add_tail(&head, &nodes[i].list_entry);
  }

  // Iterate through the list using first and next
  rtl_list_entry_t* current = rtl_list_first(&head);
  i = 0;
  while (current != NULL) {
    test_node_t* node = rtl_list_record(current, test_node_t, list_entry);
    TEST_ASSERT_EQUAL(i, node->value);
    current = rtl_list_next(current, &head);
    i++;
  }

  TEST_ASSERT_EQUAL(5, i);
}

// Test list operations with add_head
void test_list_add_head(void)
{
  rtl_list_entry_t head;
  test_node_t node1, node2, node3;

  rtl_list_init(&head);

  node1.value = 1;
  node2.value = 2;
  node3.value = 3;

  rtl_list_add_head(&head, &node1.list_entry);
  rtl_list_add_head(&head, &node2.list_entry);
  rtl_list_add_head(&head, &node3.list_entry);

  // Check order: should be 3, 2, 1

  rtl_list_entry_t* current = rtl_list_first(&head);
  TEST_ASSERT_NOT_NULL(current);
  TEST_ASSERT_EQUAL(3, rtl_list_record(current, test_node_t, list_entry)->value);

  current = rtl_list_next(current, &head);
  TEST_ASSERT_NOT_NULL(current);
  TEST_ASSERT_EQUAL(2, rtl_list_record(current, test_node_t, list_entry)->value);

  current = rtl_list_next(current, &head);
  TEST_ASSERT_NOT_NULL(current);
  TEST_ASSERT_EQUAL(1, rtl_list_record(current, test_node_t, list_entry)->value);

  current = rtl_list_next(current, &head);
  TEST_ASSERT_NULL(current);
}

// Test list operations with add_tail
void test_list_add_tail(void)
{
  rtl_list_entry_t head;
  test_node_t node1, node2, node3;

  rtl_list_init(&head);

  node1.value = 1;
  node2.value = 2;
  node3.value = 3;

  rtl_list_add_tail(&head, &node1.list_entry);
  rtl_list_add_tail(&head, &node2.list_entry);
  rtl_list_add_tail(&head, &node3.list_entry);

  // Check order: should be 1, 2, 3
  rtl_list_entry_t* current = rtl_list_first(&head);
  TEST_ASSERT_NOT_NULL(current);
  TEST_ASSERT_EQUAL(1, rtl_list_record(current, test_node_t, list_entry)->value);

  current = rtl_list_next(current, &head);
  TEST_ASSERT_NOT_NULL(current);
  TEST_ASSERT_EQUAL(2, rtl_list_record(current, test_node_t, list_entry)->value);

  current = rtl_list_next(current, &head);
  TEST_ASSERT_NOT_NULL(current);
  TEST_ASSERT_EQUAL(3, rtl_list_record(current, test_node_t, list_entry)->value);

  current = rtl_list_next(current, &head);
  TEST_ASSERT_NULL(current);
}

// Test list remove functionality with first and next
void test_list_remove_with_iteration(void)
{
  rtl_list_entry_t head;
  test_node_t node1, node2, node3;

  rtl_list_init(&head);

  node1.value = 1;
  node2.value = 2;
  node3.value = 3;

  rtl_list_add_tail(&head, &node1.list_entry);
  rtl_list_add_tail(&head, &node2.list_entry);
  rtl_list_add_tail(&head, &node3.list_entry);

  // Remove middle element
  rtl_list_remove(&node2.list_entry);

  // Check that list now contains only 1, 3
  rtl_list_entry_t* current = rtl_list_first(&head);
  TEST_ASSERT_NOT_NULL(current);
  TEST_ASSERT_EQUAL(1, rtl_list_record(current, test_node_t, list_entry)->value);

  current = rtl_list_next(current, &head);
  TEST_ASSERT_NOT_NULL(current);
  TEST_ASSERT_EQUAL(3, rtl_list_record(current, test_node_t, list_entry)->value);

  current = rtl_list_next(current, &head);
  TEST_ASSERT_NULL(current);
}

int main(void)
{
  UNITY_BEGIN();

  rtl_log_inf("Info");
  rtl_log_dbg("Debug");
  rtl_log_wrn("Warning");
  rtl_log_err("Error");

  // Memory tests
  RUN_TEST(test_memory_allocation);
  RUN_TEST(test_zero_allocation);
  RUN_TEST(test_multiple_allocations);
  RUN_TEST(test_different_sizes);
  RUN_TEST(test_reallocation_after_free);
  RUN_TEST(test_memory_stress);

  // List tests
  RUN_TEST(test_list_init);
  RUN_TEST(test_list_first_empty);
  RUN_TEST(test_list_first_single_element);
  RUN_TEST(test_list_first_multiple_elements);
  RUN_TEST(test_list_next_null_current);
  RUN_TEST(test_list_next_single_element);
  RUN_TEST(test_list_next_multiple_elements);
  RUN_TEST(test_list_iteration_with_first_and_next);
  RUN_TEST(test_list_add_head);
  RUN_TEST(test_list_add_tail);
  RUN_TEST(test_list_remove_with_iteration);

  return UNITY_END();
}
