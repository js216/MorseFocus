/**
 * @file record.c
 * @brief Manipulation of practice records.
 *
 * @author Jakob Kastelic
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "str.h"
#include "record.h"

#define MAX_LINE_LEN 4096

struct record record_load_last(const char *filename)
{
   struct record rec;
   memset(&rec, 0, sizeof(rec));

   FILE *fp = fopen(filename, "r");
   if (!fp) {
      fprintf(stderr, "error: cannot open file '%s'\n", filename);
      return rec;
   }

   char line[MAX_LINE_LEN];
   char last_line[MAX_LINE_LEN] = "";

   while (fgets(line, sizeof(line), fp)) {
      strcpy(last_line, line);
   }

   fclose(fp);

   if (last_line[0] == '\0') {
      fprintf(stderr, "error: file '%s' is empty or unreadable.\n", filename);
      return rec;
   }

   if (!strchr(last_line, '\n')) {
      fprintf(stderr, "error: could not read entire line.\n");
      return rec;
   }

   // start parsing the line
   char *token = NULL;
   char *saveptr = NULL;

   // tokenize for date and time
   char *date = str_tok(last_line, " \t\n", &saveptr);
   char *time = str_tok(NULL, " \t\n", &saveptr);

   if (!date || !time) {
      fprintf(stderr, "error: missing date or time field.\n");
      return rec;
   }

   char datetime_str[32];
   snprintf(datetime_str, sizeof(datetime_str), "%s %s", date, time);

   if (!str_ptime(datetime_str, "%Y-%m-%d %H:%M:%S", &rec.datetime)) {
      fprintf(stderr, "error: cannot parse datetime '%s'.\n", datetime_str);
      return rec;
   }

   // parse 4 floats: decay, scale, speed1, speed2
   char *floats[4];
   for (int i = 0; i < 4; ++i) {
      floats[i] = str_tok(NULL, " \t\n", &saveptr);
      if (!floats[i]) {
         fprintf(stderr, "error: missing float field #%d.\n", i + 1);
         return rec;
      }
   }

   rec.decay = strtof(floats[0], NULL);
   rec.scale = strtof(floats[1], NULL);
   rec.speed1 = strtof(floats[2], NULL);
   rec.speed2 = strtof(floats[3], NULL);

   // parse charset
   char *charset_str = str_tok(NULL, " \t\n", &saveptr);
   if (!charset_str) {
      fprintf(stderr, "error: missing charset field.\n");
      return rec;
   }

   size_t charset_len = strlen(charset_str);

   if (charset_len == 0 || charset_len > MAX_CHARSET_LEN) {
      fprintf(stderr, "error: charset length must be 1–%d characters.\n",
            MAX_CHARSET_LEN);
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
      fprintf(stderr, "error: At least one weight must be provided.\n");
      return rec;
   }

   if (str_tok(NULL, " \t\n", &saveptr)) {
      fprintf(stderr, "error: Too many weights (max %d).\n", MAX_CHARSET_LEN);
      return rec;
   }

   rec.valid = 1;
   return rec;
}


int record_append(const char *path, const struct record *r)
{
   // check record is valid
   if (!(r->valid)) {
      fprintf(stderr, "error: invalid record given\n");
      return -1;
   }

   // open output file for appending
   FILE *fp = fopen(path, "a");
   if (!fp) {
      fprintf(stderr, "error: cannot open file '%s'\n", path);
      return -1;
   }

   // format timestamp
   char timestr[32];
   if (strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S",
            &r->datetime) == 0) {
      fprintf(stderr, "error: cannot format datetime\n");
      fclose(fp);
      return -1;
   }

   // write fixed fields
   int num_pr = fprintf(fp, "%s %.3f %.3f %.3f %.3f %s", timestr, r->decay,
         r->scale, r->speed1, r->speed2, r->charset);
   if (num_pr < 0) {
      fprintf(stderr, "error: cannot write to file\n");
      fclose(fp);
      return -1;
   }

   // write weights
   for (int i = 0; i < MAX_CHARSET_LEN; ++i) {
      const int ret = fprintf(fp, " %.3f", r->weights[i]);
      if (ret < 0) {
         fprintf(stderr, "error: cannot write weights\n");
         fclose(fp);
         return -1;
      }

      num_pr += ret;
      if (num_pr > MAX_LINE_LEN) {
         fprintf(stderr, "error: wrote more than MAX_LINE_LEN\n");
         fclose(fp);
         return -1;
      }
   }

   fputc('\n', fp);
   fclose(fp);
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
            fprintf(stderr, "error: invalid character number %d\n", i);
            return;
         }
         if (record_is_integer(r->weights[i]))
            printf("'%c' : %.0f\n", ch, r->weights[i]);
         else
            printf("'%c' : %.3f\n", ch, r->weights[i]);
      }
   }
}


// end file record.c

