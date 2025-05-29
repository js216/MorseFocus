/**
 * @file test_weights.c
 * @brief Test program for the weights functions.
 *
 * @author Jakob Kastelic
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "str.h"
#include "weights.h"

#define TEST_FILE "test_weights.txt"
#define MAX_WEIGHTS 32

// writes known floating point data to the test file
static int create_test_file(const char *filename)
{
   FILE *fp = fopen(filename, "w");
   if (!fp) {
      perror("fopen");
      return -1;
   }

   fprintf(fp, "date time 1.0 2.0 3.0\n");
   fprintf(fp, "date time 4.0 5.0 6.0 7.0\n");
   fprintf(fp, "date time 8.5 9.25 10.75 11.125 12.625\n");  // last line

   fclose(fp);
   return 0;
}


// loads weights from last line and verifies expected values
static int test_weights_load_last(const char *filename)
{
   float weights[MAX_WEIGHTS] = {0.0f};
   int count = weights_load_last(weights, filename, MAX_WEIGHTS);

   if (count != 5) {
      fprintf(stderr, "error: unexpected weight count: %d\n", count);
      return -1;
   }

   float expected[] = {8.5f, 9.25f, 10.75f, 11.125f, 12.625f};
   for (int i = 0; i < count; ++i) {
      if (weights[i] != expected[i]) {
         fprintf(stderr, "error: mismatch at index %d: got %.3f, expected %.3f\n",
                 i, weights[i], expected[i]);
         return -1;
      }
   }

   // print results
   printf("loaded %d weight(s) from last line:\n", count);
   for (int i = 0; i < count; ++i)
      printf("weights[%d] = %.3f\n", i, weights[i]);

   return 0;
}


static int read_last_line(char *buf, size_t size, const char *fname)
{
   FILE *fp = fopen(fname, "r");
   if (!fp)
      return -1;

   char line[2048];
   char *last = NULL;

   while (fgets(line, sizeof(line), fp))
      last = line;

   if (!last) {
      fclose(fp);
      return -1;
   }

   strncpy(buf, last, size - 1);
   buf[size - 1] = '\0';

   fclose(fp);
   return 0;
}


static int test_weights_append(const char *fname)
{
   float weights[] = {1.23f, 4.56f, 7.89f};
   const int count = sizeof(weights) / sizeof(weights[0]);

   // remove file if it exists
   remove(fname);

   // append weights to file
   if (weights_append(fname, weights, count) != 0) {
      printf("test failed: could not append weights\n");
      return -1;
   }

   // read back the last line
   char buf[256];
   if (read_last_line(buf, sizeof(buf), fname) != 0) {
      printf("test failed: could not read back weights\n");
      return -1;
   }

   // build expected string
   char expected[256];
   snprintf(expected, sizeof(expected), "%.6f %.6f %.6f\n",
            weights[0], weights[1], weights[2]);

   // skip ahead to where weights begin
   const char *buf_weights = str_skip_fields(buf, WEIGHTS_SKIP);

   // compare
   if (strcmp(buf_weights, expected) != 0) {
      printf("test failed: expected \"%s\" but got \"%s\"\n", expected,
            buf_weights);
      return -1;
   }

   // success message
   printf("test_weights_append success\n");

   return 0;
}


static int test_weights_add(void)
{
   float w1[] = {1.0f, 2.5f, 3.3f, 4.4f};
   float w2[] = {0.5f, 1.5f, -1.3f, 2.6f};
   float expected[] = {1.5f, 4.0f, 2.0f, 7.0f};
   float w[4];

   int len = sizeof(w1) / sizeof(w1[0]);

   weights_add(w, w1, w2, len);

   for (int i = 0; i < len; i++)
   {
      if (w[i] != expected[i])
      {
         printf("test_weights_add failed at index %d: expected %f, got %f\n",
                i, expected[i], w[i]);
         return -1;
      }
   }

   // success message
   printf("test_weights_add success\n");
   return 0;
}


int main(void)
{
   if (create_test_file(TEST_FILE) != 0)
      return 1;

   if (test_weights_load_last(TEST_FILE) != 0)
      return 1;

   if (test_weights_append(TEST_FILE) != 0)
      return 1;

   if (test_weights_add() != 0)
      return 1;

   return 0;
}

// end file test_weights.c

