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

#include "rtl_hash.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "rtl.h"
#include "rtl_log.h"
#include "rtl_memory.h"

/**
 * @internal
 * @brief Finds the hash entry for a given key in a specific bucket.
 * @param bucket Pointer to the bucket's list head.
 * @param key Pointer to the key to search for.
 * @param key_size Size of the key in bytes.
 * @param key_compare Key comparison function.
 * @return Pointer to the hash entry if found, NULL otherwise.
 */
static rtl_hash_entry_t* _rtl_hash_find_entry(const rtl_list_entry_t* bucket, const void* key,
  unsigned long key_size, rtl_hash_key_compare_t key_compare)
{
  rtl_list_entry_t* current;
  rtl_list_for_each(current, bucket)
  {
    rtl_hash_entry_t* entry = rtl_list_record(current, rtl_hash_entry_t, list_entry);
    if (key_compare(entry->key, entry->key_size, key, key_size) == 0) {
      return entry;
    }
  }
  return NULL;
}

/**
 * @internal
 * @brief Creates and initializes a new hash entry.
 * @param key Pointer to the key data.
 * @param key_size Size of the key in bytes.
 * @param value Pointer to the value data.
 * @param value_size Size of the value in bytes.
 * @return Pointer to the new hash entry, or NULL on failure.
 */
static rtl_hash_entry_t* _rtl_hash_create_entry(
  const void* key, unsigned long key_size, const void* value, unsigned long value_size)
{
  rtl_hash_entry_t* entry = rtl_malloc(sizeof(rtl_hash_entry_t));
  if (!entry) {
    return NULL;
  }

  // Allocate and copy key
  entry->key = rtl_malloc(key_size);
  if (!entry->key) {
    rtl_free(entry);
    return NULL;
  }
  memcpy(entry->key, key, key_size);
  entry->key_size = key_size;

  // Allocate and copy value
  entry->value = rtl_malloc(value_size);
  if (!entry->value) {
    rtl_free(entry->key);
    rtl_free(entry);
    return NULL;
  }
  memcpy(entry->value, value, value_size);
  entry->value_size = value_size;

  return entry;
}

/**
 * @internal
 * @brief Destroys a hash entry and frees its memory.
 * @param entry Pointer to the hash entry to destroy.
 */
static void _rtl_hash_destroy_entry(rtl_hash_entry_t* entry)
{
  if (entry) {
    rtl_free(entry->key);
    rtl_free(entry->value);
    rtl_free(entry);
  }
}

bool rtl_hash_table_init(rtl_hash_table_t* table, unsigned long bucket_count,
  rtl_hash_function_t hash_function, rtl_hash_key_compare_t key_compare)
{
  rtl_assert(table != NULL, "Hash table cannot be NULL");
  rtl_assert(bucket_count > 0, "Bucket count must be greater than 0");
  rtl_assert(hash_function != NULL, "Hash function cannot be NULL");
  rtl_assert(key_compare != NULL, "Key compare function cannot be NULL");

  // Allocate buckets array
  table->buckets = rtl_malloc(bucket_count * sizeof(rtl_list_entry_t));
  if (!table->buckets) {
    return false;
  }

  // Initialize all buckets as empty lists
  for (unsigned long i = 0; i < bucket_count; i++) {
    rtl_list_init(&table->buckets[i]);
  }

  table->bucket_count = bucket_count;
  table->entry_count = 0;
  table->hash_function = hash_function;
  table->key_compare = key_compare;

  return true;
}

void rtl_hash_table_cleanup(rtl_hash_table_t* table)
{
  if (!table) {
    return;
  }

  // Free all entries in all buckets
  for (unsigned long i = 0; i < table->bucket_count; i++) {
    rtl_list_entry_t* current;
    rtl_list_entry_t* next;
    rtl_list_for_each_safe(current, next, &table->buckets[i])
    {
      rtl_hash_entry_t* entry = rtl_list_record(current, rtl_hash_entry_t, list_entry);
      rtl_list_remove(current);
      _rtl_hash_destroy_entry(entry);
    }
  }

  rtl_free(table->buckets);
  table->buckets = NULL;
  table->bucket_count = 0;
  table->entry_count = 0;
  table->hash_function = NULL;
  table->key_compare = NULL;
}

bool rtl_hash_table_insert(rtl_hash_table_t* table, const void* key, unsigned long key_size,
  const void* value, unsigned long value_size)
{
  rtl_assert(table != NULL, "Hash table cannot be NULL");
  rtl_assert(key != NULL, "Key cannot be NULL");
  rtl_assert(value != NULL, "Value cannot be NULL");
  rtl_assert(key_size > 0, "Key size must be greater than 0");
  rtl_assert(value_size > 0, "Value size must be greater than 0");

  uint32_t hash = table->hash_function(key, key_size);
  unsigned long bucket_index = hash % table->bucket_count;
  rtl_list_entry_t* bucket = &table->buckets[bucket_index];

  // Check if key already exists
  rtl_hash_entry_t* existing_entry =
    _rtl_hash_find_entry(bucket, key, key_size, table->key_compare);
  if (existing_entry) {
    // Update existing entry's value
    void* new_value = rtl_malloc(value_size);
    if (!new_value) {
      return false;
    }

    memcpy(new_value, value, value_size);
    rtl_free(existing_entry->value);
    existing_entry->value = new_value;
    existing_entry->value_size = value_size;
    return true;
  }

  // Create new entry
  rtl_hash_entry_t* new_entry = _rtl_hash_create_entry(key, key_size, value, value_size);
  if (!new_entry) {
    return false;
  }

  // Check for collision (bucket already has entries)
  if (!rtl_list_empty(bucket)) {
    rtl_log_wrn("Hash collision detected in bucket %lu", bucket_index);
  }

  // Add to bucket and increment count
  rtl_list_add_head(bucket, &new_entry->list_entry);
  table->entry_count++;
  return true;
}

void* rtl_hash_table_find(
  const rtl_hash_table_t* table, const void* key, unsigned long key_size, unsigned long* value_size)
{
  rtl_assert(table != NULL, "Hash table cannot be NULL");
  rtl_assert(key != NULL, "Key cannot be NULL");
  rtl_assert(key_size > 0, "Key size must be greater than 0");

  const uint32_t hash = table->hash_function(key, key_size);
  const unsigned long bucket_index = hash % table->bucket_count;
  const rtl_list_entry_t* bucket = &table->buckets[bucket_index];

  const rtl_hash_entry_t* entry = _rtl_hash_find_entry(bucket, key, key_size, table->key_compare);
  if (entry) {
    if (value_size) {
      *value_size = entry->value_size;
    }
    return entry->value;
  }

  return NULL;
}

bool rtl_hash_table_remove(rtl_hash_table_t* table, const void* key, unsigned long key_size)
{
  rtl_assert(table != NULL, "Hash table cannot be NULL");
  rtl_assert(key != NULL, "Key cannot be NULL");
  rtl_assert(key_size > 0, "Key size must be greater than 0");

  const uint32_t hash = table->hash_function(key, key_size);
  const unsigned long bucket_index = hash % table->bucket_count;
  const rtl_list_entry_t* bucket = &table->buckets[bucket_index];

  rtl_hash_entry_t* entry = _rtl_hash_find_entry(bucket, key, key_size, table->key_compare);
  if (entry) {
    rtl_list_remove(&entry->list_entry);
    _rtl_hash_destroy_entry(entry);
    table->entry_count--;
    return true;
  }

  return false;
}

unsigned long rtl_hash_table_size(const rtl_hash_table_t* table)
{
  rtl_assert(table != NULL, "Hash table cannot be NULL");
  return table->entry_count;
}

bool rtl_hash_table_empty(const rtl_hash_table_t* table)
{
  rtl_assert(table != NULL, "Hash table cannot be NULL");
  return table->entry_count == 0;
}

double rtl_hash_table_load_factor(const rtl_hash_table_t* table)
{
  rtl_assert(table != NULL, "Hash table cannot be NULL");
  return (double)table->entry_count / (double)table->bucket_count;
}

uint32_t rtl_hash_fnv1a(const void* key, unsigned long key_size)
{
  // FNV-1a hash algorithm (32-bit version)
  const uint32_t FNV_OFFSET_BASIS_32 = 2166136261U;
  const uint32_t FNV_PRIME_32 = 16777619U;

  uint32_t hash = FNV_OFFSET_BASIS_32;
  const unsigned char* data = key;

  for (unsigned long i = 0; i < key_size; i++) {
    hash ^= data[i];
    hash *= FNV_PRIME_32;
  }

  return hash;
}

int rtl_hash_key_compare_bytes(
  const void* key1, unsigned long key1_size, const void* key2, unsigned long key2_size)
{
  if (key1_size != key2_size) {
    return 1;
  }

  return memcmp(key1, key2, key1_size);
}

int rtl_hash_key_compare_string(
  const void* key1, unsigned long key1_size, const void* key2, unsigned long key2_size)
{
  (void)key1_size;  // Unused for string comparison
  (void)key2_size;  // Unused for string comparison

  return strcmp(key1, key2);
}
