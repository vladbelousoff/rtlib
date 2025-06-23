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
#include "rtl_memory.h"
#include "rtl_thread.h"

#include "unity.h"

#ifdef _WIN32
#include <windows.h>
typedef HANDLE rtl_thread_t;
typedef DWORD rtl_thread_id_t;
#else
#include <pthread.h>
#include <unistd.h>
typedef pthread_t rtl_thread_t;
typedef pthread_t rtl_thread_id_t;
#endif

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

// Producer-Consumer Queue
typedef struct
{
  rtl_mutex_t mutex;
  rtl_atomic_int_t size;
  rtl_atomic_int_t capacity;
  rtl_atomic_int_t head;
  rtl_atomic_int_t tail;
  int* buffer;
} pc_queue_t;

typedef struct
{
  pc_queue_t* queue;
  int producer_id;
  int items_to_produce;
  rtl_atomic_int_t* total_produced;
} producer_data_t;

typedef struct
{
  pc_queue_t* queue;
  int consumer_id;
  int items_to_consume;
  rtl_atomic_int_t* total_consumed;
} consumer_data_t;

// Reader-Writer Lock simulation
typedef struct
{
  rtl_mutex_t mutex;
  rtl_atomic_int_t readers;
  rtl_atomic_int_t writers_waiting;
  rtl_atomic_int_t writer_active;
} rw_lock_t;

typedef struct
{
  rw_lock_t* lock;
  int thread_id;
  int operations;
  int is_writer;
} rw_thread_data_t;

// Barrier implementation
typedef struct
{
  rtl_mutex_t mutex;
  rtl_atomic_int_t count;
  rtl_atomic_int_t generation;
  int expected_count;
} barrier_t;

// Complex synchronization data
typedef struct
{
  int thread_id;
  rtl_mutex_t* mutexes;
  rtl_atomic_int_t* shared_data;
} complex_sync_data_t;

// Memory consistency data
typedef struct
{
  int thread_id;
  rtl_atomic_int_t* shared_array;
  rtl_atomic_int_t* write_complete;
  rtl_atomic_int_t* read_complete;
} memory_consistency_data_t;

void setUp(void)
{
  rtl_init();
}

void tearDown(void)
{
  rtl_cleanup();
}

// Producer-Consumer Queue functions
static void pc_queue_init(pc_queue_t* queue, int capacity)
{
  rtl_mutex_init(&queue->mutex);
  rtl_atomic_store(&queue->size, 0);
  rtl_atomic_store(&queue->capacity, capacity);
  rtl_atomic_store(&queue->head, 0);
  rtl_atomic_store(&queue->tail, 0);
  queue->buffer = rtl_malloc(capacity * sizeof(int));
  TEST_ASSERT_NOT_NULL(queue->buffer);
}

static void pc_queue_destroy(pc_queue_t* queue)
{
  rtl_mutex_destroy(&queue->mutex);
  rtl_free(queue->buffer);
}

static int pc_queue_enqueue(pc_queue_t* queue, int value)
{
  rtl_mutex_lock(&queue->mutex);

  if (rtl_atomic_load(&queue->size) >= rtl_atomic_load(&queue->capacity)) {
    rtl_mutex_unlock(&queue->mutex);
    return 0;  // Queue full
  }

  int tail = rtl_atomic_load(&queue->tail);
  queue->buffer[tail] = value;
  rtl_atomic_store(&queue->tail, (tail + 1) % rtl_atomic_load(&queue->capacity));
  rtl_atomic_fetch_add(&queue->size, 1);

  rtl_mutex_unlock(&queue->mutex);
  return 1;
}

static int pc_queue_dequeue(pc_queue_t* queue, int* value)
{
  rtl_mutex_lock(&queue->mutex);

  if (rtl_atomic_load(&queue->size) == 0) {
    rtl_mutex_unlock(&queue->mutex);
    return 0;  // Queue empty
  }

  int head = rtl_atomic_load(&queue->head);
  *value = queue->buffer[head];
  rtl_atomic_store(&queue->head, (head + 1) % rtl_atomic_load(&queue->capacity));
  rtl_atomic_fetch_sub(&queue->size, 1);

  rtl_mutex_unlock(&queue->mutex);
  return 1;
}

// Producer thread function
static void* producer_thread(void* arg)
{
  producer_data_t* data = (producer_data_t*)arg;

  for (int i = 0; i < data->items_to_produce; i++) {
    int value = data->producer_id * 1000 + i;

    while (!pc_queue_enqueue(data->queue, value)) {
      rtl_thread_sleep(1);  // Wait if queue is full
    }

    rtl_atomic_fetch_add(data->total_produced, 1);
    rtl_thread_sleep(1);  // Simulate work
  }

  return NULL;
}

// Consumer thread function
static void* consumer_thread(void* arg)
{
  consumer_data_t* data = (consumer_data_t*)arg;

  for (int i = 0; i < data->items_to_consume; i++) {
    int value;

    while (!pc_queue_dequeue(data->queue, &value)) {
      rtl_thread_sleep(1);  // Wait if queue is empty
    }

    // Verify the value is reasonable
    TEST_ASSERT_TRUE(value >= 0);
    rtl_atomic_fetch_add(data->total_consumed, 1);
    rtl_thread_sleep(1);  // Simulate work
  }

  return NULL;
}

// Test Producer-Consumer pattern
void test_producer_consumer_pattern(void)
{
  const int QUEUE_CAPACITY = 10;
  const int NUM_PRODUCERS = 3;
  const int NUM_CONSUMERS = 2;
  const int ITEMS_PER_PRODUCER = 50;
  const int ITEMS_PER_CONSUMER = (NUM_PRODUCERS * ITEMS_PER_PRODUCER) / NUM_CONSUMERS;

  pc_queue_t queue;
  pc_queue_init(&queue, QUEUE_CAPACITY);

  rtl_atomic_int_t total_produced = 0;
  rtl_atomic_int_t total_consumed = 0;

  // Create producer threads
  rtl_thread_t* producer_threads = rtl_malloc(NUM_PRODUCERS * sizeof(rtl_thread_t));
  producer_data_t* producer_data = rtl_malloc(NUM_PRODUCERS * sizeof(producer_data_t));

  for (int i = 0; i < NUM_PRODUCERS; i++) {
    producer_data[i].queue = &queue;
    producer_data[i].producer_id = i;
    producer_data[i].items_to_produce = ITEMS_PER_PRODUCER;
    producer_data[i].total_produced = &total_produced;

    producer_threads[i] = rtl_thread_create(producer_thread, &producer_data[i]);
  }

  // Create consumer threads
  rtl_thread_t* consumer_threads = rtl_malloc(NUM_CONSUMERS * sizeof(rtl_thread_t));
  consumer_data_t* consumer_data = rtl_malloc(NUM_CONSUMERS * sizeof(consumer_data_t));

  for (int i = 0; i < NUM_CONSUMERS; i++) {
    consumer_data[i].queue = &queue;
    consumer_data[i].consumer_id = i;
    consumer_data[i].items_to_consume = ITEMS_PER_CONSUMER;
    consumer_data[i].total_consumed = &total_consumed;

    consumer_threads[i] = rtl_thread_create(consumer_thread, &consumer_data[i]);
  }

  // Wait for all threads to complete
  for (int i = 0; i < NUM_PRODUCERS; i++) {
    rtl_thread_join(producer_threads[i]);
  }

  for (int i = 0; i < NUM_CONSUMERS; i++) {
    rtl_thread_join(consumer_threads[i]);
  }

  // Verify results
  TEST_ASSERT_EQUAL(NUM_PRODUCERS * ITEMS_PER_PRODUCER, rtl_atomic_load(&total_produced));
  TEST_ASSERT_EQUAL(NUM_PRODUCERS * ITEMS_PER_PRODUCER, rtl_atomic_load(&total_consumed));

  pc_queue_destroy(&queue);
  rtl_free(producer_threads);
  rtl_free(producer_data);
  rtl_free(consumer_threads);
  rtl_free(consumer_data);
}

// Reader-Writer Lock functions
static void rw_lock_init(rw_lock_t* lock)
{
  rtl_mutex_init(&lock->mutex);
  rtl_atomic_store(&lock->readers, 0);
  rtl_atomic_store(&lock->writers_waiting, 0);
  rtl_atomic_store(&lock->writer_active, 0);
}

static void rw_lock_destroy(rw_lock_t* lock)
{
  rtl_mutex_destroy(&lock->mutex);
}

static void rw_lock_read_lock(rw_lock_t* lock)
{
  rtl_mutex_lock(&lock->mutex);

  while (rtl_atomic_load(&lock->writer_active) > 0 || rtl_atomic_load(&lock->writers_waiting) > 0) {
    rtl_mutex_unlock(&lock->mutex);
    rtl_thread_sleep(1);
    rtl_mutex_lock(&lock->mutex);
  }

  rtl_atomic_fetch_add(&lock->readers, 1);
  rtl_mutex_unlock(&lock->mutex);
}

static void rw_lock_read_unlock(rw_lock_t* lock)
{
  rtl_atomic_fetch_sub(&lock->readers, 1);
}

static void rw_lock_write_lock(rw_lock_t* lock)
{
  rtl_mutex_lock(&lock->mutex);
  rtl_atomic_fetch_add(&lock->writers_waiting, 1);

  while (rtl_atomic_load(&lock->readers) > 0 || rtl_atomic_load(&lock->writer_active) > 0) {
    rtl_mutex_unlock(&lock->mutex);
    rtl_thread_sleep(1);
    rtl_mutex_lock(&lock->mutex);
  }

  rtl_atomic_fetch_sub(&lock->writers_waiting, 1);
  rtl_atomic_store(&lock->writer_active, 1);
  rtl_mutex_unlock(&lock->mutex);
}

static void rw_lock_write_unlock(rw_lock_t* lock)
{
  rtl_atomic_store(&lock->writer_active, 0);
}

// Reader-Writer thread function
static void* rw_thread(void* arg)
{
  rw_thread_data_t* data = (rw_thread_data_t*)arg;

  for (int i = 0; i < data->operations; i++) {
    if (data->is_writer) {
      rw_lock_write_lock(data->lock);
      rtl_thread_sleep(5);  // Writers take longer
      rw_lock_write_unlock(data->lock);
    } else {
      rw_lock_read_lock(data->lock);
      rtl_thread_sleep(1);  // Readers are faster
      rw_lock_read_unlock(data->lock);
    }
  }

  return NULL;
}

// Test Reader-Writer Lock pattern
void test_reader_writer_pattern(void)
{
  const int NUM_READERS = 8;
  const int NUM_WRITERS = 2;

  rw_lock_t lock;
  rw_lock_init(&lock);

  // Create reader threads
  rtl_thread_t* reader_threads = rtl_malloc(NUM_READERS * sizeof(rtl_thread_t));
  rw_thread_data_t* reader_data = rtl_malloc(NUM_READERS * sizeof(rw_thread_data_t));

  for (int i = 0; i < NUM_READERS; i++) {
    const int READER_OPERATIONS = 20;
    reader_data[i].lock = &lock;
    reader_data[i].thread_id = i;
    reader_data[i].operations = READER_OPERATIONS;
    reader_data[i].is_writer = 0;

    reader_threads[i] = rtl_thread_create(rw_thread, &reader_data[i]);
  }

  // Create writer threads
  rtl_thread_t* writer_threads = rtl_malloc(NUM_WRITERS * sizeof(rtl_thread_t));
  rw_thread_data_t* writer_data = rtl_malloc(NUM_WRITERS * sizeof(rw_thread_data_t));

  for (int i = 0; i < NUM_WRITERS; i++) {
    const int WRITER_OPERATIONS = 5;
    writer_data[i].lock = &lock;
    writer_data[i].thread_id = i + NUM_READERS;
    writer_data[i].operations = WRITER_OPERATIONS;
    writer_data[i].is_writer = 1;

    writer_threads[i] = rtl_thread_create(rw_thread, &writer_data[i]);
  }

  // Wait for all threads to complete
  for (int i = 0; i < NUM_READERS; i++) {
    rtl_thread_join(reader_threads[i]);
  }

  for (int i = 0; i < NUM_WRITERS; i++) {
    rtl_thread_join(writer_threads[i]);
  }

  // Verify final state
  TEST_ASSERT_EQUAL(0, rtl_atomic_load(&lock.readers));
  TEST_ASSERT_EQUAL(0, rtl_atomic_load(&lock.writers_waiting));
  TEST_ASSERT_EQUAL(0, rtl_atomic_load(&lock.writer_active));

  rw_lock_destroy(&lock);
  rtl_free(reader_threads);
  rtl_free(reader_data);
  rtl_free(writer_threads);
  rtl_free(writer_data);
}

// Barrier functions
static void barrier_init(barrier_t* barrier, int expected_count)
{
  rtl_mutex_init(&barrier->mutex);
  rtl_atomic_store(&barrier->count, 0);
  rtl_atomic_store(&barrier->generation, 0);
  barrier->expected_count = expected_count;
}

static void barrier_destroy(barrier_t* barrier)
{
  rtl_mutex_destroy(&barrier->mutex);
}

static void barrier_wait(barrier_t* barrier)
{
  rtl_mutex_lock(&barrier->mutex);

  int generation = rtl_atomic_load(&barrier->generation);
  rtl_atomic_fetch_add(&barrier->count, 1);

  if (rtl_atomic_load(&barrier->count) == barrier->expected_count) {
    rtl_atomic_store(&barrier->count, 0);
    rtl_atomic_fetch_add(&barrier->generation, 1);
  } else {
    while (rtl_atomic_load(&barrier->generation) == generation) {
      rtl_mutex_unlock(&barrier->mutex);
      rtl_thread_sleep(1);
      rtl_mutex_lock(&barrier->mutex);
    }
  }

  rtl_mutex_unlock(&barrier->mutex);
}

// Barrier thread function
static void* barrier_thread(void* arg)
{
  barrier_t* barrier = (barrier_t*)arg;

  // Phase 1: All threads arrive at barrier
  rtl_thread_sleep(rand() % 100);  // Random delay
  barrier_wait(barrier);

  // Phase 2: All threads continue after barrier
  rtl_thread_sleep(rand() % 100);  // Random delay
  barrier_wait(barrier);

  return NULL;
}

// Test Barrier synchronization
void test_barrier_synchronization(void)
{
  const int NUM_THREADS = 8;

  barrier_t barrier;
  barrier_init(&barrier, NUM_THREADS);

  rtl_thread_t* threads = rtl_malloc(NUM_THREADS * sizeof(rtl_thread_t));

  for (int i = 0; i < NUM_THREADS; i++) {
    threads[i] = rtl_thread_create(barrier_thread, &barrier);
  }

  for (int i = 0; i < NUM_THREADS; i++) {
    rtl_thread_join(threads[i]);
  }

  barrier_destroy(&barrier);
  rtl_free(threads);
  TEST_PASS();  // If we get here, barrier worked correctly
}

// Complex thread function
static void* complex_thread(void* arg)
{
  complex_sync_data_t* data = (complex_sync_data_t*)arg;

  for (int iter = 0; iter < 10; iter++) {
    // Phase 1: All threads increment shared data
    rtl_atomic_fetch_add(data->shared_data, 1);

    // Wait for all threads to reach phase 1
    while (rtl_atomic_load(data->shared_data) < 6 * (iter + 1)) {
      rtl_thread_sleep(1);
    }

    // Phase 2: Complex mutex pattern
    if (data->thread_id % 2 == 0) {
      rtl_mutex_lock(&data->mutexes[0]);
      rtl_mutex_lock(&data->mutexes[1]);
      rtl_thread_sleep(5);
      rtl_mutex_unlock(&data->mutexes[1]);
      rtl_mutex_unlock(&data->mutexes[0]);
    } else {
      rtl_mutex_lock(&data->mutexes[1]);
      rtl_mutex_lock(&data->mutexes[2]);
      rtl_thread_sleep(5);
      rtl_mutex_unlock(&data->mutexes[2]);
      rtl_mutex_unlock(&data->mutexes[1]);
    }

    // Phase 3: Atomic compare-exchange
    int expected = rtl_atomic_load(data->shared_data);
    rtl_atomic_compare_exchange_bool(data->shared_data, expected, expected + data->thread_id);
  }

  return NULL;
}

// Test complex synchronization patterns
void test_complex_synchronization_patterns(void)
{
  const int NUM_THREADS = 6;
  const int ITERATIONS = 10;

  rtl_mutex_t mutexes[3];
  rtl_atomic_int_t shared_data = 0;

  for (int i = 0; i < 3; i++) {
    rtl_mutex_init(&mutexes[i]);
  }

  rtl_thread_t* threads = rtl_malloc(NUM_THREADS * sizeof(rtl_thread_t));
  complex_sync_data_t* thread_data = rtl_malloc(NUM_THREADS * sizeof(complex_sync_data_t));

  for (int i = 0; i < NUM_THREADS; i++) {
    thread_data[i].thread_id = i;
    thread_data[i].mutexes = mutexes;
    thread_data[i].shared_data = &shared_data;
    threads[i] = rtl_thread_create(complex_thread, &thread_data[i]);
  }

  for (int i = 0; i < NUM_THREADS; i++) {
    rtl_thread_join(threads[i]);
  }

  // Verify final state
  TEST_ASSERT_TRUE(rtl_atomic_load(&shared_data) >= NUM_THREADS * ITERATIONS);

  for (int i = 0; i < 3; i++) {
    rtl_mutex_destroy(&mutexes[i]);
  }

  rtl_free(threads);
  rtl_free(thread_data);
}

// Writer thread function
static void* writer_thread(void* arg)
{
  rtl_atomic_int_t* shared_array = (rtl_atomic_int_t*)arg;
  rtl_atomic_int_t* write_complete = shared_array + 100;

  for (int i = 0; i < 100; i++) {
    rtl_atomic_store(&shared_array[i], i + 1);
    rtl_thread_sleep(1);
  }
  rtl_atomic_store(write_complete, 1);
  return NULL;
}

// Reader thread function
static void* reader_thread(void* arg)
{
  memory_consistency_data_t* data = (memory_consistency_data_t*)arg;

  // Wait for writer to start
  while (rtl_atomic_load(data->write_complete) == 0) {
    rtl_thread_sleep(1);
  }

  // Read array and verify consistency
  int last_value = 0;
  for (int i = 0; i < 100; i++) {
    int current_value = rtl_atomic_load(&data->shared_array[i]);

    // Values should be monotonically increasing or zero
    TEST_ASSERT_TRUE(current_value >= 0);
    TEST_ASSERT_TRUE(current_value <= 100);

    if (current_value > 0) {
      TEST_ASSERT_TRUE(current_value >= last_value);
      last_value = current_value;
    }
  }

  rtl_atomic_fetch_add(data->read_complete, 1);
  return NULL;
}

// Test memory consistency across threads
void test_memory_consistency(void)
{
  const int NUM_THREADS = 4;
  const int ARRAY_SIZE = 100;

  rtl_atomic_int_t* shared_array = rtl_malloc((ARRAY_SIZE + 2) * sizeof(rtl_atomic_int_t));
  rtl_atomic_int_t* write_complete = &shared_array[ARRAY_SIZE];
  rtl_atomic_int_t* read_complete = &shared_array[ARRAY_SIZE + 1];

  // Initialize array
  for (int i = 0; i < ARRAY_SIZE; i++) {
    rtl_atomic_store(&shared_array[i], 0);
  }
  rtl_atomic_store(write_complete, 0);
  rtl_atomic_store(read_complete, 0);

  rtl_thread_t writer = rtl_thread_create(writer_thread, shared_array);
  rtl_thread_t* readers = rtl_malloc(NUM_THREADS * sizeof(rtl_thread_t));
  memory_consistency_data_t* thread_data =
    rtl_malloc(NUM_THREADS * sizeof(memory_consistency_data_t));

  for (int i = 0; i < NUM_THREADS; i++) {
    thread_data[i].thread_id = i;
    thread_data[i].shared_array = shared_array;
    thread_data[i].write_complete = write_complete;
    thread_data[i].read_complete = read_complete;
    readers[i] = rtl_thread_create(reader_thread, &thread_data[i]);
  }

  rtl_thread_join(writer);
  for (int i = 0; i < NUM_THREADS; i++) {
    rtl_thread_join(readers[i]);
  }

  TEST_ASSERT_EQUAL(NUM_THREADS, rtl_atomic_load(read_complete));

  rtl_free(shared_array);
  rtl_free(readers);
  rtl_free(thread_data);
}

int main(void)
{
  UNITY_BEGIN();

  RUN_TEST(test_producer_consumer_pattern);
  RUN_TEST(test_reader_writer_pattern);
  RUN_TEST(test_barrier_synchronization);
  RUN_TEST(test_complex_synchronization_patterns);
  RUN_TEST(test_memory_consistency);

  return UNITY_END();
}
