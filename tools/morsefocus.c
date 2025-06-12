/**
 * @file morsefocus.c
 * @brief Generate weighted random pattern.
 *
 * morsefocus generates a random string of words, plays them in audio form as
 * Morse code, waits for the user input, and calculates the character accuracy
 * statistics.
 *
 * Usage:
 *
 * $ morsefocus NUM_CHAR [options]
 *
 * Options:
 *
 *  -i MIN       set minimum word length (default 2)
 *  -x MAX       set maximum word length (default 7)
 *  -t <freq>    Tone frequency Hz (60..10000), default 700
 *  -a <amp>     Amplitude (0..1), default 0.3
 *  -n <delay>   Initial delay seconds (0..60), default 1
 *  -w FILE      load weights from last line of FILE
 *  -1 <speed1>  Speed1 in WPM (1..500), default 25
 *  -2 <speed2>  Speed2 in WPM (1..500), default 25
 *  -s scale     multiply all weights by scale (default 1.0)
 *  -d decay     scale output weights (default: 1.0)
 *
 * @author Jakob Kastelic
 */

#include "cw.h"
#include "debug.h"
#include "diff.h"
#include "gen.h"
#include "record.h"
#include "str.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_DIFF_LEN 8192
#define TARGET_ACCURACY 0.1
#define PID_K 1.0

struct parsed_args {
   int min_word;
   int max_word;
   float freq;
   float amp;
   float delay;
   const char *wfile;
   struct record rec;
};

int silence_errors;

static void usage(const char *prog)
{
   fprintf(stderr,
           "Usage: %s num_char [options]\n"
           "options:\n"
           "  -i MIN       set minimum word length (default 2)\n"
           "  -x MAX       set maximum word length (default 7)\n"
           "  -t <freq>    Tone frequency Hz (60..10000), default 700\n"
           "  -a <amp>     Amplitude (0..1), default 0.3\n"
           "  -n <delay>   Initial delay seconds (0..60), default 1\n"
           "  -w FILE      load weights from last line of FILE\n"
           "  -1 <speed1>  Speed1 in WPM (1..500), default 25\n"
           "  -2 <speed2>  Speed2 in WPM (1..500), default 25\n"
           "  -s scale     multiply all weights by scale (default 1.0)\n"
           "  -d decay     scale output weights (default: 1.0)\n",
           prog);
}

static int check_float_range(float val, float min, float max, const char *name)
{
   if (val < min || val > max) {
      ERROR("Error: %s must be between %.2f and %.2f\n", name, min, max);
      return -1;
   }
   return 0;
}

static void handle_error(const char *msg)
{
   ERROR("%s\n", msg);
   exit(-1);
}

static struct parsed_args parse_args(int argc, char **argv)
{
   if (argc < 2) {
      usage(argv[0]);
      exit(-1);
   }

   struct parsed_args args = {.min_word = 2,
                              .max_word = 7,
                              .freq = 700.0f,
                              .amp = 0.3f,
                              .delay = 1.0f,
                              .wfile = NULL,
                              .rec = {
                                  .valid = 1,
                                  .decay = 1.0f,
                                  .scale = 1.0f,
                                  .speed1 = 25,
                                  .speed2 = 25,
                              }};

   // legacy field
   strcpy(args.rec.charset, "~");

   time_t now = time(NULL);
   args.rec.datetime = *localtime(&now);

   for (int i = 2; i < argc; ++i) {
      if (!strcmp(argv[i], "-i")) {
         if (++i >= argc)
            handle_error("Missing value for -i (minimum word length)");
         args.min_word = atoi(argv[i]);

      } else if (!strcmp(argv[i], "-x")) {
         if (++i >= argc)
            handle_error("Missing value for -x (maximum word length)");
         args.max_word = atoi(argv[i]);

      } else if (!strcmp(argv[i], "-w")) {
         if (++i >= argc)
            handle_error("Missing value for -w (weights file path)");
         args.wfile = argv[i];

      } else if (!strcmp(argv[i], "-t")) {
         if (++i >= argc)
            handle_error("Missing value for -t (tone frequency)");
         args.freq = strtof(argv[i], NULL);
         if (check_float_range(args.freq, 60.0f, 10000.0f, "frequency") < 0)
            exit(-1);

      } else if (!strcmp(argv[i], "-a")) {
         if (++i >= argc)
            handle_error("Missing value for -a (amplitude)");
         args.amp = strtof(argv[i], NULL);
         if (check_float_range(args.amp, 0.0f, 1.0f, "amplitude") < 0)
            exit(-1);

      } else if (!strcmp(argv[i], "-1")) {
         if (++i >= argc)
            handle_error("Missing value for -1 (speed1)");
         args.rec.speed1 = atoi(argv[i]);
         if (args.rec.speed1 < 1 || args.rec.speed1 > 500)
            handle_error("Speed1 must be in range 1..500");

      } else if (!strcmp(argv[i], "-2")) {
         if (++i >= argc)
            handle_error("Missing value for -2 (speed2)");
         args.rec.speed2 = atoi(argv[i]);
         if (args.rec.speed2 < 1 || args.rec.speed2 > 500)
            handle_error("Speed2 must be in range 1..500");

      } else if (!strcmp(argv[i], "-s")) {
         if (++i >= argc)
            handle_error("Missing value for -s (scale)");
         args.rec.scale = strtof(argv[i], NULL);

      } else if (!strcmp(argv[i], "-n")) {
         if (++i >= argc)
            handle_error("Missing value for -n (delay)");
         args.delay = strtof(argv[i], NULL);
         if (check_float_range(args.delay, 0.0f, 60.0f, "delay") < 0)
            exit(-1);

      } else if (!strcmp(argv[i], "-d")) {
         if (++i >= argc)
            handle_error("Missing value for -n (decay)");
         args.rec.decay = strtof(argv[i], NULL);
         if (check_float_range(args.rec.decay, 0.0f, 60.0f, "decay") < 0)
            exit(-1);

      } else {
         ERROR("Unrecognized option: %s\n", argv[i]);
         usage(argv[0]);
         exit(-1);
      }
   }

   return args;
}

static int ask_yes_no(const char *prompt)
{
   char buf[16];

   while (1) {
      printf("%s (y/n): ", prompt);

      if (fgets(buf, sizeof(buf), stdin) == NULL) {
         if (feof(stdin)) {
            ERROR("Input error: End of file detected\n");
         } else if (ferror(stdin)) {
            ERROR("Input error: I/O error detected\n");
         } else {
            ERROR("Input error: Unknown error\n");
         }
         return -1;
      }

      // Remove trailing newline if present
      buf[strcspn(buf, "\n")] = '\0';

      // Check for valid input (case-insensitive)
      // Use manual lowercase conversion for portability
      for (char *p = buf; *p; ++p) {
         if (*p >= 'A' && *p <= 'Z') {
            *p = *p - 'A' + 'a';
         }
      }

      if (strcmp(buf, "y") == 0 || strcmp(buf, "yes") == 0) {
         return 1;
      }
      if (strcmp(buf, "n") == 0 || strcmp(buf, "no") == 0) {
         return 0;
      }

      ERROR("Invalid response \"%s\": please answer 'y' or 'n'\n", buf);
   }
}

static int file_is_empty(const char *filename)
{
   FILE *fp = fopen(filename, "rb");
   if (fp == NULL) {
      ERROR("failed to open file '%s'\n", filename);
      return -1;
   }

   if (fseek(fp, 0, SEEK_END) != 0) {
      ERROR("fseek failed on '%s'\n", filename);
      fclose(fp);
      return -1;
   }

   long size = ftell(fp);
   if (size == -1L) {
      ERROR("ftell failed on '%s'\n", filename);
      fclose(fp);
      return -1;
   }

   fclose(fp);
   return (size == 0) ? 1 : 0;
}

static int prepare_record(struct parsed_args *args, const int len)
{
   if (!args->wfile || file_is_empty(args->wfile) != 0)
      return 0;

   args->rec = record_load_last(args->wfile);
   if (!args->rec.valid) {
      ERROR("invalid record obtained from %s", args->wfile);
      return -1;
   }

   for (int i = 0; i < MAX_CHARSET_LEN; i++) {
      args->rec.weights[i] *= args->rec.scale;
   }

   const float err_pct = 100.0f * (float)args->rec.dist / (float)args->rec.len;
   printf("Previous accuracy = %.1f%%\n", err_pct);

   args->rec.speed2 *= (1.0f - PID_K * (err_pct / 100.0f - TARGET_ACCURACY));
   args->rec.len = len;
   return 0;
}

static char *alloc_and_generate(struct parsed_args *args)
{
   const int len = args->rec.len + 2;

   char *buf = malloc(len);
   if (!buf) {
      ERROR("out of memory");
      return NULL;
   }

   if (!args->wfile || file_is_empty(args->wfile) != 0)
      memset(args->rec.weights, 1, sizeof(args->rec.weights));

   if (gen_chars(buf, len, args->min_word, args->max_word, args->rec.weights,
                 NULL) != 0) {
      ERROR("gen_chars() failed");
      free(buf);
      return NULL;
   }

   return buf;
}

static char *get_user_input(size_t maxlen)
{
   if (maxlen < 2) {
      ERROR("maxlen too small");
      return NULL;
   }

   char *buf = malloc(maxlen);
   if (!buf) {
      ERROR("out of memory");
      return NULL;
   }

   if (fgets(buf, (int)maxlen, stdin) == NULL) {
      if (feof(stdin))
         ERROR("end of file reached unexpectedly");
      else if (ferror(stdin))
         ERROR("error reading from stdin");
      else
         ERROR("fgets returned NULL for unknown reason");
      free(buf);
      return NULL;
   }

   // If newline is not found, discard the rest of the line
   size_t len = strlen(buf);
   if (len > 0 && buf[len - 1] != '\n') {
      int ch;
      while ((ch = getchar()) != '\n' && ch != EOF) {
         /* discard */
      }
   } else {
      // Remove newline
      buf[len - 1] = '\0';
   }

   str_to_lower(buf);
   str_trim(buf);

   return buf;
}

int main(int argc, char **argv)
{
   // Parse command line arguments
   struct parsed_args args = parse_args(argc, argv);

   // Read weights and settings, if file given
   if (prepare_record(&args, atoi(argv[1])) < 0)
      return -1;

   // Prepare record and generate text buffer
   char *gen_buf = alloc_and_generate(&args);
   if (!gen_buf)
      return -1;

   printf("Using speed %.1f/%.1f wpm\r\n", args.rec.speed1, args.rec.speed2);
   printf("Received text? ");
   fflush(stdout);

   // Play Morse code audio of generated text
   if (cw_play(gen_buf, args.rec.speed1, args.rec.speed2, args.freq, args.amp,
            args.delay) < 0) {
      free(gen_buf);
      ERROR("Error: playback error\n");
      return -1;
   }

   // Read user input, compare and calculate accuracy
   char *user_buf = get_user_input(args.rec.len + 1);
   if (user_buf == NULL) {
      free(gen_buf);
      return -1;
   }

   struct record r0 = {0};
   args.rec.dist = lev_diff(&r0, gen_buf, user_buf);
   free(user_buf);

   printf("Expected text: %s\n", gen_buf);
   const float err_pct = 100.0f * (float)args.rec.dist / (float)args.rec.len;
   record_printout(&r0);
   printf("%d errors out of %d = %.1f%%\n", args.rec.dist, args.rec.len,
         err_pct);
   free(gen_buf);

   // Save updated weights, if file given
   if (args.wfile && ask_yes_no("Record this to the given weights file?")) {
      for (int i = 0; i < MAX_CHARSET_LEN; i++)
         args.rec.weights[i] += r0.weights[i];

      if (record_append(args.wfile, &args.rec) != 0) {
         ERROR("writing record to file: %s", args.wfile);
         return -1;
      }
   }

   return 0;
}

// end file morsefocus.c
