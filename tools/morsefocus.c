/**
 * @file morsefocus.c
 * @brief Generate weighted random pattern.
 *
 * morsefocus generates a random string of words, plays them in audio form as
 * Morse code, waits for the user input, and calculates the character accuracy
 * statistics.
 *
 * @author Jakob Kastelic
 */

#include "cw.h"
#include "debug.h"
#include "diff.h"
#include "gen.h"
#include "record.h"
#include "str.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_DIFF_LEN 8192
#define SEC_PER_MIN 60.0F
#define TARGET_ACCURACY 0.1F
#define PID_K 1.0F
#define PROMPT_BUF_SIZE 16

struct ParsedArgs {
   float min_word;
   float max_word;
   float freq;
   float amp;
   float delay;
   const char *file_name;
   struct record rec;
};

struct ArgDef {
   const char *flag;
   const char *name;
   float min_val;
   float max_val;
   void *target;
};

int silence_errors;
static struct ParsedArgs args;

static const struct ParsedArgs default_args = {
    .min_word = 2.0F,
    .max_word = 7.0F,
    .freq = 700.0F,
    .amp = 0.3F,
    .delay = 1.0F,
    .file_name = NULL,
    .rec = {.len = 250.0F, .speed1 = 25.0F, .speed2 = 25.0F, .scale = 1.0F},
};

static const struct ArgDef arg_defs[] = {
    {"-n", "length", 1.0F, 1000.0F, &args.rec.len},
    {"-s", "scale", 0.001F, 1.0F, &args.rec.scale},
    {"-1", "speed1", 1.0F, 500.0F, &args.rec.speed1},
    {"-2", "speed2", 1.0F, 500.0F, &args.rec.speed2},
    {"-i", "min word", 1.0F, 1000.0F, &args.min_word},
    {"-x", "max word", 1.0F, 1000.0F, &args.max_word},
    {"-f", "frequency", 60.0F, 10000.0F, &args.freq},
    {"-a", "amplitude", 0.0F, 1.0F, &args.amp},
    {"-w", "delay", 0.0F, 60.0F, &args.delay}};

static const char *usage =
    "Usage: %s file_name [options]\n\n"
    "Options:\n"
    "  -n <num>     number of characters to generate (default: 250)\n"
    "  -d <scale>   scale weights (default: 1.0)\n"
    "  -1 <speed>   Character speed in WPM (1..500), default 25\n"
    "  -2 <speed>   Farnsworth in WPM (1..500), default 25\n"
    "  -i <min>     set minimum word length (default 2)\n"
    "  -x <max>     set maximum word length (default 7)\n"
    "  -f <freq>    Tone frequency Hz (60..10000), default 700\n"
    "  -a <amp>     Amplitude (0..1), default 0.3\n"
    "  -w <wait>    Initial delay seconds (0..60), default 1\n";

static int check_float_range(float val, float min, float max, const char *name)
{
   if (val < min || val > max) {
      ERROR("error: %s must be between %.2f and %.2f\n", name, min, max);
      return -1;
   }
   return 0;
}

/**
 * @brief Check whether a file exists and contains data.
 *
 * @param filename Path to the file to check.
 * @return -1 on error (e.g. NULL filename, I/O failure),
 *          1 if the file exists and has content,
 *          0 if the file is missing or empty.
 */
static int file_has_content(const char *filename)
{
   if (filename == NULL) {
      ERROR("no filename given");
      return -1;
   }

   FILE *fp = fopen(filename, "rb");
   if (fp == NULL)
      return 0; // file missing or inaccessible → treat as no content

   if (fseek(fp, 0, SEEK_END) != 0) {
      if (fclose(fp) != 0) {
         ERROR("failed to close file");
         return -1;
      }
      ERROR("fseek failed on '%s'\n", filename);
      return -1;
   }

   long size = ftell(fp);
   if (size == -1L) {
      if (fclose(fp) != 0) {
         ERROR("failed to close file");
         return -1;
      }
      ERROR("ftell failed on '%s'\n", filename);
      return -1;
   }

   if (fclose(fp) != 0) {
      ERROR("failed to close file");
      return -1;
   }
   return (size > 0) ? 1 : 0;
}

static int ask_yes_no(const char *prompt)
{
   char buf[PROMPT_BUF_SIZE];

   while (1) {
      printf("%s (Y/n): ", prompt);

      if (fgets(buf, sizeof(buf), stdin) == NULL) {
         if (feof(stdin)) {
            ERROR("input error: End of file detected\n");
         } else if (ferror(stdin)) {
            ERROR("input error: I/O error detected\n");
         } else {
            ERROR("input error: Unknown error\n");
         }
         return -1;
      }

      // Newline means yes
      if (buf[0] == '\n')
         return 1;

      // Remove trailing newline if present
      buf[strcspn(buf, "\n")] = '\0';

      // Check for valid input (case-insensitive)
      // Use manual lowercase conversion for portability
      for (char *p = buf; *p; p++) {
         if (*p >= 'A' && *p <= 'Z') {
            *p = (char)(*p - 'A' + 'a');
         }
      }

      if (strcmp(buf, "y") == 0 || strcmp(buf, "yes") == 0) {
         return 1;
      }
      if (strcmp(buf, "n") == 0 || strcmp(buf, "no") == 0) {
         return 0;
      }

      ERROR("invalid response \"%s\": please answer 'y' or 'n'\n", buf);
   }
}

static int parse_args(int argc, char **argv)
{
   // assign defaults
   args = default_args;

   // mandatory positional filename argument
   if (argc < 2) {
      if (fprintf(stderr, usage, argv[0]) < 0) {
         ERROR("fprintf failed");
         exit(-1);
      }
      exit(-1);
   }
   args.file_name = argv[1];

   // read values from file, if it has any content
   if (file_has_content(args.file_name)) {
      args.rec = record_load_last(args.file_name);
      if (!args.rec.valid) {
         ERROR("invalid record obtained from %s", args.file_name);
         return -1;
      }

      const float err_pct = 100.0F * args.rec.dist / args.rec.len;
      args.rec.speed2 *= (1.0F - PID_K * (err_pct / 100.0F - TARGET_ACCURACY));
   }

   // fixed fields
   strcpy(args.rec.charset, "~");
   time_t now = time(NULL);
   args.rec.datetime = *localtime(&now);

   // optional flagged arguments
   for (int i = 2; i < argc; i++) {
      const char *arg = argv[i];
      const struct ArgDef *def = NULL;

      // every optional argument needs a value
      if (i++ >= argc) {
         ERROR("missing value for argument %s\n", arg);
         exit(-1);
      }

      // find the flag in arg_defs
      const size_t num_args = sizeof(arg_defs) / sizeof(arg_defs[0]);
      for (size_t j = 0; j < num_args; j++) {
         if (strcmp(arg, arg_defs[j].flag) == 0) {
            def = &arg_defs[j];
            break;
         }
      }

      // flag found?
      if (def == NULL) {
         ERROR("unrecognized option: %s\n", arg);
         if (fprintf(stderr, usage, argv[0]) < 0) {
            ERROR("fprintf failed");
            exit(-1);
         }
         exit(-1);
      }

      // set argument value
      const float val = strtof(argv[i], NULL);
      *(float *)def->target = val;
      if (check_float_range(val, def->min_val, def->max_val, def->name) < 0)
         exit(-1);
   }

   if (args.rec.speed1 < args.rec.speed2) {
      ERROR("speed1 must be equal or greater than speed2\n");
      exit(-1);
   }

   if (record_scale_weights(&args.rec) < 0)
      return -1;

   args.rec.valid = 1;
   return 0;
}

static char *alloc_and_generate(void)
{
   const int len = (int)(args.rec.len + 2);

   char *buf = malloc(len);
   if (!buf) {
      ERROR("out of memory");
      return NULL;
   }

   if (!file_has_content(args.file_name))
      for (int i = 0; i < MAX_CHARSET_LEN; i++)
         args.rec.weights[i] = 1;

   if (gen_chars(buf, len, (int)args.min_word, (int)args.max_word,
                 args.rec.weights, NULL) != 0) {
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
      int ch = '\0';
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
   if (parse_args(argc, argv) < 0)
      return -1;

   // Generate random characters
   char *gen_buf = alloc_and_generate();
   if (!gen_buf)
      return -1;

   // Initial stats
   const float secs = cw_duration(gen_buf, args.rec.speed1, args.rec.speed2);
   if (secs < 0)
      return -1;
   printf("Sending %.0f characters at %.1f/%.1f wpm (~%.1f min)\r\n",
          args.rec.len, args.rec.speed1, args.rec.speed2, secs / SEC_PER_MIN);
   printf("Received text? ");
   if (fflush(stdout) != 0) {
      ERROR("fflush failed");
      return -1;
   }

   // Arguments for the audio player
   struct cw_data cw = {
       .speed1 = args.rec.speed1,
       .speed2 = args.rec.speed2,
       .freq = args.freq,
       .amp = args.amp,
       .delay_sec = args.delay,
   };

   // Play Morse code audio of generated text
   if (cw_play(gen_buf, &cw) < 0) {
      free(gen_buf);
      ERROR("error: playback error\n");
      return -1;
   }

   // Read user input
   char *user_buf = get_user_input((size_t)(args.rec.len + 1));
   if (user_buf == NULL) {
      free(gen_buf);
      return -1;
   }

   // Compare and calculate accuracy
   struct record r0 = {0};
   args.rec.dist = (float)lev_diff(&r0, gen_buf, user_buf);
   free(user_buf);

   // Printout stats
   printf("Expected text: %s\n", gen_buf);
   const float err_pct = 100.0F * args.rec.dist / args.rec.len;
   record_printout(&r0);
   printf("%.0f errors out of %.0f = %.1f%%\n", args.rec.dist, args.rec.len,
          err_pct);
   free(gen_buf);

   // Save updated weights, if file given
   if (ask_yes_no("Record this to the given weights file?")) {
      for (int i = 0; i < MAX_CHARSET_LEN; i++)
         args.rec.weights[i] += r0.weights[i];

      if (record_append(args.file_name, &args.rec) != 0) {
         ERROR("writing record to file: %s", args.file_name);
         return -1;
      }
   }

   return 0;
}

// end file morsefocus.c
