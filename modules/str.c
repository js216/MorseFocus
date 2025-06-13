/**
 * @file str.c
 * @brief String utility functions.
 *
 * @author Jakob Kastelic
 */

#include "str.h"
#include "debug.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int str_clean(char *s1, const char *s2, size_t len)
{
   size_t i;
   for (i = 0; i < len; ++i) {
      unsigned char ch = (unsigned char)s2[i];
      if (ch >= 32 && ch <= 126)
         s1[i] = (char)tolower(ch);
      else
         s1[i] = ' ';
   }
   return 0;
}

int str_file_len(const char *fname)
{
   FILE *f = fopen(fname, "rb");
   if (!f)
      return -1;

   if (fseek(f, 0, SEEK_END) != 0) {
      fclose(f);
      return -1;
   }

   long size = ftell(f);
   fclose(f);

   if (size < 0)
      return -1;

   return (int)size;
}

int str_read_file(char *buf, const char *fname, const size_t max_len)
{
   if (!buf || !fname || max_len < 1)
      return -1;

   FILE *f = fopen(fname, "rb");
   if (!f)
      return -1;

   size_t read_len = fread(buf, 1, max_len - 1, f);
   fclose(f);

   buf[read_len] = '\0'; // ensure null termination
   return (int)read_len;
}

int str_char_to_int(const char ch)
{
   if (ch >= '0' && ch <= '9') {
      return ch - '0';
   }
   if (ch >= 'a' && ch <= 'z') {
      return 10 + (ch - 'a');
   }
   switch (ch) {
   case '.':
      return 36;
   case '=':
      return 37;
   case ',':
      return 38;
   case '/':
      return 39;
   case '?':
      return 40;
   case '\'':
      return 41;
   default:
      return -1;
   }
}

char str_int_to_char(const int i)
{
   static const char lookup[] = "0123456789abcdefghijklmnopqrstuvwxyz.=,/?'";

   if (i >= 0 && i < (int)(sizeof(lookup) - 1))
      return lookup[i];

   return '\0';
}

char *str_tok(char *str, const char *delim, char **saveptr)
{
   char *start;
   if (str)
      start = str;
   else if (*saveptr)
      start = *saveptr;
   else
      return NULL;

   // Skip leading delimiters
   start += strspn(start, delim);
   if (*start == '\0') {
      *saveptr = NULL;
      return NULL;
   }

   // Find end of token
   char *end = start + strcspn(start, delim);
   if (*end) {
      *end = '\0';
      *saveptr = end + 1;
   } else {
      *saveptr = NULL;
   }

   return start;
}

char *str_ptime(const char *s, const char *format, struct tm *tm)
{
   int year, mon, mday, hour, min, sec;

   // Only support exact format "%Y-%m-%d %H:%M:%S"
   if (format == NULL || strcmp(format, "%Y-%m-%d %H:%M:%S") != 0)
      return NULL;

   int ret = sscanf(s, "%4d-%2d-%2d %2d:%2d:%2d", &year, &mon, &mday, &hour,
                    &min, &sec);
   if (ret != 6)
      return NULL;

   tm->tm_year = year - 1900; // years since 1900
   tm->tm_mon = mon - 1;      // months since January [0-11]
   tm->tm_mday = mday;
   tm->tm_hour = hour;
   tm->tm_min = min;
   tm->tm_sec = sec;
   tm->tm_isdst = -1; // Not known

   // Return pointer after the matched datetime (fixed length = 19 chars)
   return (char *)(s + 19);
}

int str_is_clean(const char *s)
{
   for (int i = 0; i < MAX_CHARSET_LEN; i++) {
      if (s[i] == '\0')
         return 0;

      if (str_char_to_int(s[i]) < 0) {
         ERROR("invalid character %d (ASCII '%c')", s[i], s[i]);
         return -1;
      }
   }
   return 0;
}

void str_to_lower(char *s)
{
   if (!s)
      return;
   for (; *s; s++) {
      *s = (char)tolower((unsigned char)*s);
   }
}

void str_trim(char *s)
{
   if (!s)
      return;

   // Trim trailing whitespace
   size_t len = strlen(s);
   while (len > 0 && isspace((unsigned char)s[len - 1])) {
      s[--len] = '\0';
   }

   // Find start of non-whitespace
   char *start = s;
   while (*start && isspace((unsigned char)*start)) {
      start++;
   }

   // Shift trimmed string to the beginning if needed
   if (start != s) {
      memmove(s, start, strlen(start) + 1);
   }
}

char *str_dup(const char *s)
{
   if (!s)
      return NULL;

   size_t len = strlen(s) + 1;
   char *dup = (char *)malloc(len);
   if (dup)
      memcpy(dup, s, len);
   return dup;
}

int str_file_lines(const char *filename)
{
   FILE *f = fopen(filename, "r");
   if (!f) {
      ERROR("could not open file");
      return -1;
   }

   int lines = 0;
   int c;

   while ((c = fgetc(f)) != EOF) {
      if (c == '\n') {
         lines++;
      }
   }

   // If the file does not end with a newline but is not empty,
   // count the last line as well
   if (lines > 0) {
      fseek(f, -1, SEEK_END);
      c = fgetc(f);
      if (c != '\n') {
         lines++;
      }
   }

   fclose(f);
   return lines;
}

// end file str.c
