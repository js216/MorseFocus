/**
 * @file run_gen.c
 * @brief Generate weighted random pattern.
 *
 * run_gen is a command-line utility that generates a random string of
 * printable, space-separated words. Each word has a random length between
 * specified minimum and maximum values. Characters are drawn from a
 * customizable set, and their frequency can be weighted using a file of
 * floating-point values.
 *
 * Usage:
 *
 *    run_gen NUM_CHAR [options]
 *
 * Options:
 *
 *   -i MIN      set minimum word length (default: 2)
 *   -x MAX      set maximum word length (default: 7)
 *   -w FILE     load character weights from the last line of FILE
 *   -c CHARS    specify a custom character set (printable, no spaces)
 *   -o FILE     write output to FILE instead of printing to standard output
 *   -s SCALE    multiply all weights by scale factor (default 1.0)
 *
 * @author Jakob Kastelic
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "debug.h"
#include "gen.h"
#include "str.h"
#include "record.h"

#define DEFAULT_MIN_WORD 2
#define DEFAULT_MAX_WORD 7

int silence_errors;

static void usage(const char *prog)
{
   fprintf(stderr,
      "Usage: %s num_char [options]\n"
      "options:\n"
      "  -i MIN     set minimum word length (default %d)\n"
      "  -x MAX     set maximum word length (default %d)\n"
      "  -w FILE    load weights from last line of FILE\n"
      "  -c CHARS   use custom charset (printable, no space)\n"
      "  -o FILE    write output to FILE instead of stdout\n"
      "  -s scale   multiply all weights by scale (default 1.0)\n",
      prog, DEFAULT_MIN_WORD, DEFAULT_MAX_WORD);
}

int main(int argc, char **argv)
{
   if (argc < 2) {
      usage(argv[0]);
      return -1;
   }

   size_t num_char = (size_t)atoi(argv[1]);
   int min_word = DEFAULT_MIN_WORD;
   int max_word = DEFAULT_MAX_WORD;
   const char *wfile = NULL;
   const char *charset = NULL;
   const char *out_file = NULL;
   float scale = 1.0f;
   float *wptr = NULL;

   for (int i = 2; i < argc; ++i) {
      if (!strcmp(argv[i], "-i") && i + 1 < argc) {
         min_word = atoi(argv[++i]);
      } else if (!strcmp(argv[i], "-x") && i + 1 < argc) {
         max_word = atoi(argv[++i]);
      } else if (!strcmp(argv[i], "-w") && i + 1 < argc) {
         wfile = argv[++i];
      } else if (!strcmp(argv[i], "-c") && i + 1 < argc) {
         charset = argv[++i];
      } else if (!strcmp(argv[i], "-o") && i + 1 < argc) {
         out_file = argv[++i];
      } else if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
         scale = strtof(argv[++i], NULL);
      } else {
         usage(argv[0]);
         return -1;
      }
   }

   struct record l = {0};
   if (wfile) {
      l = record_load_last(wfile);
      if (l.valid == 0) {
         ERROR("invalid record obtained from %s", wfile);
         return -1;
      }

      for (int i = 0; i < MAX_CHARSET_LEN; i++) {
         l.weights[i] *= scale;
         l.weights[i] += 1.0f;
      }

      wptr = l.weights;
   }

   char *buffer = malloc(num_char + 1);
   if (!buffer) {
      ERROR("out of memory");
      return -1;
   }

   if (gen_chars(buffer, num_char, min_word, max_word, wptr, charset) != 0) {
      ERROR("gen_chars() failed");
      free(buffer);
      return -1;
   }

   if (out_file) {
      FILE *f = fopen(out_file, "w");
      if (!f) {
         ERROR("cannot open %s for writing", out_file);
         free(buffer);
         return -1;
      }
      fputs(buffer, f);
      fclose(f);
   } else {
      puts(buffer);
   }

   free(buffer);
   return 0;
}

// end file run_gen.c

