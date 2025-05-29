/**
 * @file test_diff.c
 * @brief Test program for the lev_diff function.
 *
 * @author Jakob Kastelic
 */

#include <stdio.h>
#include <string.h>

#include "diff.h"
#include "weights.h"
#include "str.h"

int main(void)
{
   const char *s1 = "abc test hey";
   const char *s2 = "abd tests hey";

   int weights[MAX_CHARS] = {0};
   int distance = lev_diff(weights, s1, s2);

   float result[MAX_CHARS] = {0};
   for (int i = 0; i < MAX_CHARS; ++i) {
      result[i] = (float)weights[i];
   }

   printf("Levenshtein distance: %d\n", distance);
   printf("Character edit frequencies (insertions/deletions):\n");

   weights_printout(result, MAX_CHARS);

   return 0;
}


// end file test_diff.c

