/**
 * @file str.c
 * @brief String utility functions.
 *
 * @author Jakob Kastelic
 */

#include <stdio.h>
#include <stdlib.h>
#include "str.h"

#include <ctype.h>

int str_clean(char *s1, const char *s2, size_t len)
{
   size_t i;
   for (i = 0; i < len; ++i)
   {
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

   buf[read_len] = '\0';  // ensure null termination
   return (int)read_len;
}

// end file str.c

