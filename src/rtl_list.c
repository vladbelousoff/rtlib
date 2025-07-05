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

#include "rtl_list.h"
#include <stddef.h>

void rtl_list_init(rtl_list_entry_t* head)
{
  head->prev = head;
  head->next = head;
}

bool rtl_list_empty(const rtl_list_entry_t* head)
{
  return head->next == head;
}

/**
 * @internal
 * @brief Inserts a new entry between two known consecutive entries.
 *        This is an internal function, list users should use
 *        rtl_list_push_front() or rtl_list_push_back().
 * @param _new Pointer to the new entry to be inserted.
 * @param prev Pointer to the entry that will precede the new entry.
 * @param next Pointer to the entry that will follow the new entry.
 */
static void _rtl_list_insert(
  rtl_list_entry_t* _new, rtl_list_entry_t* prev, rtl_list_entry_t* next)
{
  next->prev = _new;
  _new->next = next;
  _new->prev = prev;
  prev->next = _new;
}

/**
 * @brief Adds a new entry to the front of the list.
 * @param head Pointer to the list head entry.
 * @param entry Pointer to the new entry to add.
 */
void rtl_list_add_head(rtl_list_entry_t* head, rtl_list_entry_t* entry)
{
  _rtl_list_insert(entry, head, head->next);
}

void rtl_list_add_tail(rtl_list_entry_t* head, rtl_list_entry_t* entry)
{
  _rtl_list_insert(entry, head->prev, head);
}

/**
 * @internal
 * @brief Removes an entry by connecting its neighbors.
 *        This is an internal function, list users should use rtl_list_remove().
 * @param prev Pointer to the previous entry.
 * @param next Pointer to the next entry.
 */
static void _rtl_list_remove(rtl_list_entry_t* prev, rtl_list_entry_t* next)
{
  next->prev = prev;
  prev->next = next;
}

void rtl_list_remove(const rtl_list_entry_t* entry)
{
  _rtl_list_remove(entry->prev, entry->next);
}

rtl_list_entry_t* rtl_list_first(const rtl_list_entry_t* head)
{
  if (rtl_list_empty(head)) {
    return NULL;
  }

  return head->next;
}

rtl_list_entry_t* rtl_list_next(const rtl_list_entry_t* current, const rtl_list_entry_t* head)
{
  if (current == NULL || current->next == head) {
    return NULL;
  }

  return current->next;
}
