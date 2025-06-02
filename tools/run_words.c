/**
 * @file run_words.c
 * @brief Command-line utility for generating weighted random words.
 *
 * This program wraps the `gen_words()` function, allowing the user
 * to specify a word source file and generate a number of words
 * according to optional parameters.
 *
 * Usage:
 *   run_words FILE NUM_WORDS [options]
 *
 * Mandatory arguments:
 *   FILE       file containing one word per line
 *   NUM_WORDS  number of words to generate
 *
 * Options:
 *   -o FILE    write output to FILE instead of standard output
 *   -n NUM     number of lines to read from FILE (default: all)
 *
 * @author Jakob Kastelic
 */

#include "debug.h"
#include "gen.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int silence_errors;

static void usage(const char *progname)
{
   printf("Usage: %s NUM_WORDS [FILE] [options]\n", progname);
   printf("\nMandatory argument:\n");
   printf("  NUM_WORDS   number of words to generate\n");
   printf("\nOptional arguments:\n");
   printf("  FILE        input file containing one word per line (default: "
          "standard input)\n");
   printf("  -o FILE     write output to FILE instead of standard output\n");
   printf("  -n NUM      number of lines to read from input (default: all)\n");
}

int main(int argc, char *argv[])
{
   if (argc < 2) {
      usage(argv[0]);
      return 1;
   }

   const char *num_words_str = argv[1];
   const char *word_file = NULL;
   const char *out_file = NULL;
   int num_lines = -1; // default: read all lines

   // parse NUM_WORDS
   char *endptr;
   int num_words = (int)strtol(num_words_str, &endptr, 10);
   if (*endptr != '\0' || num_words <= 0) {
      ERROR("invalid NUM_WORDS: '%s'\n", num_words_str);
      return 1;
   }

   // determine if FILE was provided
   int arg_index = 2;
   if (arg_index < argc && argv[arg_index][0] != '-') {
      word_file = argv[arg_index];
      ++arg_index;
   }

   // parse options
   for (int i = arg_index; i < argc; ++i) {
      if (strcmp(argv[i], "-o") == 0) {
         if (i + 1 >= argc) {
            ERROR("missing argument after -o\n");
            return 1;
         }
         out_file = argv[++i];
      } else if (strcmp(argv[i], "-n") == 0) {
         if (i + 1 >= argc) {
            ERROR("missing argument after -n\n");
            return 1;
         }
         num_lines = (int)strtol(argv[++i], &endptr, 10);
         if (*endptr != '\0' || num_lines <= 0) {
            ERROR("invalid number of lines: '%s'\n", argv[i]);
            return 1;
         }
      } else {
         ERROR("unknown option: %s\n", argv[i]);
         return 1;
      }
   }

   // run generator
   if (gen_words(out_file, word_file, num_words, num_lines) != 0) {
      ERROR("failed to generate words.\n");
      return 1;
   }

   return 0;
}

// end file run_words.c
