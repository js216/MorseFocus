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

   printf("Levenshtein distance: %d\n", distance);
   printf("Character edit frequencies (insertions/deletions):\n");

   record_printout(&r);

   return 0;
}


// end file test_diff.c

