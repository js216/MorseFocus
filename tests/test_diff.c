/**
 * @file test_diff.c
 * @brief Test program for the lev_diff function.
 *
 * @author Jakob Kastelic
 */

#include <stdio.h>
#include <string.h>

#include "diff.h"
#include "record.h"
#include "str.h"

int test_diff(void)
{
   const char *s1 = "abc test hey";
   const char *s2 = "abd tests hey";

   struct record r = {0};
   int distance = lev_diff(&r, s1, s2);

   // check distance
   if (distance != 2) {
       printf("FAIL: incorrect distance %d calculated\n", distance);
       return -1;
   }

   // expected weights
   float w_corr[MAX_CHARSET_LEN] = {0};
   w_corr[str_char_to_int('c')] = 1;
   w_corr[str_char_to_int('d')] = 1;
   w_corr[str_char_to_int('s')] = 1;

   // compare actual and expected weights
   for (int i=0; i<MAX_CHARSET_LEN; i++) {
      if (r.weights[i] != w_corr[i]) {
         printf("FAIL: incorrect weight for '%c'\n", str_int_to_char(i));
         return -1;
      }
   }

   printf("SUCCESS: test_diff\n");
   return 0;
}


// end file test_diff.c

