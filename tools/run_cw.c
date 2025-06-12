/**
 * @file run_cw.c
 * @brief Tool to play back a text file as Morse code.
 *
 * Usage:
 *
 * $ build/run_cw [options]
 *
 * Options:
 *
 *   -f <file>    Read input string from file (default stdin)
 *   -1 <speed1>  Speed1 in WPM (1..500), default 25
 *   -2 <speed2>  Speed2 in WPM (1..500), default 25
 *   -t <freq>    Tone frequency Hz (60..10000), default 700
 *   -a <amp>     Amplitude (0..1), default 0.3
 *   -d <delay>   Initial delay seconds (0..60), default 1
 *
 * @author Jakob Kastelic
 */

#include "cw.h"
#include "debug.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int silence_errors;

extern int cw_play(const char *str, float speed1, float speed2, float freq,
                   float amp, float delay);

static void print_usage(const char *progname)
{
   fprintf(stderr,
           "Usage: %s [options]\n"
           "  -f <file>    Read input string from file (default stdin)\n"
           "  -1 <speed1>  Speed1 in WPM (1..500), default 25\n"
           "  -2 <speed2>  Speed2 in WPM (1..500), default 25\n"
           "  -t <freq>    Tone frequency Hz (60..10000), default 700\n"
           "  -a <amp>     Amplitude (0..1), default 0.3\n"
           "  -d <delay>   Initial delay seconds (0..60), default 1\n",
           progname);
}

static int check_float_range(float val, float min, float max, const char *name)
{
   if (val < min || val > max) {
      ERROR("Error: %s must be between %.2f and %.2f\n", name, min, max);
      return -1;
   }
   return 0;
}

static char *read_input_file(const char *filename)
{
   FILE *f = fopen(filename, "rb");
   if (!f) {
      ERROR("Error: cannot open file '%s': %s\n", filename, strerror(errno));
      return NULL;
   }
   if (fseek(f, 0, SEEK_END) != 0) {
      fclose(f);
      ERROR("Error: fseek failed on '%s'\n", filename);
      return NULL;
   }
   long size = ftell(f);
   if (size < 0) {
      fclose(f);
      ERROR("Error: ftell failed on '%s'\n", filename);
      return NULL;
   }
   rewind(f);

   char *buf = malloc(size + 1);
   if (!buf) {
      fclose(f);
      ERROR("Error: malloc failed\n");
      return NULL;
   }
   size_t read_size = fread(buf, 1, size, f);
   fclose(f);
   if ((long)read_size != size) {
      free(buf);
      ERROR("Error: fread failed on '%s'\n", filename);
      return NULL;
   }
   buf[size] = '\0';
   return buf;
}

static char *read_stdin(void)
{
   // Read stdin until EOF into dynamically allocated buffer
   size_t capacity = 4096;
   size_t len = 0;
   char *buf = malloc(capacity);
   if (!buf) {
      ERROR("Error: malloc failed\n");
      return NULL;
   }

   int c;
   while ((c = getchar()) != EOF) {
      if (len + 1 >= capacity) {
         capacity *= 2;
         char *tmp = realloc(buf, capacity);
         if (!tmp) {
            free(buf);
            ERROR("Error: realloc failed\n");
            return NULL;
         }
         buf = tmp;
      }
      buf[len++] = (char)c;
   }
   buf[len] = '\0';
   return buf;
}

int main(int argc, char **argv)
{
   const char *filename = NULL;
   float speed1 = 25.0f;
   float speed2 = 25.0f;
   float freq = 700.0f;
   float amp = 0.3f;
   float delay = 1.0f;

   for (int i = 1; i < argc; i++) {
      if (strcmp(argv[i], "-f") == 0) {
         if (i + 1 >= argc) {
            ERROR("Error: -f requires a filename\n");
            print_usage(argv[0]);
            return -1;
         }
         filename = argv[++i];
      } else if (strcmp(argv[i], "-1") == 0) {
         if (i + 1 >= argc) {
            ERROR("Error: -1 requires a speed value\n");
            print_usage(argv[0]);
            return -1;
         }
         speed1 = atof(argv[++i]);
         if (check_float_range(speed1, 1.0f, 500.0f, "speed1") < 0)
            return -1;
      } else if (strcmp(argv[i], "-2") == 0) {
         if (i + 1 >= argc) {
            ERROR("Error: -2 requires a speed value\n");
            print_usage(argv[0]);
            return -1;
         }
         speed2 = atof(argv[++i]);
         if (check_float_range(speed2, 1.0f, 500.0f, "speed2") < 0)
            return -1;
      } else if (strcmp(argv[i], "-t") == 0) {
         if (i + 1 >= argc) {
            ERROR("Error: -t requires a frequency value\n");
            print_usage(argv[0]);
            return -1;
         }
         freq = atof(argv[++i]);
         if (check_float_range(freq, 60.0f, 10000.0f, "frequency") < 0)
            return -1;
      } else if (strcmp(argv[i], "-a") == 0) {
         if (i + 1 >= argc) {
            ERROR("Error: -a requires an amplitude value\n");
            print_usage(argv[0]);
            return -1;
         }
         amp = atof(argv[++i]);
         if (check_float_range(amp, 0.0f, 1.0f, "amplitude") < 0)
            return -1;
      } else if (strcmp(argv[i], "-d") == 0) {
         if (i + 1 >= argc) {
            ERROR("Error: -d requires a delay value\n");
            print_usage(argv[0]);
            return -1;
         }
         delay = atof(argv[++i]);
         if (check_float_range(delay, 0.0f, 60.0f, "delay") < 0)
            return -1;
      } else {
         ERROR("Error: Unknown argument '%s'\n", argv[i]);
         print_usage(argv[0]);
         return -1;
      }
   }

   char *input_str = NULL;
   if (filename) {
      input_str = read_input_file(filename);
      if (!input_str)
         return -1;
   } else {
      input_str = read_stdin();
      if (!input_str)
         return -1;
   }

   int ret = cw_play(input_str, speed1, speed2, freq, amp, delay);
   if (ret < 0)
      ERROR("Error: playback error\n");

   free(input_str);
   return ret;
}

// end file run_cw.c
