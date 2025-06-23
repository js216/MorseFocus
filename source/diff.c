/**
 * @file diff.c
 * @brief Levenshtein distance computation and character frequency tracking.
 *
 * @author Jakob Kastelic
 */

#include "diff.h"
#include "debug.h"
#include "record.h"
#include "str.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int **alloc_matrix(size_t rows, size_t cols)
{
   int **matrix = (int **)calloc(rows, sizeof(int *));
   if (!matrix)
      return NULL;

   for (size_t i = 0; i < rows; ++i) {
      matrix[i] = malloc(cols * sizeof(int));
      if (!matrix[i]) {
         for (size_t k = 0; k < i; ++k)
            free(matrix[k]);
         free((void *)matrix);
         return NULL;
      }
   }
   return matrix;
}

static void free_matrix(int **matrix, size_t rows)
{
   for (size_t i = 0; i < rows; ++i)
      free(matrix[i]);
   free((void *)matrix);
}

static void init_matrix(int **dp, size_t len1, size_t len2)
{
   for (size_t i = 0; i <= len1; ++i)
      dp[i][0] = (int)i;
   for (size_t j = 0; j <= len2; ++j)
      dp[0][j] = (int)j;
}

static int compute_min3(int a, int b, int c)
{
   int min = a;
   if (b < min)
      min = b;
   if (c < min)
      min = c;
   return min;
}

static void fill_matrix(int **dp, const char *s1, const char *s2, size_t len1,
                        size_t len2)
{
   for (size_t i = 1; i <= len1; ++i) {
      for (size_t j = 1; j <= len2; ++j) {
         int cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;
         dp[i][j] = compute_min3(dp[i - 1][j] + 1,       // deletion
                                 dp[i][j - 1] + 1,       // insertion
                                 dp[i - 1][j - 1] + cost // substitution
         );
      }
   }
}

static void backtrack_weights(struct record *r, int **dp, const char *s1,
                              const char *s2, size_t len1, size_t len2)
{
   int i = (int)len1;
   int j = (int)len2;

   while (i > 0 || j > 0) {
      if (i > 0 && j > 0 &&
          dp[i][j] == dp[i - 1][j - 1] + ((s1[i - 1] != s2[j - 1]) ? 1 : 0)) {
         if (s1[i - 1] != s2[j - 1]) {
            r->weights[str_char_to_int(s1[i - 1])]++;
            r->weights[str_char_to_int(s2[j - 1])]++;
         }
         --i;
         --j;
      } else if (i > 0 && dp[i][j] == dp[i - 1][j] + 1) {
         r->weights[str_char_to_int(s1[i - 1])]++;
         --i;
      } else {
         r->weights[str_char_to_int(s2[j - 1])]++;
         --j;
      }
   }
}

int lev_diff(struct record *r, const char *s1, const char *s2)
{
   size_t len1 = strlen(s1);
   size_t len2 = strlen(s2);

   if (len1 == 0 || len2 == 0) {
      ERROR("cannot compare zero-length strings");
      return -1;
   }

   int **dp = alloc_matrix(len1 + 1, len2 + 1);
   if (!dp)
      return -1;

   init_matrix(dp, len1, len2);
   fill_matrix(dp, s1, s2, len1, len2);
   backtrack_weights(r, dp, s1, s2, len1, len2);

   int dist = dp[len1][len2];
   free_matrix(dp, len1 + 1);
   return dist;
}

// end of file diff.c
