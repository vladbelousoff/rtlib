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

#include "rtl_test.h"

#include <stdio.h>

// Global test context
struct rtl_test_context g_rtl_test_ctx = { 0 };

void rtl_test_init()
{
  g_rtl_test_ctx.tests_run = 0;
  g_rtl_test_ctx.tests_failed = 0;
  g_rtl_test_ctx.current_test_name = NULL;

  printf("\n===== RTL TEST SUITE =====\n\n");
}

int rtl_test_summary()
{
  printf("\n===== TEST SUMMARY =====\n");
  printf("Tests run: %d\n", g_rtl_test_ctx.tests_run);
  printf("Tests failed: %d\n", g_rtl_test_ctx.tests_failed);

  if (g_rtl_test_ctx.tests_failed > 0) {
    printf("\nRESULT: FAILURE\n\n");
    return 1;
  } else {
    printf("\nRESULT: SUCCESS\n\n");
    return 0;
  }
}
