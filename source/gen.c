/**
 * @file gen.c
 * @brief Random pattern generation functions.
 *
 * @author Jakob Kastelic
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#include "str.h"

/**
 * @brief generate a pseudo-random float in the range [0, 1)
 *
 * this function seeds the random number generator on the first call,
 * then returns uniformly distributed floats from 0 (inclusive) up to
 * but not including 1.0.
 *
 * @return a float in [0, 1)
 */
static float gen_rand()
{
   static int seeded = 0;
   if (!seeded) {
      srand((unsigned)time(NULL));
      seeded = 1;
   }
   const float r = rand() / (RAND_MAX + 1.0f);
   return r;
}


/**
 * Check that all characters with non-unity weights exist in the charset.
 *
 * @param weights  Array of 95 weights for printable ASCII (33 to 126).
 * @param charset  String of characters to be used for generation.
 * @return         0 if all non-unity weights are in charset, -1 otherwise.
 */
static int gen_check_weights(const float *weights, const char *charset)
{
   int found[MAX_CHARS] = {0};

   for (size_t i = 0; charset[i] != '\0'; ++i)
   {
      unsigned char ch = charset[i];
      if (str_char_to_int(ch) > 0) {
         const int j = str_char_to_int(ch);
         if (j < 0) {
            fprintf(stderr, "error: character '%c' (ASCII %d) is invalid\n",
                  ch, ch);
            return -1;
         }
         found[j] = 1;
      }
   }

   for (int i = 0; i < MAX_CHARS; ++i)
   {
      if ((weights[i] != 0.0f) && (weights[i] != 1.0f) && !found[i])
      {
         char ch = str_int_to_char(i);
         if (ch == '\0')
            ch = ' ';

         fprintf(stderr,
               "error: character '%c' (ASCII %d) has weight %f but "
               "is not in charset\n", ch, ch,  weights[i]);
         return -1;
      }
   }

   return 0;
}


int gen_clean_charset(char *c1, const char* c2)
{
   int i = 0;
   while (c2[i] != '\0' && i < MAX_CHARS) {
      char ch = tolower((unsigned char)c2[i]);
      if (str_char_to_int(ch) < 0) {
         fprintf(stderr, "error: unsupported character: '%c'\n", c2[i]);
         return -1;
      }
      c1[i] = ch;
      i++;
   }
   if (i < MAX_CHARS) {
      c1[i] = '\0';
   } else {
      c1[MAX_CHARS - 1] = '\0';  // ensure null-termination
   }
   return 0;
}


int gen_chars(char *s, const size_t num_char,
      const int min_word, const int max_word,
      const float *weights, const char *charset)
{
   if (min_word < 1 || max_word < 1 || min_word > max_word)
   {
      fprintf(stderr, "error: invalid word size range: min=%d, max=%d\n",
            min_word, max_word);
      return 1;
   }

   const char *default_charset = "kmuresnaptlwi.jz=foy,vg5/q92h38b?47c1d60x";
   if (!charset)
      charset = default_charset;

   char clean_charset[MAX_CHARS];
   int ret = gen_clean_charset(clean_charset, charset);
   if (ret != 0) {
      fprintf(stderr, "error: failed to clean charset\n");
      return -1;
   }

   if (weights && gen_check_weights(weights, clean_charset) != 0)
      return -1;

   size_t charset_len = strlen(clean_charset);
   if (charset_len == 0 || num_char < 2)
      return -1;

   float *cdf = NULL;
   if (weights) {
      float *tmp_weights = malloc(sizeof(float) * charset_len);
      if (!tmp_weights)
         return -1;

      float sum = 0.0f;
      for (size_t j = 0; j < charset_len; ++j) {
         unsigned char ch = clean_charset[j];
         if (str_char_to_int(ch) < 0) {
            free(tmp_weights);
            return -1;
         }
         const int k = str_char_to_int(ch);
         if (k < 0) {
            fprintf(stderr, "error: character '%c' (ASCII %d) is invalid\n",
                  ch, ch);
            return -1;
         }
         float w = weights[k];
         tmp_weights[j] = w;
         sum += w;
      }

      if (sum == 0.0f) {
         free(tmp_weights);
         return -1;
      }

      cdf = malloc(sizeof(float) * charset_len);
      if (!cdf) {
         free(tmp_weights);
         return -1;
      }

      float accum = 0.0f;
      for (size_t j = 0; j < charset_len; ++j) {
         accum += tmp_weights[j];
         cdf[j] = accum / sum;
      }

      free(tmp_weights);
   }

   size_t written = 0;
   while (written < num_char - 1) {
      int wlen = min_word + (int)(gen_rand() * (max_word - min_word + 1));
      if (wlen > (int)(num_char - 1 - written))
         wlen = (int)(num_char - 1 - written);

      for (int i = 0; i < wlen && written < num_char - 2; ++i) {
         char ch;
         if (!weights) {
            int idx = (int)(gen_rand() * charset_len);
            ch = clean_charset[idx];
         } else {
            float r = gen_rand();
            size_t lo = 0, hi = charset_len - 1;
            while (lo < hi) {
               size_t mid = (lo + hi) / 2;
               if (cdf[mid] < r)
                  lo = mid + 1;
               else
                  hi = mid;
            }
            ch = clean_charset[lo];
         }
         s[written++] = ch;
      }

      if (written < num_char - 2)
         s[written++] = ' ';
      else
         break;
   }

   s[written] = '\0';

   if (cdf)
      free(cdf);
   return 0;
}


// end file gen.c

