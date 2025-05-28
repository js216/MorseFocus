/**
 * @file test_gen.c
 * @brief Test program for the text generation functions.
 *
 * @author Jakob Kastelic
 */

#include <stdio.h>
#include <string.h>
#include "gen.h"

#define NUM_CHARS 95

static const char *default_charset = "kmuresnaptlwi.jz=foy,vg5/q92h38b?47c1d60x";

int main(void)
{
   char buf[101];
   int ret;

   // test with default charset and uniform weights
   ret = gen_chars(buf, 75, 3, 6, NULL, NULL);
   printf("gen_chars default, uniform weights, returned %d\n", ret);
   printf("result: \"%s\"\n", buf);
   printf("length: %zu\n", strlen(buf));
   printf("\n");

   // test with custom charset and uniform weights
   ret = gen_chars(buf, 50, 2, 4, NULL, "abcde");
   printf("gen_chars custom charset, returned %d\n", ret);
   printf("result: \"%s\"\n", buf);
   printf("length: %zu\n", strlen(buf));
   printf("\n");

   // set nonzero weights only for chars in default_charset
   float weights[NUM_CHARS] = {0.0f};
   size_t charset_len = strlen(default_charset);
   for (size_t i = 0; i < charset_len; ++i)
   {
      unsigned char ch = default_charset[i];
      if (ch >= 33 && ch <= 126)
         weights[ch - 33] = 1.0f;
   }

   // test with weighted chars favoring '!' (ASCII 33) heavily
   weights[20] = 50.0f;

   ret = gen_chars(buf, 80, 4, 8, weights, NULL);
   printf("gen_chars weighted, returned %d\n", ret);
   printf("result: \"%s\"\n", buf);
   printf("length: %zu\n", strlen(buf));
   printf("\n");

   return 0;
}

// end file test_gen.c

