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
 *    run_diff F1 F2 [options]
 *
 * Positional Arguments:
 *
 *   F1 path to first input text file
 *   F2 path to second input text file
 *
 * Optional Flags:
 *
 *   -w F3      path to file containing stored weights (default: none)
 *   -d DECAY   floating-point decay factor (default: 1.0)
 *   -o F4      append the resulting weights to file f4 (default: print only)
 *   -s SCALE   to record to file (default: 0)
 *   -1 FIRST   speed to record to file (default: 0)
 *   -2 SECOND  speed to record to file (default: 0)
 *   -c CHARSET to record to file (default: "~")
 *
 * @author Jakob Kastelic
 */

#include "debug.h"
#include "diff.h"
#include "record.h"
#include "str.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_DIFF_LEN 8192
#define TARGET_ACCURACY 0.1
#define PID_K 1.0

int silence_errors;

static void print_usage(const char *prog)
{
   fprintf(stderr,
           "Usage: %s f1 f2 [options]\n"
           "Options:\n"
           "  -w file    load weights from file\n"
           "  -d decay   scale output weights (default: 1.0)\n"
           "  -o file    append output weights to file\n"
           "  -s scale   scale to record to file (default: 0)\n"
           "  -1 speed1  first speed to record to file (default: 0)\n"
           "  -2 speed2  second speed to record to file (default: 0)\n"
           "  -c charset charset to record to file (default: \"~\")\n",
           prog);
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
   float scale = 1.000;
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
      } else if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
         scale = strtof(argv[++i], NULL);
      } else if (strcmp(argv[i], "-1") == 0 && i + 1 < argc) {
         speed1 = strtof(argv[++i], NULL);
      } else if (strcmp(argv[i], "-2") == 0 && i + 1 < argc) {
         speed2 = strtof(argv[++i], NULL);
      } else if (strcmp(argv[i], "-c") == 0 && i + 1 < argc) {
         charset = argv[++i];
      } else {
         print_usage(argv[0]);
         return -1;
      }
   }

   // check files are not too long
   if ((str_file_len(file1) >= MAX_DIFF_LEN) ||
       (str_file_len(file1) >= MAX_DIFF_LEN)) {
      ERROR("files too long");
      return -1;
   }

   // allocate buffers
   char buf1[MAX_DIFF_LEN], buf2[MAX_DIFF_LEN];
   char clean1[MAX_DIFF_LEN], clean2[MAX_DIFF_LEN];
   int len1 = str_read_file(buf1, file1, MAX_DIFF_LEN);
   int len2 = str_read_file(buf2, file2, MAX_DIFF_LEN);
   if (len1 <= 0 || len2 <= 0) {
      ERROR("cannot read input files");
      return -1;
   }

   str_clean(clean1, buf1, len1);
   str_clean(clean2, buf2, len2);

   struct record r = {0};

   const int dist = lev_diff(&r, clean1, clean2);

   const float err_pct = 100 * (float)dist / (float)len1;
   printf("%d errors out of %d = %.1f%%\n", dist, len1, err_pct);
   printf("Next speed: %.1f\n",
          speed1 * (1 - PID_K * (err_pct / 100.0 - TARGET_ACCURACY)));
   record_printout(&r);

   if (wfile) {
      struct record l = record_load_last(wfile);
      if (l.valid == 0) {
         ERROR("invalid record obtained from %s", wfile);
         return -1;
      }

      for (int i = 0; i < MAX_CHARSET_LEN; i++)
         r.weights[i] = r.weights[i] + l.weights[i];
   }

   if (ofile) {
      // current time
      time_t now = time(NULL);
      r.valid = 1;
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
         ERROR("charset too long (max %d characters)", MAX_CHARSET_LEN - 1);
         return -1;
      }
      memcpy(r.charset, charset, len);

      // write record to file
      if (record_append(ofile, &r) != 0) {
         ERROR("writing record to file: %s\n", ofile);
         return -1;
      }
   }

   return 0;
}

// end file run_diff.c
