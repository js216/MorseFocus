/**
 * @file run_diff.c
 * @brief Compare two text files.
 *
 * run_diff is a command-line tool that compares two text files, computes the
 * Levenshtein-based difference between their contents, and combines this
 * difference with previously stored weights. It optionally writes the result to
 * a file.
 *
 * Usage:
 *
 *    run_diff f1 f2 [options]
 *
 * Positional Arguments:
 *
 * f1: path to first input text file
 * f2: path to second input text file
 *
 * Optional Flags:
 *
 * -w f3: path to file containing stored weights (default: none)
 * -s scale: floating-point scaling factor (default: 1.0)
 * -o f4: append the resulting weights to file f4 (default: print only)
 *
 * @author Jakob Kastelic
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "str.h"
#include "weights.h"
#include "diff.h"

#define MAX_LEN 8192
#define NUM_CHARS 95

static void print_usage(const char *prog)
{
   fprintf(stderr,
      "Usage: %s f1 f2 [options]\n"
      "Options:\n"
      "  -w file   load weights from file\n"
      "  -s scale  scale output weights (default: 1.0)\n"
      "  -o file   append output weights to file\n", prog);
}

int main(int argc, char *argv[])
{
   if (argc < 3) {
      print_usage(argv[0]);
      return 1;
   }

   const char *file1 = argv[1];
   const char *file2 = argv[2];
   const char *wfile = NULL;
   const char *ofile = NULL;
   float scale = 1.0f;

   // parse optional arguments
   for (int i = 3; i < argc; ++i) {
      if (strcmp(argv[i], "-w") == 0 && i + 1 < argc) {
         wfile = argv[++i];
      } else if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
         scale = strtof(argv[++i], NULL);
      } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
         ofile = argv[++i];
      } else {
         print_usage(argv[0]);
         return 1;
      }
   }

   // allocate buffers
   char buf1[MAX_LEN], buf2[MAX_LEN];
   char clean1[MAX_LEN], clean2[MAX_LEN];
   int len1 = str_read_file(buf1, file1, MAX_LEN);
   int len2 = str_read_file(buf2, file2, MAX_LEN);
   if (len1 <= 0 || len2 <= 0) {
      fprintf(stderr, "error reading input files\n");
      return 1;
   }

   str_clean(clean1, buf1, len1);
   str_clean(clean2, buf2, len2);

   int weights[NUM_CHARS] = {0};
   float result[NUM_CHARS] = {0};

   lev_diff(weights, clean1, clean2);

   for (int i = 0; i < NUM_CHARS; ++i) {
      result[i] = (float)weights[i];
   }

   if (wfile) {
      float loaded[NUM_CHARS] = {0};
      if (weights_load_last(loaded, wfile, NUM_CHARS) > 0) {
         for (int i = 0; i < NUM_CHARS; ++i) {
            result[i] += loaded[i];
         }
      } else {
         fprintf(stderr, "warning: failed to load weights from %s\n", wfile);
      }
   }

   for (int i = 0; i < NUM_CHARS; ++i) {
      result[i] *= scale;
   }

   if (ofile) {
      if (weights_append(ofile, result, NUM_CHARS) != 0) {
         fprintf(stderr, "error writing to file: %s\n", ofile);
         return 1;
      }
   } else {
      weights_printout(result, NUM_CHARS);
   }

   return 0;
}

// end file run_diff.c

