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
   return rand() / (RAND_MAX + 1.0f);
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
   int found[95] = {0};

   for (size_t i = 0; charset[i] != '\0'; ++i)
   {
      unsigned char ch = charset[i];
      if (ch >= 33 && ch <= 126)
         found[ch - 33] = 1;
   }

   for (int i = 0; i < 95; ++i)
   {
      if (weights[i] != 1.0f && !found[i])
      {
         fprintf(stderr,
                 "error: character '%c' (ASCII %d) has weight %f but "
                 "is not in charset\n", i + 33, i + 33, weights[i]);
         return -1;
      }
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

   if (weights && gen_check_weights(weights, charset) != 0)
      return -1;

   size_t charset_len = strlen(charset);
   if (charset_len == 0 || num_char < 2)
      return -1;

   float *cdf = NULL;
   if (weights) {
      float *tmp_weights = malloc(sizeof(float) * charset_len);
      if (!tmp_weights)
         return -1;

      float sum = 0.0f;
      for (size_t j = 0; j < charset_len; ++j) {
         unsigned char ch = charset[j];
         if (ch < 33 || ch > 126) {
            free(tmp_weights);
            return -1;
         }
         float w = weights[ch - 33];
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
      int wlen = min_word + rand() % (max_word - min_word + 1);
      if (wlen > (int)(num_char - 1 - written))
         wlen = (int)(num_char - 1 - written);

      for (int i = 0; i < wlen && written < num_char - 2; ++i) {
         char ch;
         if (!weights) {
            ch = charset[rand() % charset_len];
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
            ch = charset[lo];
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

