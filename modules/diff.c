/**
 * @file diff.c
 * @brief Levenshtein distance computation and character frequency tracking.
 *
 * @author Jakob Kastelic
 */

#include "diff.h"
#include "debug.h"
#include "str.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int lev_diff(struct record *r, const char *s1, const char *s2)
{
   size_t len1 = strlen(s1);
   size_t len2 = strlen(s2);

   if ((len1 == 0) || (len2 == 0)) {
      ERROR("cannot compare zero-length strings");
      return -1;
   }

   int **dp = calloc((len1 + 1), sizeof(int *));
   if (!dp)
      return -1;

   for (size_t i = 0; i <= len1; ++i) {
      dp[i] = malloc((len2 + 1) * sizeof(int));
      if (!dp[i]) {
         for (size_t k = 0; k < i; ++k)
            free(dp[k]);
         free(dp);
         return -1;
      }
   }

   // initialize first row and column
   for (size_t i = 0; i <= len1; ++i)
      dp[i][0] = i;
   for (size_t j = 0; j <= len2; ++j)
      dp[0][j] = j;

   // fill the matrix
   for (size_t i = 1; i <= len1; ++i) {
      for (size_t j = 1; j <= len2; ++j) {
         int cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;
         int del = dp[i - 1][j] + 1;
         int ins = dp[i][j - 1] + 1;
         int sub = dp[i - 1][j - 1] + cost;
         int min = del;
         if (ins < min)
            min = ins;
         if (sub < min)
            min = sub;
         dp[i][j] = min;
      }
   }

   // backtrack to count edits per character
   int i = (size_t)len1;
   int j = (size_t)len2;
   while (i > 0 || j > 0) {
      if (i > 0 && j > 0 &&
          dp[i][j] == dp[i - 1][j - 1] + ((s1[i - 1] != s2[j - 1]) ? 1 : 0)) {
         if (s1[i - 1] != s2[j - 1]) {
            // substitution: count both characters
            r->weights[str_char_to_int(s1[i - 1])]++;
            r->weights[str_char_to_int(s2[j - 1])]++;
         }
         --i;
         --j;
      } else if (i > 0 && dp[i][j] == dp[i - 1][j] + 1) {
         // deletion from s1
         r->weights[str_char_to_int(s1[i - 1])]++;
         --i;
      } else {
         // insertion into s1 (from s2)
         r->weights[str_char_to_int(s2[j - 1])]++;
         --j;
      }
   }

   int dist = dp[len1][len2];

   for (size_t k = 0; k <= len1; ++k)
      free(dp[k]);
   free(dp);

   return dist;
}

// end of file diff.c
