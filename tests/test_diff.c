/**
 * @file test_diff.c
 * @brief Test program for the lev_diff function.
 *
 * @author Jakob Kastelic
 */

#include "modules/debug.h"
#include "modules/diff.h"
#include "modules/record.h"
#include "modules/str.h"
#include <math.h>
#include <stddef.h>

#define TEST_DIFF_NUM 6

struct test_diff_case {
   const char *s1;
   const char *s2;
   int expected_distance;
   float w_exp[MAX_CHARSET_LEN];
};

static struct test_diff_case get_diff_test_case(const int i)
{
   struct test_diff_case tc = {0};

   if (i == 0) {
      tc.s1 = "abc test hey";
      tc.s2 = "abd tests hey";
      tc.expected_distance = 2;
      tc.w_exp[str_char_to_int('c')] = 1.0F;
      tc.w_exp[str_char_to_int('d')] = 1.0F;
      tc.w_exp[str_char_to_int('s')] = 1.0F;
   }

   else if (i == 1) {
      tc.s1 = "hello";
      tc.s2 = "hullo";
      tc.expected_distance = 1;
      tc.w_exp[str_char_to_int('e')] = 1.0F;
      tc.w_exp[str_char_to_int('u')] = 1.0F;
   }

   else if (i == 2) {
      tc.s1 = "morse code";
      tc.s2 = "horse rode";
      tc.expected_distance = 2;
      tc.w_exp[str_char_to_int('c')] = 1.0F;
      tc.w_exp[str_char_to_int('h')] = 1.0F;
      tc.w_exp[str_char_to_int('m')] = 1.0F;
      tc.w_exp[str_char_to_int('r')] = 1.0F;
   }

   else if (i == 3) {
      tc.s1 = "ezb4z";
      tc.s2 = "ezb4z";
      tc.expected_distance = 0;
   }

   else if (i == 4) {
      tc.s1 = "a";
      tc.s2 = "b";
      tc.expected_distance = 1;
      tc.w_exp[str_char_to_int('a')] = 1.0F;
      tc.w_exp[str_char_to_int('b')] = 1.0F;
   }

   else if (i == 5) {
      tc.s1 = "cn";
      tc.s2 = "cn";
      tc.expected_distance = 0;
   }

   else {
      ERROR("invalid test case number");
   }

   return tc;
}

static int compare_weights(const float *actual, const float *expected,
                           size_t len, size_t case_num)
{
   const float eps = 1e-6F;

   for (size_t i = 0; i < len; ++i) {
      if (fabsf(actual[i] - expected[i]) > eps) {
         TEST_FAIL("test %zu: mismatch at char '%c' weight[%zu] = %.6f, "
                   "expected %.6f",
                   case_num,
                   str_int_to_char(i) == '\0' ? ' ' : str_int_to_char(i), i,
                   actual[i], expected[i]);
         return -1;
      }
   }

   return 0;
}

int test_diff(void)
{
   for (int i = 0; i < TEST_DIFF_NUM; ++i) {
      struct test_diff_case tc = get_diff_test_case(i);
      struct record r = {0};

      int dist = lev_diff(&r, tc.s1, tc.s2);
      if (dist != tc.expected_distance) {
         TEST_FAIL("test %d: expected distance %d, got %d", i,
                   tc.expected_distance, dist);
         return -1;
      }

      if (compare_weights(r.weights, tc.w_exp, MAX_CHARSET_LEN, i) != 0) {
         return -1;
      }
   }

   TEST_SUCCESS();
   return 0;
}

// end file test_diff.c
