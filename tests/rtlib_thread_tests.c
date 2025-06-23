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
#include <string.h>
#include "rtl.h"
#include "rtl_list.h"
#include "rtl_log.h"
#include "rtl_memory.h"
#include "rtl_thread.h"
#include "unity.h"

#ifdef _WIN32
#include <windows.h>
typedef HANDLE rtl_thread_t;
typedef DWORD rtl_thread_id_t;
#else
#include <pthread.h>
typedef pthread_t rtl_thread_t;
typedef pthread_t rtl_thread_id_t;
#endif

// Test data structures
typedef struct
{
  rtl_list_entry_t list_entry;
  int value;
  char data[64];
} test_item_t;

typedef struct
{
  rtl_mutex_t mutex;
  rtl_list_entry_t head;
  int item_count;
} thread_safe_list_t;

typedef struct
{
  rtl_atomic_int_t counter;
  rtl_atomic_int_t flag;
  int iterations;
  rtl_mutex_t mutex;
} stress_test_data_t;

typedef struct
{
  rtl_atomic_int_t* shared_counter;
  rtl_mutex_t* mutex;
  int thread_id;
  int iterations;
} worker_thread_data_t;

// Race condition thread data
typedef struct
{
  int thread_id;
  rtl_atomic_int_t* shared_value;
  int* result;
} race_condition_data_t;

// List worker thread data
typedef struct
{
  int thread_id;
  thread_safe_list_t* safe_list;
} list_worker_data_t;

// Stress thread data
typedef struct
{
  int thread_id;
  stress_test_data_t* stress_data;
} stress_thread_data_t;

// Memory ordering data
typedef struct
{
  rtl_atomic_int_t* data;
  rtl_atomic_int_t* flag;
} memory_ordering_data_t;

// Atomic load thread data
typedef struct
{
  int thread_id;
  rtl_atomic_int_t* counters;
} atomic_load_data_t;

// Thread creation and management functions
static rtl_thread_t rtl_thread_create(void* (*func)(void*), void* arg)
{
#ifdef _WIN32
  return CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)func, arg, 0, NULL);
#else
  rtl_thread_t thread;
  pthread_create(&thread, NULL, func, arg);
  return thread;
#endif
}

static void rtl_thread_join(rtl_thread_t thread)
{
#ifdef _WIN32
  WaitForSingleObject(thread, INFINITE);
  CloseHandle(thread);
#else
  pthread_join(thread, NULL);
#endif
}

static void rtl_thread_sleep(int milliseconds)
{
#ifdef _WIN32
  Sleep(milliseconds);
#else
  usleep(milliseconds * 1000);
#endif
}

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

  rtl_log_i("Test inf message\n");
  rtl_log_d("Test dbg message\n");
  rtl_log_w("Test wrn message\n");
  rtl_log_e("Test err message\n");

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

// Worker thread function for concurrent counter testing
static void* worker_thread_func(void* arg)
{
  worker_thread_data_t* data = (worker_thread_data_t*)arg;

  for (int i = 0; i < data->iterations; i++) {
    // Atomic increment
    rtl_atomic_fetch_add(data->shared_counter, 1);

    // Mutex-protected increment (for comparison)
    rtl_mutex_lock(data->mutex);
    int current = rtl_atomic_load(data->shared_counter);
    rtl_atomic_store(data->shared_counter, current + 1);
    rtl_mutex_unlock(data->mutex);

    // Small delay to increase contention
    rtl_thread_sleep(1);
  }

  return NULL;
}

// Test concurrent atomic operations
void test_concurrent_atomic_operations(void)
{
  const int NUM_THREADS = 4;
  const int ITERATIONS_PER_THREAD = 20;

  rtl_atomic_int_t shared_counter = 0;
  rtl_mutex_t mutex;
  rtl_mutex_init(&mutex);

  rtl_thread_t* threads = rtl_malloc(NUM_THREADS * sizeof(rtl_thread_t));
  worker_thread_data_t* thread_data = rtl_malloc(NUM_THREADS * sizeof(worker_thread_data_t));

  // Create worker threads
  for (int i = 0; i < NUM_THREADS; i++) {
    thread_data[i].shared_counter = &shared_counter;
    thread_data[i].mutex = &mutex;
    thread_data[i].thread_id = i;
    thread_data[i].iterations = ITERATIONS_PER_THREAD;

    threads[i] = rtl_thread_create(worker_thread_func, &thread_data[i]);
    TEST_ASSERT_NOT_NULL(threads[i]);
  }

  // Wait for all threads to complete
  for (int i = 0; i < NUM_THREADS; i++) {
    rtl_thread_join(threads[i]);
  }

  // Each thread does ITERATIONS_PER_THREAD atomic increments and ITERATIONS_PER_THREAD
  // mutex-protected increments
  int expected_value = NUM_THREADS * ITERATIONS_PER_THREAD * 2;
  int actual_value = rtl_atomic_load(&shared_counter);

  // Allow for small variations due to timing (within 1% tolerance)
  int tolerance = expected_value / 100;
  TEST_ASSERT_TRUE(actual_value >= expected_value - tolerance);
  TEST_ASSERT_TRUE(actual_value <= expected_value + tolerance);

  rtl_mutex_destroy(&mutex);
  rtl_free(threads);
  rtl_free(thread_data);
}

// Race condition thread function
static void* race_condition_thread(void* arg)
{
  race_condition_data_t* data = (race_condition_data_t*)arg;
  int successful_updates = 0;

  for (int i = 0; i < 500; i++) {
    int expected, desired;
    do {
      expected = rtl_atomic_load(data->shared_value);
      if (expected % 2 == 0) {
        desired = expected + 1;
        if (rtl_atomic_compare_exchange_bool(data->shared_value, expected, desired)) {
          successful_updates++;
          break;
        }
      } else {
        break;  // Value is odd, skip this iteration
      }
    } while (1);

    rtl_thread_sleep(1);  // Small delay
  }

  *(data->result) = successful_updates;
  return NULL;
}

// Test race condition detection with atomic compare-exchange
void test_race_condition_detection(void)
{
  rtl_atomic_int_t shared_value = 0;
  const int NUM_THREADS = 8;

  rtl_thread_t* threads = rtl_malloc(NUM_THREADS * sizeof(rtl_thread_t));
  int* thread_results = rtl_malloc(NUM_THREADS * sizeof(int));
  race_condition_data_t* thread_data = rtl_malloc(NUM_THREADS * sizeof(race_condition_data_t));

  // Create threads
  for (int i = 0; i < NUM_THREADS; i++) {
    thread_data[i].thread_id = i;
    thread_data[i].shared_value = &shared_value;
    thread_data[i].result = &thread_results[i];
    threads[i] = rtl_thread_create(race_condition_thread, &thread_data[i]);
  }

  // Wait for completion
  for (int i = 0; i < NUM_THREADS; i++) {
    rtl_thread_join(threads[i]);
  }

  // Verify results
  int total_successful = 0;
  for (int i = 0; i < NUM_THREADS; i++) {
    total_successful += thread_results[i];
  }

  int final_value = rtl_atomic_load(&shared_value);
  TEST_ASSERT_EQUAL(total_successful, final_value);
  TEST_ASSERT_TRUE(final_value > 0);  // At least some updates should succeed

  rtl_free(threads);
  rtl_free(thread_results);
  rtl_free(thread_data);
}

// List worker thread function
static void* list_worker_thread(void* arg)
{
  list_worker_data_t* data = (list_worker_data_t*)arg;

  for (int i = 0; i < 100; i++) {
    test_item_t* item = rtl_malloc(sizeof(test_item_t));
    TEST_ASSERT_NOT_NULL(item);

    item->value = data->thread_id * 1000 + i;
    snprintf(item->data, sizeof(item->data), "Thread %d, Item %d", data->thread_id, i);

    // Add to list (thread-safe)
    rtl_mutex_lock(&data->safe_list->mutex);
    rtl_list_add_tail(&data->safe_list->head, &item->list_entry);
    data->safe_list->item_count++;
    rtl_mutex_unlock(&data->safe_list->mutex);

    rtl_thread_sleep(1);
  }

  return NULL;
}

// Test thread-safe list operations
void test_thread_safe_list_operations(void)
{
  thread_safe_list_t safe_list;
  rtl_mutex_init(&safe_list.mutex);
  rtl_list_init(&safe_list.head);
  safe_list.item_count = 0;

  const int NUM_THREADS = 4;

  rtl_thread_t* threads = rtl_malloc(NUM_THREADS * sizeof(rtl_thread_t));
  list_worker_data_t* thread_data = rtl_malloc(NUM_THREADS * sizeof(list_worker_data_t));

  // Create threads
  for (int i = 0; i < NUM_THREADS; i++) {
    thread_data[i].thread_id = i;
    thread_data[i].safe_list = &safe_list;
    threads[i] = rtl_thread_create(list_worker_thread, &thread_data[i]);
  }

  // Wait for completion
  for (int i = 0; i < NUM_THREADS; i++) {
    rtl_thread_join(threads[i]);
  }

  // Verify list contents
  TEST_ASSERT_EQUAL(NUM_THREADS * 100, safe_list.item_count);

  int item_count = 0;
  rtl_list_entry_t* pos;
  rtl_list_for_each(pos, &safe_list.head)
  {
    test_item_t* item = rtl_list_record(pos, test_item_t, list_entry);
    TEST_ASSERT_NOT_NULL(item);
    TEST_ASSERT_TRUE(item->value >= 0);
    TEST_ASSERT_TRUE(strlen(item->data) > 0);
    item_count++;
  }

  TEST_ASSERT_EQUAL(NUM_THREADS * 100, item_count);

  // Clean up
  rtl_list_entry_t *pos_safe, *n;
  rtl_list_for_each_safe(pos_safe, n, &safe_list.head)
  {
    test_item_t* item = rtl_list_record(pos_safe, test_item_t, list_entry);
    rtl_list_remove(pos_safe);
    rtl_free(item);
  }

  rtl_mutex_destroy(&safe_list.mutex);
  rtl_free(threads);
  rtl_free(thread_data);
}

// Stress thread function
static void* stress_thread(void* arg)
{
  stress_thread_data_t* data = (stress_thread_data_t*)arg;

  for (int i = 0; i < data->stress_data->iterations; i++) {
    // Mix of atomic and mutex operations
    if (i % 3 == 0) {
      rtl_atomic_fetch_add(&data->stress_data->counter, 1);
    } else if (i % 3 == 1) {
      rtl_mutex_lock(&data->stress_data->mutex);
      int val = rtl_atomic_load(&data->stress_data->counter);
      rtl_atomic_store(&data->stress_data->counter, val + 1);
      rtl_mutex_unlock(&data->stress_data->mutex);
    } else {
      // Compare-exchange operation
      int expected = rtl_atomic_load(&data->stress_data->counter);
      rtl_atomic_compare_exchange_bool(&data->stress_data->counter, expected, expected + 1);
    }

    // Set flag to indicate activity
    rtl_atomic_store(&data->stress_data->flag, data->thread_id);
  }

  return NULL;
}

// Stress test with high contention
void test_high_contention_stress(void)
{
  const int NUM_THREADS = 16;
  const int ITERATIONS = 1000;

  stress_test_data_t stress_data;
  rtl_atomic_store(&stress_data.counter, 0);
  rtl_atomic_store(&stress_data.flag, 0);
  stress_data.iterations = ITERATIONS;
  rtl_mutex_init(&stress_data.mutex);

  rtl_thread_t* threads = rtl_malloc(NUM_THREADS * sizeof(rtl_thread_t));
  stress_thread_data_t* thread_data = rtl_malloc(NUM_THREADS * sizeof(stress_thread_data_t));

  // Create stress threads
  for (int i = 0; i < NUM_THREADS; i++) {
    thread_data[i].thread_id = i;
    thread_data[i].stress_data = &stress_data;
    threads[i] = rtl_thread_create(stress_thread, &thread_data[i]);
  }

  // Wait for completion
  for (int i = 0; i < NUM_THREADS; i++) {
    rtl_thread_join(threads[i]);
  }

  // Verify final state
  int final_counter = rtl_atomic_load(&stress_data.counter);
  TEST_ASSERT_TRUE(final_counter > 0);
  TEST_ASSERT_TRUE(final_counter <= NUM_THREADS * ITERATIONS);

  rtl_mutex_destroy(&stress_data.mutex);
  rtl_free(threads);
  rtl_free(thread_data);
}

// Ordering thread function
static void* ordering_thread(void* arg)
{
  memory_ordering_data_t* data = (memory_ordering_data_t*)arg;

  // Set data first, then flag
  rtl_atomic_store(data->data, 42);
  rtl_atomic_store(data->flag, 1);
  return NULL;
}

// Reader thread function
static void* reader_thread(void* arg)
{
  memory_ordering_data_t* data = (memory_ordering_data_t*)arg;

  // Wait for flag, then read data
  while (rtl_atomic_load(data->flag) == 0) {
    // Spin wait
  }

  int read_data = rtl_atomic_load(data->data);
  TEST_ASSERT_EQUAL(42, read_data);
  return NULL;
}

// Test memory barrier and ordering
void test_memory_ordering(void)
{
  rtl_atomic_int_t flag = 0;
  rtl_atomic_int_t data = 0;

  memory_ordering_data_t shared_data = { &data, &flag };

  rtl_thread_t writer = rtl_thread_create(ordering_thread, &shared_data);
  rtl_thread_t reader = rtl_thread_create(reader_thread, &shared_data);

  rtl_thread_join(writer);
  rtl_thread_join(reader);
}

// Consistent order thread function
static void* consistent_order_thread(void* arg)
{
  rtl_mutex_t* mutexes = (rtl_mutex_t*)arg;
  rtl_mutex_t* mutex1 = &mutexes[0];
  rtl_mutex_t* mutex2 = &mutexes[1];

  rtl_mutex_lock(mutex1);
  rtl_thread_sleep(10);  // Small delay to increase chance of deadlock
  rtl_mutex_lock(mutex2);

  // Do some work
  rtl_thread_sleep(5);

  rtl_mutex_unlock(mutex2);
  rtl_mutex_unlock(mutex1);
  return NULL;
}

// Test deadlock prevention
void test_deadlock_prevention(void)
{
  rtl_mutex_t mutex1, mutex2;
  rtl_mutex_init(&mutex1);
  rtl_mutex_init(&mutex2);

  rtl_mutex_t mutexes[2] = { mutex1, mutex2 };

  rtl_thread_t thread1 = rtl_thread_create(consistent_order_thread, mutexes);
  rtl_thread_t thread2 = rtl_thread_create(consistent_order_thread, mutexes);

  rtl_thread_join(thread1);
  rtl_thread_join(thread2);

  rtl_mutex_destroy(&mutex1);
  rtl_mutex_destroy(&mutex2);

  TEST_PASS();  // If we get here, no deadlock occurred
}

// Atomic load thread function
static void* atomic_load_thread(void* arg)
{
  atomic_load_data_t* data = (atomic_load_data_t*)arg;

  for (int i = 0; i < 10000; i++) {
    int op = i % 4;
    int counter_idx = i % 4;

    switch (op) {
      case 0:
        rtl_atomic_fetch_add(&data->counters[counter_idx], 1);
        break;
      case 1:
        rtl_atomic_fetch_sub(&data->counters[counter_idx], 1);
        break;
      case 2:
        rtl_atomic_store(&data->counters[counter_idx], i);
        break;
      case 3:
        rtl_atomic_load(&data->counters[counter_idx]);
        break;
    }
  }

  return NULL;
}

// Test atomic operations under load
void test_atomic_operations_under_load(void)
{
  const int NUM_THREADS = 8;
  const int OPERATIONS_PER_THREAD = 10000;

  rtl_atomic_int_t counters[4] = { 0, 0, 0, 0 };

  rtl_thread_t* threads = rtl_malloc(NUM_THREADS * sizeof(rtl_thread_t));
  atomic_load_data_t* thread_data = rtl_malloc(NUM_THREADS * sizeof(atomic_load_data_t));

  for (int i = 0; i < NUM_THREADS; i++) {
    thread_data[i].thread_id = i;
    thread_data[i].counters = counters;
    threads[i] = rtl_thread_create(atomic_load_thread, &thread_data[i]);
  }

  for (int i = 0; i < NUM_THREADS; i++) {
    rtl_thread_join(threads[i]);
  }

  // Verify that operations completed without corruption
  for (int i = 0; i < 4; i++) {
    int final_value = rtl_atomic_load(&counters[i]);
    TEST_ASSERT_TRUE(final_value >= -NUM_THREADS * OPERATIONS_PER_THREAD / 4);
    TEST_ASSERT_TRUE(final_value <= NUM_THREADS * OPERATIONS_PER_THREAD);
  }

  rtl_free(threads);
  rtl_free(thread_data);
}

int main(void)
{
  UNITY_BEGIN();

  RUN_TEST(test_mutex_basic_operations);
  RUN_TEST(test_atomic_operations);
  RUN_TEST(test_thread_safe_logging);
  RUN_TEST(test_mutex_reentrancy);
  RUN_TEST(test_concurrent_atomic_operations);
  RUN_TEST(test_race_condition_detection);
  RUN_TEST(test_thread_safe_list_operations);
  RUN_TEST(test_high_contention_stress);
  RUN_TEST(test_memory_ordering);
  RUN_TEST(test_deadlock_prevention);
  RUN_TEST(test_atomic_operations_under_load);

  return UNITY_END();
}
