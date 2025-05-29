/**
 * @file weights.c
 * @brief Manipulation of character weight factors.
 *
 * @author Jakob Kastelic
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "str.h"
#include "weights.h"

#define MAX_LINE 2048

int weights_load_last(float *weights, const char *fname, const int max_num)
{
   FILE *fp = fopen(fname, "r");
   if (!fp)
      return -1;

   char line[MAX_LINE];
   char *last_line = NULL;

   // read all lines, keep last non-empty line
   while (fgets(line, sizeof(line), fp)) {
      // check if line has any non-whitespace characters
      char *p = line;
      while (*p != '\0' && isspace((unsigned char)*p))
         p++;
      if (*p != '\0') // non-empty line
         last_line = line;
   }
   fclose(fp);

   if (!last_line)
      return -1;

   // trim trailing whitespace from last_line
   size_t len = strlen(last_line);
   while (len > 0 && isspace((unsigned char)last_line[len - 1])) {
      last_line[len - 1] = '\0';
      len--;
   }

   // skip ahead to where weights begin
   last_line = str_skip_fields(last_line, WEIGHTS_SKIP);

   if (!last_line)
      return -1;

   // parse floats from last_line into weights array
   int count = 0;
   char *token = strtok(last_line, " ");

   while (token != NULL && count < max_num) {
      char *endptr;
      float val = strtof(token, &endptr);
      if (endptr == token)
         break; // no conversion done
      weights[count++] = val;
      token = strtok(NULL, " ");
   }

   return count;
}


/**
 * @brief Check if a floating-point weight has no fractional part.
 *
 * This function determines whether the given float value represents
 * an integer (i.e., its fractional part is zero).
 *
 * @param weight The float value to check.
 * @return int Returns 1 if the fractional part of the weight is 0, otherwise 0.
 */
static int weights_is_integer(const float weight)
{
    return floorf(weight) == weight;
}


int weights_append(const char *fname, const float *weights, const int nw)
{
   // open file for writing
   FILE *fp = fopen(fname, "a");
   if (!fp)
      return -1;

   // write date and time
   if (str_write_time(fp) != 0) {
      return -1;
   }

   // write weights
   for (int i = 0; i < nw; ++i) {
      int ret = -1;
      if (weights_is_integer(weights[i]))
         ret = fprintf(fp, "%.0f", weights[i]);
      else
         ret = fprintf(fp, "%.6f", weights[i]);
      if (ret < 0) {
         fclose(fp);
         return -1;
      }

      if (i < nw - 1)
         fputc(' ', fp);
   }

   // terminating newline
   fputc('\n', fp);
   fclose(fp);
   return 0;
}


int weights_add(float *w, const float *w1, const float *w2, const int len)
{
   int i;
   for (i = 0; i < len; i++) {
      w[i] = w1[i] + w2[i];
   }
   return 0;
}


void weights_printout(const float *weights, const int num)
{
   for (int i = 0; i < num; i++) {
      if (weights[i] > 0) {
         char ch = str_int_to_char(i);
         if (ch == '\0') {
            fprintf(stderr, "error: invalid character number %d\n", i);
            return;
         }
         printf("'%c' : %f\n", ch, weights[i]);
      }
   }
}

// end file weights.c

