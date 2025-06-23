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
#else
#include <pthread.h>
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
  return __sync_fetch_and_add(ptr, 0);
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
