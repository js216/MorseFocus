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
#include "record.h"
#include "diff.h"

#define MAX_LEN 8192

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

   // optional data to store in the file
   float scale = 0.000;
   float speed1 = 0.0;
   float speed2 = 0.0;
   char *charset = "~";

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

   struct record r = {0};

   const int dist = lev_diff(&r, clean1, clean2);

   printf("Distance: %d\n", dist);
   record_printout(&r);

   if (wfile) {
      struct record l = record_load_last(wfile);
      if (l.valid == 0) {
         fprintf(stderr, "warning: invalid record obtained from %s\n", wfile);
         return -1;
      }

      for (int i = 0; i < MAX_CHARSET_LEN; i++)
         r.weights[i] = r.weights[i] + l.weights[i];
   }

   if (ofile) {
      // current time
      time_t now = time(NULL);
      r.datetime = *localtime(&now);
      r.len = len1;
      r.dist = dist;
      r.decay = decay;
      r.scale = scale;
      r.speed1 = speed1;
      r.speed2 = speed2;

      // copy charset, making sure it's not too long to fit
      const size_t len = strlen(charset);
      if (len >= MAX_CHARSET_LEN) {
         fprintf(stderr, "error: charset too long (max %d characters)\n",
               MAX_CHARSET_LEN - 1);
         return -1;
      }
      memcpy(r.charset, charset, len);

      // write record to file
      if (record_append(ofile, &r) != 0) {
         fprintf(stderr, "error writing record to file: %s\n", ofile);
         return -1;
      }
   }

   return 0;
}

// end file run_diff.c

