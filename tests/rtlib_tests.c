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
#include "rtl_hash.h"
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

// Test list length with empty list
void test_list_length_empty(void)
{
  rtl_list_entry_t head;
  rtl_list_init(&head);

  unsigned long length = rtl_list_length(&head);
  TEST_ASSERT_EQUAL(0, length);
}

// Test list length with single element
void test_list_length_single_element(void)
{
  rtl_list_entry_t head;
  test_node_t node;

  rtl_list_init(&head);
  node.value = 42;
  rtl_list_add_head(&head, &node.list_entry);

  unsigned long length = rtl_list_length(&head);
  TEST_ASSERT_EQUAL(1, length);
}

// Test list length with multiple elements added with add_head
void test_list_length_multiple_elements_head(void)
{
  rtl_list_entry_t head;
  test_node_t node1, node2, node3;

  rtl_list_init(&head);

  node1.value = 1;
  node2.value = 2;
  node3.value = 3;

  rtl_list_add_head(&head, &node1.list_entry);
  TEST_ASSERT_EQUAL(1, rtl_list_length(&head));

  rtl_list_add_head(&head, &node2.list_entry);
  TEST_ASSERT_EQUAL(2, rtl_list_length(&head));

  rtl_list_add_head(&head, &node3.list_entry);
  TEST_ASSERT_EQUAL(3, rtl_list_length(&head));
}

// Test list length with multiple elements added with add_tail
void test_list_length_multiple_elements_tail(void)
{
  rtl_list_entry_t head;
  test_node_t node1, node2, node3;

  rtl_list_init(&head);

  node1.value = 1;
  node2.value = 2;
  node3.value = 3;

  rtl_list_add_tail(&head, &node1.list_entry);
  TEST_ASSERT_EQUAL(1, rtl_list_length(&head));

  rtl_list_add_tail(&head, &node2.list_entry);
  TEST_ASSERT_EQUAL(2, rtl_list_length(&head));

  rtl_list_add_tail(&head, &node3.list_entry);
  TEST_ASSERT_EQUAL(3, rtl_list_length(&head));
}

// Test list length after removing elements
void test_list_length_after_remove(void)
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

  TEST_ASSERT_EQUAL(3, rtl_list_length(&head));

  // Remove middle element
  rtl_list_remove(&node2.list_entry);
  TEST_ASSERT_EQUAL(2, rtl_list_length(&head));

  // Remove first element
  rtl_list_remove(&node1.list_entry);
  TEST_ASSERT_EQUAL(1, rtl_list_length(&head));

  // Remove last element
  rtl_list_remove(&node3.list_entry);
  TEST_ASSERT_EQUAL(0, rtl_list_length(&head));
}

// Test list length with larger number of elements
void test_list_length_stress(void)
{
  rtl_list_entry_t head;
  test_node_t nodes[50];
  int i;

  rtl_list_init(&head);

  // Add 50 elements
  for (i = 0; i < 50; i++) {
    nodes[i].value = i;
    rtl_list_add_tail(&head, &nodes[i].list_entry);
    TEST_ASSERT_EQUAL(i + 1, rtl_list_length(&head));
  }

  // Remove elements one by one
  for (i = 0; i < 50; i++) {
    rtl_list_remove(&nodes[i].list_entry);
    TEST_ASSERT_EQUAL(50 - i - 1, rtl_list_length(&head));
  }
}

// Test rtl_list_for_each_indexed with empty list
void test_list_for_each_indexed_empty(void)
{
  rtl_list_entry_t head;
  rtl_list_init(&head);

  rtl_list_entry_t* position;
  unsigned long index = 0;
  int iteration_count = 0;

  rtl_list_for_each_indexed(index, position, &head)
  {
    iteration_count++;
  }

  TEST_ASSERT_EQUAL(0, iteration_count);
}

// Test rtl_list_for_each_indexed with single element
void test_list_for_each_indexed_single_element(void)
{
  rtl_list_entry_t head;
  test_node_t node;

  rtl_list_init(&head);
  node.value = 42;
  rtl_list_add_head(&head, &node.list_entry);

  rtl_list_entry_t* position;
  int iteration_count = 0;
  unsigned long index = 0;
  unsigned long expected_index = 0;

  rtl_list_for_each_indexed(index, position, &head)
  {
    TEST_ASSERT_EQUAL(expected_index, index);
    test_node_t* current_node = rtl_list_record(position, test_node_t, list_entry);
    TEST_ASSERT_EQUAL(42, current_node->value);
    iteration_count++;
    expected_index++;
  }

  TEST_ASSERT_EQUAL(1, iteration_count);
}

// Test rtl_list_for_each_indexed with multiple elements
void test_list_for_each_indexed_multiple_elements(void)
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

  rtl_list_entry_t* position;
  int iteration_count = 0;
  unsigned long index = 0;
  unsigned long expected_index = 0;

  rtl_list_for_each_indexed(index, position, &head)
  {
    TEST_ASSERT_EQUAL(expected_index, index);
    test_node_t* current_node = rtl_list_record(position, test_node_t, list_entry);
    TEST_ASSERT_EQUAL(expected_index, current_node->value);
    iteration_count++;
    expected_index++;
  }

  TEST_ASSERT_EQUAL(5, iteration_count);
}

// Test rtl_list_for_each_indexed order with head insertions
void test_list_for_each_indexed_head_insertion_order(void)
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

  // Order should be: 3, 2, 1 (reverse insertion order for head insertions)
  rtl_list_entry_t* position;
  int iteration_count = 0;
  unsigned long index = 0;

  rtl_list_for_each_indexed(index, position, &head)
  {
    const int expected_values[] = { 3, 2, 1 };
    TEST_ASSERT_EQUAL(iteration_count, index);
    const test_node_t* current_node = rtl_list_record(position, test_node_t, list_entry);
    TEST_ASSERT_EQUAL(expected_values[iteration_count], current_node->value);
    iteration_count++;
  }

  TEST_ASSERT_EQUAL(3, iteration_count);
}

// Test rtl_list_for_each_indexed with larger list
void test_list_for_each_indexed_stress(void)
{
  rtl_list_entry_t head;
  test_node_t nodes[100];
  int i;

  rtl_list_init(&head);

  // Add 100 nodes
  for (i = 0; i < 100; i++) {
    nodes[i].value = i;
    rtl_list_add_tail(&head, &nodes[i].list_entry);
  }

  rtl_list_entry_t* position;
  int iteration_count = 0;
  unsigned long index = 0;

  rtl_list_for_each_indexed(index, position, &head)
  {
    TEST_ASSERT_EQUAL(iteration_count, index);
    test_node_t* current_node = rtl_list_record(position, test_node_t, list_entry);
    TEST_ASSERT_EQUAL(iteration_count, current_node->value);
    iteration_count++;
  }

  TEST_ASSERT_EQUAL(100, iteration_count);
}

void test_rtl_assert_success(void)
{
  // Test that assert passes when condition is true
  rtl_assert(1 == 1, "This should not trigger");
  rtl_assert(5 >= 3, "Math should work: %d >= %d", 5, 3);

  // If we reach here, the assertions passed correctly
  TEST_ASSERT_TRUE(1);
}

// Hash table tests

// Test hash table initialization and cleanup
void test_hash_table_init_cleanup(void)
{
  rtl_hash_table_t table;
  bool result = rtl_hash_table_init(&table, 10, rtl_hash_fnv1a, rtl_hash_key_compare_bytes);
  TEST_ASSERT_TRUE(result);
  TEST_ASSERT_EQUAL(0, rtl_hash_table_size(&table));
  TEST_ASSERT_TRUE(rtl_hash_table_empty(&table));
  TEST_ASSERT_EQUAL(0.0, rtl_hash_table_load_factor(&table));

  rtl_hash_table_cleanup(&table);
}

// Test hash table insertion
void test_hash_table_insert(void)
{
  rtl_hash_table_t table;
  bool result = rtl_hash_table_init(&table, 5, rtl_hash_fnv1a, rtl_hash_key_compare_bytes);
  TEST_ASSERT_TRUE(result);

  int key = 42;
  int value = 123;

  result = rtl_hash_table_insert(&table, &key, sizeof(key), &value, sizeof(value));
  TEST_ASSERT_TRUE(result);
  TEST_ASSERT_EQUAL(1, rtl_hash_table_size(&table));
  TEST_ASSERT_FALSE(rtl_hash_table_empty(&table));

  rtl_hash_table_cleanup(&table);
}

// Test hash table find
void test_hash_table_find(void)
{
  rtl_hash_table_t table;
  bool result = rtl_hash_table_init(&table, 5, rtl_hash_fnv1a, rtl_hash_key_compare_bytes);
  TEST_ASSERT_TRUE(result);

  int key = 42;
  int value = 123;

  // Insert a value
  result = rtl_hash_table_insert(&table, &key, sizeof(key), &value, sizeof(value));
  TEST_ASSERT_TRUE(result);

  // Find the value
  unsigned long found_value_size;
  int* found_value = (int*)rtl_hash_table_find(&table, &key, sizeof(key), &found_value_size);
  TEST_ASSERT_NOT_NULL(found_value);
  TEST_ASSERT_EQUAL(sizeof(int), found_value_size);
  TEST_ASSERT_EQUAL(123, *found_value);

  // Try to find non-existent key
  int nonexistent_key = 99;
  void* not_found = rtl_hash_table_find(&table, &nonexistent_key, sizeof(nonexistent_key), NULL);
  TEST_ASSERT_NULL(not_found);

  rtl_hash_table_cleanup(&table);
}

// Test hash table update (insert existing key)
void test_hash_table_update(void)
{
  rtl_hash_table_t table;
  bool result = rtl_hash_table_init(&table, 5, rtl_hash_fnv1a, rtl_hash_key_compare_bytes);
  TEST_ASSERT_TRUE(result);

  const int key = 42;
  const int value1 = 123;
  const int value2 = 456;

  // Insert initial value
  result = rtl_hash_table_insert(&table, &key, sizeof(key), &value1, sizeof(value1));
  TEST_ASSERT_TRUE(result);
  TEST_ASSERT_EQUAL(1, rtl_hash_table_size(&table));

  // Update with new value
  result = rtl_hash_table_insert(&table, &key, sizeof(key), &value2, sizeof(value2));
  TEST_ASSERT_TRUE(result);
  TEST_ASSERT_EQUAL(1, rtl_hash_table_size(&table));  // Size should not change

  // Verify the new value
  const int* found_value = rtl_hash_table_find(&table, &key, sizeof(key), NULL);
  TEST_ASSERT_NOT_NULL(found_value);
  TEST_ASSERT_EQUAL(456, *found_value);

  rtl_hash_table_cleanup(&table);
}

// Test hash table remove
void test_hash_table_remove(void)
{
  rtl_hash_table_t table;
  bool result = rtl_hash_table_init(&table, 5, rtl_hash_fnv1a, rtl_hash_key_compare_bytes);
  TEST_ASSERT_TRUE(result);

  int key = 42;
  int value = 123;

  // Insert a value
  result = rtl_hash_table_insert(&table, &key, sizeof(key), &value, sizeof(value));
  TEST_ASSERT_TRUE(result);
  TEST_ASSERT_EQUAL(1, rtl_hash_table_size(&table));

  // Remove the value
  result = rtl_hash_table_remove(&table, &key, sizeof(key));
  TEST_ASSERT_TRUE(result);
  TEST_ASSERT_EQUAL(0, rtl_hash_table_size(&table));
  TEST_ASSERT_TRUE(rtl_hash_table_empty(&table));

  // Verify it's gone
  void* found_value = rtl_hash_table_find(&table, &key, sizeof(key), NULL);
  TEST_ASSERT_NULL(found_value);

  // Try to remove non-existent key
  result = rtl_hash_table_remove(&table, &key, sizeof(key));
  TEST_ASSERT_FALSE(result);

  rtl_hash_table_cleanup(&table);
}

// Test hash table with multiple entries
void test_hash_table_multiple_entries(void)
{
  rtl_hash_table_t table;
  bool result = rtl_hash_table_init(&table, 3, rtl_hash_fnv1a, rtl_hash_key_compare_bytes);
  TEST_ASSERT_TRUE(result);

  // Insert multiple key-value pairs
  for (int i = 0; i < 10; i++) {
    int key = i;
    int value = i * 10;
    result = rtl_hash_table_insert(&table, &key, sizeof(key), &value, sizeof(value));
    TEST_ASSERT_TRUE(result);
  }

  TEST_ASSERT_EQUAL(10, rtl_hash_table_size(&table));
  TEST_ASSERT_FALSE(rtl_hash_table_empty(&table));

  // Verify all values can be found
  for (int i = 0; i < 10; i++) {
    int key = i;
    int* found_value = (int*)rtl_hash_table_find(&table, &key, sizeof(key), NULL);
    TEST_ASSERT_NOT_NULL(found_value);
    TEST_ASSERT_EQUAL(i * 10, *found_value);
  }

  // Remove every other entry
  for (int i = 0; i < 10; i += 2) {
    int key = i;
    result = rtl_hash_table_remove(&table, &key, sizeof(key));
    TEST_ASSERT_TRUE(result);
  }

  TEST_ASSERT_EQUAL(5, rtl_hash_table_size(&table));

  // Verify remaining entries
  for (int i = 0; i < 10; i++) {
    int key = i;
    int* found_value = (int*)rtl_hash_table_find(&table, &key, sizeof(key), NULL);
    if (i % 2 == 0) {
      TEST_ASSERT_NULL(found_value);  // Should be removed
    } else {
      TEST_ASSERT_NOT_NULL(found_value);  // Should still exist
      TEST_ASSERT_EQUAL(i * 10, *found_value);
    }
  }

  rtl_hash_table_cleanup(&table);
}

// Test hash table with string keys
void test_hash_table_string_keys(void)
{
  rtl_hash_table_t table;
  bool result = rtl_hash_table_init(&table, 5, rtl_hash_fnv1a, rtl_hash_key_compare_string);
  TEST_ASSERT_TRUE(result);

  const char* key1 = "hello";
  const char* key2 = "world";
  const char* key3 = "test";
  int value1 = 100;
  int value2 = 200;
  int value3 = 300;

  // Insert string keys
  result = rtl_hash_table_insert(&table, key1, strlen(key1) + 1, &value1, sizeof(value1));
  TEST_ASSERT_TRUE(result);

  result = rtl_hash_table_insert(&table, key2, strlen(key2) + 1, &value2, sizeof(value2));
  TEST_ASSERT_TRUE(result);

  result = rtl_hash_table_insert(&table, key3, strlen(key3) + 1, &value3, sizeof(value3));
  TEST_ASSERT_TRUE(result);

  TEST_ASSERT_EQUAL(3, rtl_hash_table_size(&table));

  // Find values by string keys
  const int* found_value = (int*)rtl_hash_table_find(&table, key1, strlen(key1) + 1, NULL);
  TEST_ASSERT_NOT_NULL(found_value);
  TEST_ASSERT_EQUAL(100, *found_value);

  found_value = (int*)rtl_hash_table_find(&table, key2, strlen(key2) + 1, NULL);
  TEST_ASSERT_NOT_NULL(found_value);
  TEST_ASSERT_EQUAL(200, *found_value);

  found_value = (int*)rtl_hash_table_find(&table, key3, strlen(key3) + 1, NULL);
  TEST_ASSERT_NOT_NULL(found_value);
  TEST_ASSERT_EQUAL(300, *found_value);

  rtl_hash_table_cleanup(&table);
}

// Test hash table collision handling
void test_hash_table_collisions(void)
{
  // Use a small table to force collisions
  rtl_hash_table_t table;
  bool result = rtl_hash_table_init(&table, 2, rtl_hash_fnv1a, rtl_hash_key_compare_bytes);
  TEST_ASSERT_TRUE(result);

  // Insert many entries to ensure collisions
  for (int i = 0; i < 20; i++) {
    int key = i;
    int value = i * 100;
    result = rtl_hash_table_insert(&table, &key, sizeof(key), &value, sizeof(value));
    TEST_ASSERT_TRUE(result);
  }

  TEST_ASSERT_EQUAL(20, rtl_hash_table_size(&table));

  // Verify all entries can still be found
  for (int i = 0; i < 20; i++) {
    int key = i;
    int* found_value = (int*)rtl_hash_table_find(&table, &key, sizeof(key), NULL);
    TEST_ASSERT_NOT_NULL(found_value);
    TEST_ASSERT_EQUAL(i * 100, *found_value);
  }

  // Check load factor
  double load_factor = rtl_hash_table_load_factor(&table);
  TEST_ASSERT_EQUAL(10.0, load_factor);  // 20 entries / 2 buckets = 10.0

  rtl_hash_table_cleanup(&table);
}

// Test hash table load factor
void test_hash_table_load_factor(void)
{
  rtl_hash_table_t table;
  bool result = rtl_hash_table_init(&table, 4, rtl_hash_fnv1a, rtl_hash_key_compare_bytes);
  TEST_ASSERT_TRUE(result);

  TEST_ASSERT_EQUAL(0.0, rtl_hash_table_load_factor(&table));

  // Add 2 entries
  for (int i = 0; i < 2; i++) {
    int key = i;
    int value = i;
    rtl_hash_table_insert(&table, &key, sizeof(key), &value, sizeof(value));
  }

  TEST_ASSERT_EQUAL(0.5, rtl_hash_table_load_factor(&table));  // 2/4 = 0.5

  // Add 2 more entries
  for (int i = 2; i < 4; i++) {
    int key = i;
    int value = i;
    rtl_hash_table_insert(&table, &key, sizeof(key), &value, sizeof(value));
  }

  TEST_ASSERT_EQUAL(1.0, rtl_hash_table_load_factor(&table));  // 4/4 = 1.0

  rtl_hash_table_cleanup(&table);
}

// Test hash table with different value sizes
void test_hash_table_different_value_sizes(void)
{
  rtl_hash_table_t table;
  bool result = rtl_hash_table_init(&table, 5, rtl_hash_fnv1a, rtl_hash_key_compare_bytes);
  TEST_ASSERT_TRUE(result);

  // Test with different sized values
  int key1 = 1;
  char value1 = 'A';
  result = rtl_hash_table_insert(&table, &key1, sizeof(key1), &value1, sizeof(value1));
  TEST_ASSERT_TRUE(result);

  int key2 = 2;
  int value2 = 12345;
  result = rtl_hash_table_insert(&table, &key2, sizeof(key2), &value2, sizeof(value2));
  TEST_ASSERT_TRUE(result);

  int key3 = 3;
  long value3 = 987654321L;  // Using long instead of double to avoid precision issues
  result = rtl_hash_table_insert(&table, &key3, sizeof(key3), &value3, sizeof(value3));
  TEST_ASSERT_TRUE(result);

  // Verify different sized values
  unsigned long found_size;
  char* found_char = (char*)rtl_hash_table_find(&table, &key1, sizeof(key1), &found_size);
  TEST_ASSERT_NOT_NULL(found_char);
  TEST_ASSERT_EQUAL(sizeof(char), found_size);
  TEST_ASSERT_EQUAL('A', *found_char);

  int* found_int = (int*)rtl_hash_table_find(&table, &key2, sizeof(key2), &found_size);
  TEST_ASSERT_NOT_NULL(found_int);
  TEST_ASSERT_EQUAL(sizeof(int), found_size);
  TEST_ASSERT_EQUAL(12345, *found_int);

  long* found_long = (long*)rtl_hash_table_find(&table, &key3, sizeof(key3), &found_size);
  TEST_ASSERT_NOT_NULL(found_long);
  TEST_ASSERT_EQUAL(sizeof(long), found_size);
  TEST_ASSERT_EQUAL(987654321L, *found_long);

  rtl_hash_table_cleanup(&table);
}

// Test FNV-1a hash function
void test_hash_fnv1a_function(void)
{
  const char* test_string = "hello";
  unsigned long hash1 = rtl_hash_fnv1a(test_string, strlen(test_string));
  unsigned long hash2 = rtl_hash_fnv1a(test_string, strlen(test_string));

  // Same input should produce same hash
  TEST_ASSERT_EQUAL(hash1, hash2);

  const char* different_string = "world";
  unsigned long hash3 = rtl_hash_fnv1a(different_string, strlen(different_string));

  // Different input should produce different hash (very likely)
  TEST_ASSERT_NOT_EQUAL(hash1, hash3);
}

// Test key comparison functions
void test_hash_key_compare_functions(void)
{
  // Test byte comparison
  const char* data1 = "test";
  const char* data2 = "test";
  const char* data3 = "different";

  int result = rtl_hash_key_compare_bytes(data1, 4, data2, 4);
  TEST_ASSERT_EQUAL(0, result);

  result = rtl_hash_key_compare_bytes(data1, 4, data3, 9);
  TEST_ASSERT_NOT_EQUAL(0, result);

  // Test string comparison
  result = rtl_hash_key_compare_string(data1, 0, data2, 0);
  TEST_ASSERT_EQUAL(0, result);

  result = rtl_hash_key_compare_string(data1, 0, data3, 0);
  TEST_ASSERT_NOT_EQUAL(0, result);
}

int main(void)
{
  UNITY_BEGIN();

  // Assert tests
  RUN_TEST(test_rtl_assert_success);

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
  RUN_TEST(test_list_length_empty);
  RUN_TEST(test_list_length_single_element);
  RUN_TEST(test_list_length_multiple_elements_head);
  RUN_TEST(test_list_length_multiple_elements_tail);
  RUN_TEST(test_list_length_after_remove);
  RUN_TEST(test_list_length_stress);
  RUN_TEST(test_list_for_each_indexed_empty);
  RUN_TEST(test_list_for_each_indexed_single_element);
  RUN_TEST(test_list_for_each_indexed_multiple_elements);
  RUN_TEST(test_list_for_each_indexed_head_insertion_order);
  RUN_TEST(test_list_for_each_indexed_stress);

  // Hash table tests
  RUN_TEST(test_hash_table_init_cleanup);
  RUN_TEST(test_hash_table_insert);
  RUN_TEST(test_hash_table_find);
  RUN_TEST(test_hash_table_update);
  RUN_TEST(test_hash_table_remove);
  RUN_TEST(test_hash_table_multiple_entries);
  RUN_TEST(test_hash_table_string_keys);
  RUN_TEST(test_hash_table_collisions);
  RUN_TEST(test_hash_table_load_factor);
  RUN_TEST(test_hash_table_different_value_sizes);
  RUN_TEST(test_hash_fnv1a_function);
  RUN_TEST(test_hash_key_compare_functions);

  return UNITY_END();
}
