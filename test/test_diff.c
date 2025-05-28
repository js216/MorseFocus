/**
 * @file test_diff.c
 * @brief Test program for the lev_diff function.
 *
 * @author Jakob Kastelic
 */

#include <stdio.h>
#include <string.h>
#include "diff.h"

int main(void)
{
   const char *s1 = "abc";
   const char *s2 = "abd";

   int warr[95] = {0};

   int distance = lev_diff(warr, s1, s2);

   printf("Levenshtein distance: %d\n", distance);
   printf("Character edit frequencies (insertions/deletions):\n");

   for (int i = 0; i < 95; ++i) {
      if (warr[i] > 0) {
         printf("'%c': %d\n", i + 33, warr[i]);
      }
   }

   return 0;
}

// end file test_diff.c

