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

#include <stdbool.h>
#include "rtl_list.h"

/**
 * @brief Hash function type definition.
 * @param key Pointer to the key to hash.
 * @param key_size Size of the key in bytes.
 * @return Hash value for the key.
 */
typedef unsigned long (*rtl_hash_function_t)(const void* key, unsigned long key_size);

/**
 * @brief Key comparison function type definition.
 * @param key1 Pointer to the first key.
 * @param key1_size Size of the first key in bytes.
 * @param key2 Pointer to the second key.
 * @param key2_size Size of the second key in bytes.
 * @return 0 if keys are equal, non-zero otherwise.
 */
typedef int (*rtl_hash_key_compare_t)(
  const void* key1, unsigned long key1_size, const void* key2, unsigned long key2_size);

/**
 * @brief Hash table entry structure.
 *        Contains key-value pair and is linked via list for collision resolution.
 */
typedef struct rtl_hash_entry_t
{
  void* key;                   /**< Pointer to the key */
  unsigned long key_size;      /**< Size of the key in bytes */
  void* value;                 /**< Pointer to the value */
  unsigned long value_size;    /**< Size of the value in bytes */
  rtl_list_entry_t list_entry; /**< List entry for chaining collisions */
} rtl_hash_entry_t;

/**
 * @brief Hash table structure.
 *        Fixed-size hash table with chaining for collision resolution.
 */
typedef struct rtl_hash_table_t
{
  rtl_list_entry_t* buckets;          /**< Array of bucket heads */
  unsigned long bucket_count;         /**< Number of buckets */
  unsigned long entry_count;          /**< Current number of entries */
  rtl_hash_function_t hash_function;  /**< Hash function to use */
  rtl_hash_key_compare_t key_compare; /**< Key comparison function */
} rtl_hash_table_t;

/**
 * @brief Initializes a hash table with the specified number of buckets.
 * @param table Pointer to the hash table structure to initialize.
 * @param bucket_count Number of buckets in the hash table (must be > 0).
 * @param hash_function Hash function to use for hashing keys.
 * @param key_compare Key comparison function for comparing keys.
 * @return true if initialization was successful, false otherwise.
 */
bool rtl_hash_table_init(rtl_hash_table_t* table, unsigned long bucket_count,
  rtl_hash_function_t hash_function, rtl_hash_key_compare_t key_compare);

/**
 * @brief Cleans up a hash table and frees all associated internal memory.
 * @param table Pointer to the hash table to clean up.
 *        Note: This frees internal entries and buckets,
 *              but does not free the table structure itself.
 */
void rtl_hash_table_cleanup(rtl_hash_table_t* table);

/**
 * @brief Inserts or updates a key-value pair in the hash table.
 * @param table Pointer to the hash table.
 * @param key Pointer to the key data.
 * @param key_size Size of the key data in bytes.
 * @param value Pointer to the value data.
 * @param value_size Size of the value data in bytes.
 * @return true if the operation was successful, false otherwise.
 *         Note: If the key already exists, its value will be updated.
 */
bool rtl_hash_table_insert(rtl_hash_table_t* table, const void* key, unsigned long key_size,
  const void* value, unsigned long value_size);

/**
 * @brief Finds a value in the hash table by its key.
 * @param table Pointer to the hash table.
 * @param key Pointer to the key to search for.
 * @param key_size Size of the key in bytes.
 * @param value_size Pointer to store the size of the found value (optional).
 * @return Pointer to the value if found, NULL otherwise.
 */
void* rtl_hash_table_find(const rtl_hash_table_t* table, const void* key, unsigned long key_size,
  unsigned long* value_size);

/**
 * @brief Removes a key-value pair from the hash table.
 * @param table Pointer to the hash table.
 * @param key Pointer to the key to remove.
 * @param key_size Size of the key in bytes.
 * @return true if the key was found and removed, false otherwise.
 *         Note: This only removes the entry from the table,
 *               it does not free the user-provided key/value data.
 */
bool rtl_hash_table_remove(rtl_hash_table_t* table, const void* key, unsigned long key_size);

/**
 * @brief Gets the number of entries currently in the hash table.
 * @param table Pointer to the hash table.
 * @return Number of key-value pairs in the table.
 */
unsigned long rtl_hash_table_size(const rtl_hash_table_t* table);

/**
 * @brief Checks if the hash table is empty.
 * @param table Pointer to the hash table.
 * @return true if the table is empty, false otherwise.
 */
bool rtl_hash_table_empty(const rtl_hash_table_t* table);

/**
 * @brief Gets the load factor of the hash table.
 * @param table Pointer to the hash table.
 * @return Load factor (entries per bucket) as a floating point value.
 */
double rtl_hash_table_load_factor(const rtl_hash_table_t* table);

/**
 * @brief Default hash function using FNV-1a algorithm.
 * @param key Pointer to the key data.
 * @param key_size Size of the key in bytes.
 * @return Hash value for the key.
 */
unsigned long rtl_hash_fnv1a(const void* key, unsigned long key_size);

/**
 * @brief Default key comparison function for byte-wise comparison.
 * @param key1 Pointer to the first key.
 * @param key1_size Size of the first key in bytes.
 * @param key2 Pointer to the second key.
 * @param key2_size Size of the second key in bytes.
 * @return 0 if keys are equal, non-zero otherwise.
 */
int rtl_hash_key_compare_bytes(
  const void* key1, unsigned long key1_size, const void* key2, unsigned long key2_size);

/**
 * @brief String key comparison function.
 * @param key1 Pointer to the first null-terminated string.
 * @param key1_size Size of the first key (ignored for strings).
 * @param key2 Pointer to the second null-terminated string.
 * @param key2_size Size of the second key (ignored for strings).
 * @return 0 if strings are equal, non-zero otherwise.
 */
int rtl_hash_key_compare_string(
  const void* key1, unsigned long key1_size, const void* key2, unsigned long key2_size);
