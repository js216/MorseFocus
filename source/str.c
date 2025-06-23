/**
 * @file str.c
 * @brief String utility functions.
 *
 * @author Jakob Kastelic
 */

#include "str.h"
#include "debug.h"
#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int str_clean(char *s1, const char *s2, size_t len)
{
   for (size_t i = 0; i < len; ++i) {
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
      if (fclose(f) != 0) {
         ERROR("failed to close file");
         return -1;
      }
      return -1;
   }

   long size = ftell(f);
   if (fclose(f) != 0) {
      ERROR("failed to close file");
      return -1;
   }

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
   if (fclose(f) != 0) {
      ERROR("failed to close file");
      return -1;
   }

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
   char *start = NULL;
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

static int parse_int(const char **p, int width, int *out)
{
   char buf[12]; // enough for any int substring
   strncpy(buf, *p, width);
   buf[width] = '\0';

   char *endptr = NULL;
   long val = strtol(buf, &endptr, 10);
   if (endptr != buf + width || val < INT_MIN || val > INT_MAX)
      return -1;

   *out = (int)val;
   *p += width;
   return 0;
}

int parse_datetime(struct tm *tm, const char *s)
{
   if (!tm || !s)
      return -1;

   memset(tm, 0, sizeof(*tm));
   const char *p = s;

   if (parse_int(&p, 4, &tm->tm_year) != 0 || *p != '-')
      return -1;
   p++; // skip '-'

   if (parse_int(&p, 2, &tm->tm_mon) != 0 || *p != '-')
      return -1;
   p++; // skip '-'

   if (parse_int(&p, 2, &tm->tm_mday) != 0 || *p != ' ')
      return -1;
   p++; // skip ' '

   if (parse_int(&p, 2, &tm->tm_hour) != 0 || *p != ':')
      return -1;
   p++; // skip ':'

   if (parse_int(&p, 2, &tm->tm_min) != 0 || *p != ':')
      return -1;
   p++; // skip ':'

   if (parse_int(&p, 2, &tm->tm_sec) != 0)
      return -1;

   if (*p != '\0') // must be end of string here
      return -1;

   if (tm->tm_mon < 1 || tm->tm_mon > 12)
      return -1;
   if (tm->tm_mday < 1 || tm->tm_mday > 31)
      return -1;
   if (tm->tm_hour < 0 || tm->tm_hour > 23)
      return -1;
   if (tm->tm_min < 0 || tm->tm_min > 59)
      return -1;
   if (tm->tm_sec < 0 || tm->tm_sec > 60) // leap second allowed
      return -1;

   tm->tm_year -= 1900;
   tm->tm_mon -= 1;

   return 0;
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

// end file str.c
