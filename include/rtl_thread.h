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

#ifdef _WIN32
#include <windows.h>
// have to go after windows.h
#include <synchapi.h>
#include <intrin.h>  // For atomic operations
#else
#include <pthread.h>
#include <unistd.h>
#endif

// Thread types
#ifdef _WIN32
typedef HANDLE rtl_thread_t;
typedef DWORD rtl_thread_id_t;
#else
typedef pthread_t rtl_thread_t;
typedef pthread_t rtl_thread_id_t;
#endif

// Mutex type
#ifdef _WIN32
typedef CRITICAL_SECTION rtl_mutex_t;
#else
typedef pthread_mutex_t rtl_mutex_t;
#endif

// Atomic integer type
#ifdef _WIN32
typedef LONG rtl_atomic_int_t;
#else
typedef int rtl_atomic_int_t;
#endif

// Thread management functions
rtl_thread_t rtl_thread_create(void* (*func)(void*), void* arg);
void rtl_thread_join(rtl_thread_t thread);
void rtl_thread_sleep(int milliseconds);
rtl_thread_id_t rtl_thread_get_id(void);

// Mutex functions
static inline void rtl_mutex_init(rtl_mutex_t* mutex)
{
#ifdef _WIN32
  InitializeCriticalSection(mutex);
#else
  pthread_mutex_init(mutex, NULL);
#endif
}

static inline void rtl_mutex_destroy(rtl_mutex_t* mutex)
{
#ifdef _WIN32
  DeleteCriticalSection(mutex);
#else
  pthread_mutex_destroy(mutex);
#endif
}

static inline void rtl_mutex_lock(rtl_mutex_t* mutex)
{
#ifdef _WIN32
  EnterCriticalSection(mutex);
#else
  pthread_mutex_lock(mutex);
#endif
}

static inline void rtl_mutex_unlock(rtl_mutex_t* mutex)
{
#ifdef _WIN32
  LeaveCriticalSection(mutex);
#else
  pthread_mutex_unlock(mutex);
#endif
}

// Atomic functions
static inline rtl_atomic_int_t rtl_atomic_load(rtl_atomic_int_t* ptr)
{
#ifdef _WIN32
  return InterlockedCompareExchange(ptr, 0, 0);
#else
  return __sync_val_compare_and_swap(ptr, 0, 0);
#endif
}

static inline void rtl_atomic_store(rtl_atomic_int_t* ptr, rtl_atomic_int_t value)
{
#ifdef _WIN32
  InterlockedExchange(ptr, value);
#else
  __sync_synchronize();
  *ptr = value;
  __sync_synchronize();
#endif
}

static inline rtl_atomic_int_t rtl_atomic_fetch_add(rtl_atomic_int_t* ptr, rtl_atomic_int_t value)
{
#ifdef _WIN32
  return InterlockedExchangeAdd(ptr, value);
#else
  return __sync_fetch_and_add(ptr, value);
#endif
}

static inline rtl_atomic_int_t rtl_atomic_fetch_sub(rtl_atomic_int_t* ptr, rtl_atomic_int_t value)
{
#ifdef _WIN32
  return InterlockedExchangeAdd(ptr, -value);
#else
  return __sync_fetch_and_sub(ptr, value);
#endif
}

static inline rtl_atomic_int_t rtl_atomic_compare_exchange(
  rtl_atomic_int_t* ptr, rtl_atomic_int_t expected, rtl_atomic_int_t desired)
{
#ifdef _WIN32
  return InterlockedCompareExchange(ptr, desired, expected);
#else
  return __sync_val_compare_and_swap(ptr, expected, desired);
#endif
}

static inline int rtl_atomic_compare_exchange_bool(
  rtl_atomic_int_t* ptr, rtl_atomic_int_t expected, rtl_atomic_int_t desired)
{
#ifdef _WIN32
  return InterlockedCompareExchange(ptr, desired, expected) == expected;
#else
  return __sync_bool_compare_and_swap(ptr, expected, desired);
#endif
}

// Producer-Consumer Queue
typedef struct rtl_pc_queue
{
  rtl_mutex_t mutex;
  rtl_atomic_int_t size;
  rtl_atomic_int_t capacity;
  rtl_atomic_int_t head;
  rtl_atomic_int_t tail;
  void** buffer;
} rtl_pc_queue_t;

void rtl_pc_queue_init(rtl_pc_queue_t* queue, int capacity);
void rtl_pc_queue_destroy(rtl_pc_queue_t* queue);
int rtl_pc_queue_enqueue(rtl_pc_queue_t* queue, void* value);
int rtl_pc_queue_dequeue(rtl_pc_queue_t* queue, void** value);
int rtl_pc_queue_is_empty(rtl_pc_queue_t* queue);
int rtl_pc_queue_is_full(rtl_pc_queue_t* queue);
int rtl_pc_queue_size(rtl_pc_queue_t* queue);

// Reader-Writer Lock
typedef struct rtl_rw_lock
{
  rtl_mutex_t mutex;
  rtl_atomic_int_t readers;
  rtl_atomic_int_t writers_waiting;
  rtl_atomic_int_t writer_active;
} rtl_rw_lock_t;

void rtl_rw_lock_init(rtl_rw_lock_t* lock);
void rtl_rw_lock_destroy(rtl_rw_lock_t* lock);
void rtl_rw_lock_read_lock(rtl_rw_lock_t* lock);
void rtl_rw_lock_read_unlock(rtl_rw_lock_t* lock);
void rtl_rw_lock_write_lock(rtl_rw_lock_t* lock);
void rtl_rw_lock_write_unlock(rtl_rw_lock_t* lock);

// Barrier synchronization
typedef struct rtl_barrier
{
  rtl_mutex_t mutex;
  rtl_atomic_int_t count;
  rtl_atomic_int_t generation;
  int expected_count;
} rtl_barrier_t;

void rtl_barrier_init(rtl_barrier_t* barrier, int expected_count);
void rtl_barrier_destroy(rtl_barrier_t* barrier);
void rtl_barrier_wait(rtl_barrier_t* barrier);

// Lock-free Queue (Single Producer, Single Consumer)
typedef struct rtl_lockfree_queue
{
  rtl_atomic_int_t head;
  rtl_atomic_int_t tail;
  rtl_atomic_int_t size;
  int capacity;
  void** buffer;
} rtl_lockfree_queue_t;

void rtl_lockfree_queue_init(rtl_lockfree_queue_t* queue, int capacity);
void rtl_lockfree_queue_destroy(rtl_lockfree_queue_t* queue);
int rtl_lockfree_queue_enqueue(rtl_lockfree_queue_t* queue, void* value);
int rtl_lockfree_queue_dequeue(rtl_lockfree_queue_t* queue, void** value);
int rtl_lockfree_queue_is_empty(rtl_lockfree_queue_t* queue);
int rtl_lockfree_queue_is_full(rtl_lockfree_queue_t* queue);
int rtl_lockfree_queue_size(rtl_lockfree_queue_t* queue);

// Lock-free Queue (Multiple Producer, Multiple Consumer)
typedef struct rtl_lockfree_mpmc_queue
{
  rtl_atomic_int_t head;
  rtl_atomic_int_t tail;
  rtl_atomic_int_t size;
  int capacity;
  void** buffer;
} rtl_lockfree_mpmc_queue_t;

void rtl_lockfree_mpmc_queue_init(rtl_lockfree_mpmc_queue_t* queue, int capacity);
void rtl_lockfree_mpmc_queue_destroy(rtl_lockfree_mpmc_queue_t* queue);
int rtl_lockfree_mpmc_queue_enqueue(rtl_lockfree_mpmc_queue_t* queue, void* value);
int rtl_lockfree_mpmc_queue_dequeue(rtl_lockfree_mpmc_queue_t* queue, void** value);
int rtl_lockfree_mpmc_queue_is_empty(rtl_lockfree_mpmc_queue_t* queue);
int rtl_lockfree_mpmc_queue_is_full(rtl_lockfree_mpmc_queue_t* queue);
int rtl_lockfree_mpmc_queue_size(rtl_lockfree_mpmc_queue_t* queue);
