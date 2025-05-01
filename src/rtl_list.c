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

void rtl_list_init_head(struct rtl_list_entry* head)
{
  head->prev = head;
  head->next = head;
}

bool rtl_list_empty(const struct rtl_list_entry* head)
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
static void __rtl_list_insert(
  struct rtl_list_entry* _new, struct rtl_list_entry* prev, struct rtl_list_entry* next)
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
void rtl_list_add_head(struct rtl_list_entry* head, struct rtl_list_entry* entry)
{
  __rtl_list_insert(entry, head, head->next);
}

void rtl_list_add_tail(struct rtl_list_entry* head, struct rtl_list_entry* entry)
{
  __rtl_list_insert(entry, head->prev, head);
}

/**
 * @internal
 * @brief Removes an entry by connecting its neighbors.
 *        This is an internal function, list users should use rtl_list_remove().
 * @param prev Pointer to the previous entry.
 * @param next Pointer to the next entry.
 */
static void __rtl_list_remove(struct rtl_list_entry* prev, struct rtl_list_entry* next)
{
  next->prev = prev;
  prev->next = next;
}

void rtl_list_remove(const struct rtl_list_entry* entry)
{
  __rtl_list_remove(entry->prev, entry->next);
}
