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


int weights_append(const char *fname, const float *weights, const int nw)
{
   FILE *fp = fopen(fname, "a");
   if (!fp)
      return -1;

   for (int i = 0; i < nw; ++i) {
      if (fprintf(fp, "%.6f", weights[i]) < 0) {
         fclose(fp);
         return -1;
      }

      if (i < nw - 1)
         fputc(' ', fp);
   }

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
         printf("'%c' : %f\n", i + 33, weights[i]);
      }
   }
   printf("\n");
}

// end file weights.c

