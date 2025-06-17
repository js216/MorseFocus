/**
 * @file record.c
 * @brief Manipulation of practice records.
 *
 * @author Jakob Kastelic
 */

#include "record.h"
#include "debug.h"
#include "str.h"
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct record record_load_last(const char *filename)
{
   struct record rec;
   memset(&rec, 0, sizeof(rec));

   FILE *fp = fopen(filename, "r");
   if (!fp) {
      ERROR("cannot open file '%s'", filename);
      return rec;
   }

   char line[MAX_CSV_LEN];
   char last_line[MAX_CSV_LEN] = "";

   while (fgets(line, sizeof(line), fp)) {
      strncpy(last_line, line, sizeof(last_line) - 1);
      last_line[sizeof(last_line) - 1] = '\0'; // ensure null termination
   }

   if (fclose(fp) != 0) {
      ERROR("failed to close file");
      return rec;
   }

   if (last_line[0] == '\0') {
      ERROR("file '%s' is empty or unreadable", filename);
      return rec;
   }

   if (!strchr(last_line, '\n')) {
      ERROR("could not read entire line");
      return rec;
   }

   // start parsing the line
   char *token = NULL;
   char *saveptr = NULL;

   // tokenize for date and time
   char *date = str_tok(last_line, " \t\n", &saveptr);
   char *time = str_tok(NULL, " \t\n", &saveptr);

   if (!date || !time) {
      ERROR("missing date or time field");
      return rec;
   }

   char datetime_str[32];

   int ret = snprintf(datetime_str, sizeof(datetime_str), "%s %s", date, time);
   if (ret < 0 || (size_t)ret >= sizeof(datetime_str)) {
      ERROR("datetime string formatting failed or was truncated");
      return rec;
   }

   if (parse_datetime(&rec.datetime, datetime_str) != 0) {
      ERROR("cannot parse datetime '%s'", datetime_str);
      return rec;
   }

   // parse 5 floats: scale, speed1, speed2, len, dist
   char *floats[5];
   for (int i = 0; i < 5; ++i) {
      floats[i] = str_tok(NULL, " \t\n", &saveptr);
      if (!floats[i]) {
         ERROR("missing float field #%d", i + 1);
         return rec;
      }
   }

   rec.scale = strtof(floats[0], NULL);
   rec.speed1 = strtof(floats[1], NULL);
   rec.speed2 = strtof(floats[2], NULL);
   rec.dist = strtof(floats[3], NULL);
   rec.len = strtof(floats[4], NULL);

   // parse charset
   char *charset_str = str_tok(NULL, " \t\n", &saveptr);
   if (!charset_str) {
      ERROR("missing charset field");
      return rec;
   }

   size_t charset_len = strlen(charset_str);

   if (charset_len == 0 || charset_len > MAX_CHARSET_LEN) {
      ERROR("charset length must be 1–%d characters", MAX_CHARSET_LEN);
      return rec;
   }

   strncpy(rec.charset, charset_str, MAX_CHARSET_LEN);
   rec.charset[MAX_CHARSET_LEN - 1] = '\0';

   // parse weights
   size_t weight_count = 0;
   while (weight_count < MAX_CHARSET_LEN &&
          (token = str_tok(NULL, " \t\n", &saveptr)) != NULL) {
      rec.weights[weight_count++] = strtof(token, NULL);
   }

   if (weight_count == 0) {
      ERROR("at least one weight must be provided");
      return rec;
   }

   if (str_tok(NULL, " \t\n", &saveptr)) {
      ERROR("too many weights (max %d)", MAX_CHARSET_LEN);
      return rec;
   }

   rec.valid = 1;
   return rec;
}

static int is_int(const float f)
{
   if (fabsf(f) < 1e-30)
      return 1;
   return f == truncf(f);
}

int record_append(const char *path, const struct record *r)
{
   // check record is valid
   if (!(r->valid)) {
      ERROR("invalid record given");
      return -1;
   }

   // open output file for appending
   FILE *fp = fopen(path, "a");
   if (!fp) {
      ERROR("cannot open file '%s'", path);
      return -1;
   }

   // format timestamp
   char timestr[32];
   if (strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S", &r->datetime) ==
       0) {
      ERROR("cannot format datetime");
      if (fclose(fp) != 0) {
         ERROR("failed to close file");
         return -1;
      }
      return -1;
   }

   // write fixed fields
   int num_pr = fprintf(fp, "%s %.3f %.1f %.1f %.0f %.0f %s", timestr, r->scale,
                        r->speed1, r->speed2, r->dist, r->len, r->charset);
   if (num_pr < 0) {
      ERROR("cannot write to file");
      if (fclose(fp) != 0) {
         ERROR("failed to close file");
         return -1;
      }
      return -1;
   }

   // write weights
   for (int i = 0; i < MAX_CHARSET_LEN; ++i) {
      int ret = -1;
      if (is_int(r->weights[i]))
         ret = fprintf(fp, " %.0f", r->weights[i]);
      else
         ret = fprintf(fp, " %.3f", r->weights[i]);
      if (ret < 0) {
         ERROR("cannot write weights");
         if (fclose(fp) != 0) {
            ERROR("failed to close file");
            return -1;
         }
         return -1;
      }

      num_pr += ret;
      if (num_pr > MAX_CSV_LEN) {
         ERROR("wrote more than MAX_CSV_LEN");
         if (fclose(fp) != 0) {
            ERROR("failed to close file");
            return -1;
         }
         return -1;
      }
   }

   if (fputc('\n', fp) == EOF) {
      ERROR("failed to write newline to file");
   }

   if (fclose(fp) != 0) {
      ERROR("failed to close file");
      return -1;
   }
   return 0;
}

/**
 * @brief Check if a floating-point weight has no fractional part.
 *
 * This function determines whether the given float value represents
 * an integer (i.e., its fractional part is zero).
 *
 * @param weight The float value to check.
 * @return int Returns 1 if the fractional part of the weight is 0, otherwise 0.
 */
static int record_is_integer(const float weight)
{
   return floorf(weight) == weight;
}

void record_printout(const struct record *r)
{
   for (int i = 0; i < MAX_CHARSET_LEN; i++) {
      if (r->weights[i] > 0) {
         char ch = str_int_to_char(i);
         if (ch == '\0') {
            ERROR("invalid character number %d", i);
            return;
         }
         if (record_is_integer(r->weights[i]))
            printf("'%c' : %.0f\n", ch, r->weights[i]);
         else
            printf("'%c' : %.3f\n", ch, r->weights[i]);
      }
   }
}

int record_scale_weights(struct record *r)
{
   if (r == NULL) {
      ERROR("record does not exist");
      return -1;
   }

   if ((r->scale <= 0.01F) || (r->scale > 1.0F)) {
      ERROR("scale %.3e out of range", r->scale);
      return -1;
   }

   for (int i = 0; i < MAX_CHARSET_LEN; i++) {
      if (r->weights[i] < 0)
         r->weights[i] = 0;
      else
         r->weights[i] = powf(r->weights[i], r->scale);
   }

   return 0;
}

// end file record.c
