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

#include "rtl.h"
#include "rtl_memory.h"

#include <stdio.h>

#define RTL_TEST_EQUAL(a, b)                                                                       \
  if (!(a == b)) {                                                                                 \
    fprintf(stderr, "%s != %s\n", #a, #b);                                                         \
    return __LINE__;                                                                               \
  }

int main(int argc, char** argv)
{
  (void)argc;
  (void)argv;

  // Init the library
  rtl_init();

  char* data = rtl_malloc(10);
#ifdef RTL_DEBUG_BUILD
  const struct rtl_memory_header* header =
    (struct rtl_memory_header*)(data - sizeof(struct rtl_memory_header));
  RTL_TEST_EQUAL(header->size, 10);
  RTL_TEST_EQUAL(header->source_location.line, 42);
#endif
  rtl_free(data);

  // Clean up the library
  rtl_term();

  return 0;
}
