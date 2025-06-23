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

#include "rtl_thread.h"
#include "rtl_memory.h"

// Thread management functions
rtl_thread_t rtl_thread_create(void* (*func)(void*), void* arg)
{
#ifdef _WIN32
  return CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)func, arg, 0, NULL);
#else
  rtl_thread_t thread;
  pthread_create(&thread, NULL, func, arg);
  return thread;
#endif
}

void rtl_thread_join(rtl_thread_t thread)
{
#ifdef _WIN32
  WaitForSingleObject(thread, INFINITE);
  CloseHandle(thread);
#else
  pthread_join(thread, NULL);
#endif
}

void rtl_thread_sleep(int milliseconds)
{
#ifdef _WIN32
  Sleep(milliseconds);
#else
  usleep(milliseconds * 1000);
#endif
}

rtl_thread_id_t rtl_thread_get_id(void)
{
#ifdef _WIN32
  return GetCurrentThreadId();
#else
  return pthread_self();
#endif
}

// Producer-Consumer Queue functions
void rtl_pc_queue_init(rtl_pc_queue_t* queue, int capacity)
{
  rtl_mutex_init(&queue->mutex);
  rtl_atomic_store(&queue->size, 0);
  rtl_atomic_store(&queue->capacity, capacity);
  rtl_atomic_store(&queue->head, 0);
  rtl_atomic_store(&queue->tail, 0);
  queue->buffer = rtl_malloc(capacity * sizeof(void*));
}

void rtl_pc_queue_destroy(rtl_pc_queue_t* queue)
{
  rtl_mutex_destroy(&queue->mutex);
  rtl_free(queue->buffer);
}

int rtl_pc_queue_enqueue(rtl_pc_queue_t* queue, void* value)
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

int rtl_pc_queue_dequeue(rtl_pc_queue_t* queue, void** value)
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

int rtl_pc_queue_is_empty(rtl_pc_queue_t* queue)
{
  return rtl_atomic_load(&queue->size) == 0;
}

int rtl_pc_queue_is_full(rtl_pc_queue_t* queue)
{
  return rtl_atomic_load(&queue->size) >= rtl_atomic_load(&queue->capacity);
}

int rtl_pc_queue_size(rtl_pc_queue_t* queue)
{
  return rtl_atomic_load(&queue->size);
}

// Reader-Writer Lock functions
void rtl_rw_lock_init(rtl_rw_lock_t* lock)
{
  rtl_mutex_init(&lock->mutex);
  rtl_atomic_store(&lock->readers, 0);
  rtl_atomic_store(&lock->writers_waiting, 0);
  rtl_atomic_store(&lock->writer_active, 0);
}

void rtl_rw_lock_destroy(rtl_rw_lock_t* lock)
{
  rtl_mutex_destroy(&lock->mutex);
}

void rtl_rw_lock_read_lock(rtl_rw_lock_t* lock)
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

void rtl_rw_lock_read_unlock(rtl_rw_lock_t* lock)
{
  rtl_atomic_fetch_sub(&lock->readers, 1);
}

void rtl_rw_lock_write_lock(rtl_rw_lock_t* lock)
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

void rtl_rw_lock_write_unlock(rtl_rw_lock_t* lock)
{
  rtl_atomic_store(&lock->writer_active, 0);
}

// Barrier functions
void rtl_barrier_init(rtl_barrier_t* barrier, int expected_count)
{
  rtl_mutex_init(&barrier->mutex);
  rtl_atomic_store(&barrier->count, 0);
  rtl_atomic_store(&barrier->generation, 0);
  barrier->expected_count = expected_count;
}

void rtl_barrier_destroy(rtl_barrier_t* barrier)
{
  rtl_mutex_destroy(&barrier->mutex);
}

void rtl_barrier_wait(rtl_barrier_t* barrier)
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