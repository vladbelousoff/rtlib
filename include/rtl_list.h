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

/**
 * @brief Get the struct for this entry.
 * @param address The address of the list entry structure.
 * @param type The type of the struct this is embedded in.
 * @param field The name of the list_entry within the struct.
 * @return A pointer to the containing structure.
 */
#define rtl_list_record(address, type, field)                                                      \
  ((type*)((char*)(address) - (char*)(&((type*)0)->field)))

/**
 * @brief Iterate over a list.
 * @param position The &struct rtl_list_entry to use as a loop cursor.
 * @param head The head for your list.
 */
#define rtl_list_for_each(position, head)                                                          \
  for (position = (head)->next; position != head; position = position->next)

/**
 * @brief Iterate over a list safe against removal of list entry.
 * @param position The &struct rtl_list_entry to use as a loop cursor.
 * @param n Another &struct rtl_list_entry to use as temporary storage.
 * @param head The head for your list.
 */
#define rtl_list_for_each_safe(position, n, head)                                                  \
  for (position = (head)->next, n = position->next; position != (head);                            \
    position = n, n = position->next)

/**
 * @brief Iterate over a list with an index counter.
 * @param index The index variable.
 * @param position The &struct rtl_list_entry to use as a loop cursor.
 * @param head The head for your list.
 */
#define rtl_list_for_each_indexed(index, position, head)                                           \
  for (position = (head)->next, index = 0; position != (head); position = position->next, index++)

/**
 * @brief Doubly linked list entry structure.
 *        Can be embedded in other structures to create linked lists.
 */
struct rtl_list_entry_t
{
  struct rtl_list_entry_t* prev;
  struct rtl_list_entry_t* next;
};

typedef struct rtl_list_entry_t rtl_list_entry_t;

/**
 * @brief Initializes the head of a list.
 * @param head Pointer to the list head entry.
 */
void rtl_list_init(rtl_list_entry_t* head);

/**
 * @brief Checks if a list is empty.
 * @param head Pointer to the list head entry.
 * @return Non-zero if the list is empty, 0 otherwise.
 */
bool rtl_list_empty(const rtl_list_entry_t* head);

/**
 * @brief Adds a new entry to the front of the list.
 * @param head Pointer to the list head entry.
 * @param entry Pointer to the new entry to add.
 */
void rtl_list_add_head(rtl_list_entry_t* head, rtl_list_entry_t* entry);

/**
 * @brief Adds a new entry to the back of the list.
 * @param head Pointer to the list head entry.
 * @param entry Pointer to the new entry to add.
 */
void rtl_list_add_tail(rtl_list_entry_t* head, rtl_list_entry_t* entry);

/**
 * @brief Removes an entry from the list.
 * @param entry Pointer to the entry to remove.
 *        Note: entry is not freed, only removed from the list.
 */
void rtl_list_remove(const rtl_list_entry_t* entry);

/**
 * @brief Gets the first entry in the list.
 * @param head Pointer to the list head entry.
 * @return Pointer to the first entry, or NULL if the list is empty.
 */
rtl_list_entry_t* rtl_list_first(const rtl_list_entry_t* head);

/**
 * @brief Gets the next entry in the list.
 * @param current Pointer to the current entry.
 * @param head Pointer to the list head entry.
 * @return Pointer to the next entry, or NULL if current is the last entry.
 */
rtl_list_entry_t* rtl_list_next(const rtl_list_entry_t* current, const rtl_list_entry_t* head);

/**
 * @brief Gets the length (number of entries) in the list.
 * @param head Pointer to the list head entry.
 * @return The number of entries in the list (excluding the head).
 */
unsigned long rtl_list_length(const rtl_list_entry_t* head);
