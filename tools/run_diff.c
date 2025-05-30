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
 * -d decay: floating-point decay factor (default: 1.0)
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

static void print_usage(const char *prog)
{
   fprintf(stderr,
      "Usage: %s f1 f2 [options]\n"
      "Options:\n"
      "  -w file   load weights from file\n"
      "  -d decay  scale output weights (default: 1.0)\n"
      "  -o file   append output weights to file\n", prog);
}

int main(int argc, char *argv[])
{
   if (argc < 3) {
      print_usage(argv[0]);
      return -1;
   }

   const char *file1 = argv[1];
   const char *file2 = argv[2];
   const char *wfile = NULL;
   const char *ofile = NULL;
   float decay = 1.0f;

   // parse optional arguments
   for (int i = 3; i < argc; ++i) {
      if (strcmp(argv[i], "-w") == 0 && i + 1 < argc) {
         wfile = argv[++i];
      } else if (strcmp(argv[i], "-d") == 0 && i + 1 < argc) {
         decay = strtof(argv[++i], NULL);
      } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
         ofile = argv[++i];
      } else {
         print_usage(argv[0]);
         return -1;
      }
   }

   // allocate buffers
   char buf1[MAX_LEN], buf2[MAX_LEN];
   char clean1[MAX_LEN], clean2[MAX_LEN];
   int len1 = str_read_file(buf1, file1, MAX_LEN);
   int len2 = str_read_file(buf2, file2, MAX_LEN);
   if (len1 <= 0 || len2 <= 0) {
      fprintf(stderr, "error reading input files\n");
      return -1;
   }

   str_clean(clean1, buf1, len1);
   str_clean(clean2, buf2, len2);

   float weights[MAX_CHARS] = {0};

   const int diff = lev_diff(weights, clean1, clean2);

   printf("Distance: %d\n", diff);
   weights_printout(weights, MAX_CHARS);

   if (wfile) {
      float loaded[MAX_CHARS] = {0};
      if (weights_load_last(loaded, wfile, MAX_CHARS) < 0) {
         fprintf(stderr, "warning: failed to load weights from %s\n", wfile);
         return -1;
      }

      weights_add(weights, weights, loaded, MAX_CHARS);
   }

   if (ofile) {
      if (weights_append(ofile, weights, MAX_CHARS, decay) != 0) {
         fprintf(stderr, "error writing to file: %s\n", ofile);
         return -1;
      }
   }

   return 0;
}

// end file run_diff.c

