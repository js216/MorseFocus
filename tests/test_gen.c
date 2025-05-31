/**
 * @file test_gen.c
 * @brief Test program for the text generation functions.
 *
 * @author Jakob Kastelic
 */

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include "debug.h"
#include "str.h"
#include "record.h"
#include "gen.h"

#define TEST_MAX_GEN_LEN 100000
#define TEST_EPS 0.1

static const char *charset_def = "kmuresnaptlwi.jz=foy,vg5/q92h38b?47c1d60x";

/**
 * @brief Generate character frequency table based on str_int_to_char mapping.
 *
 * This function fills the `freq` array with counts of how often each character
 * returned by `str_int_to_char(i)` appears in the string `s`, for each index
 * `i` from 0 to MAX_CHARSET_LEN - 1. If `str_int_to_char(i)` returns `\0`,
 * `freq[i]` is set to 0.
 *
 * @param freq Array to hold frequencies, must be MAX_CHARSET_LEN long.
 * @param s    Null-terminated string to analyze.
 * @return     0 on success.
 */
static void test_gen_freq(float *freq, const char *s)
{
   for (int i = 0; i < MAX_CHARSET_LEN; i++) {
      char ch = str_int_to_char(i);
      if (ch == '\0') {
         freq[i] = 0;
         continue;
      }

      int count = 0;
      for (const char *p = s; *p; ++p) {
         if (*p == ch) {
            count++;
         }
      }

      freq[i] = count;
   }
}


/**
 * @brief Compare two frequency tables after normalization.
 *
 * This function normalizes the two frequency tables `f1` and `f2` by dividing
 * each element by the sum of its array. It then compares the normalized values
 * at each index. If the values differ by more than `eps` times their average
 * (when the average is non-zero), or if one is zero and the other is not, it
 * prints an error and returns -1.
 *
 * @param f1   First frequency table (float values).
 * @param f2   Second frequency table (float values).
 * @param eps  Allowed relative difference.
 * @return     0 if frequencies are within tolerance, -1 otherwise.
 */
static int test_gen_check_freq(const float *f1, const float *f2, const float ep)
{
    float sum1 = 0.0f, sum2 = 0.0f;

    for (int i = 0; i < MAX_CHARSET_LEN; i++) {
        sum1 += f1[i];
        sum2 += f2[i];
    }

    for (int i = 0; i < MAX_CHARSET_LEN; i++) {
        float v1 = (sum1 > 0.0f) ? f1[i] / sum1 : 0.0f;
        float v2 = (sum2 > 0.0f) ? f2[i] / sum2 : 0.0f;
        float avg = (v1 + v2) / 2.0f;

        if (avg == 0.0f) {
            if (v1 != 0.0f || v2 != 0.0f) {
                TEST_FAIL("index %d (ASCII '%c'), one is zero, one is not "
                       "(%.6f vs %.6f)", i, str_int_to_char(i), v1, v2);
                return -1;
            }
        } else {
            float diff = fabsf(v1 - v2);
            if (diff > ep * avg) {
                TEST_FAIL("index %d (ASCII '%c'): diff %.2f%% exceeds "
                       "%.2f%% (%.6f vs %.6f)", i, str_int_to_char(i),
                       100*diff/avg, 100*ep, v1, v2);
                return -1;
            }
        }
    }

    return 0;
}


/**
 * @brief Checks that all words in the string are within given length bounds.
 *
 * A "word" is defined as a sequence of non-whitespace characters separated by
 * whitespace. The last word in the string is exempt from the test if string
 * doesn't end with whitespace.
 *
 * @param[in] s Null-terminated input string containing words.
 * @param[in] min_word Minimum allowed word length (inclusive).
 * @param[in] max_word Maximum allowed word length (inclusive).
 *
 * @return 0 if all words have length between min_word and max_word, inclusive;
 *         -1 if any word violates these bounds.
 */
static int test_gen_check_word_len(const char *s, const int min_word,
      const int max_word)
{
   int word_len = 0;

   if (!s || min_word > max_word || min_word < 0)
      return -1;

   while (*s) {
      if (!isspace((unsigned char)*s)) {
         // inside a word: count length
         word_len++;
      } else {
         // end of a word
         if (word_len > 0) {
            if (word_len < min_word || word_len > max_word)
               return -1;
            word_len = 0;
         }
      }
      s++;
   }

   return 0;
}


/**
 * @brief Verify character frequencies and word lengths in given string.
 * @param s String to analyze.
 * @param f Expected frequencies.
 * @param min_word minimum length of each word (>=1)
 * @param max_word maximum length of each word (>=min_word)
 * @return 0 on success, -1 on failure
 */
static int test_gen_analyze(const char *s, const float *f,
      const int min_word, const int max_word)
{
   float fs[MAX_CHARSET_LEN];
   test_gen_freq(fs, s);
   int ret = 0;
   ret |= test_gen_check_freq(f, fs, TEST_EPS);
   ret |= test_gen_check_word_len(s, min_word, max_word);
   return ret;
}


/**
 * @brief Initializes a weight array based on a given character set.
 *
 * This function sets all entries in the weight array `w` to 0.0, then
 * sets entries corresponding to characters in `charset` to 1.000.
 * The position in the array is determined by converting each character
 * using `str_char_to_int()`.
 *
 * If `charset` is too long (i.e., its length is greater than or equal to
 * MAX_CHARSET_LEN), or if `str_char_to_int()` returns -1 or a value
 * outside the valid range, the function prints an error message and
 * returns -1.
 *
 * @param[out] w Pointer to an array of floats with exactly MAX_CHARSET_LEN
 * elements. This array will be initialized by the function.
 * @param[in] charset Null-terminated string of characters to set weights for.
 *
 * @return 0 on success, -1 on error.
 */

int test_gen_create_weights(float *w, const char *charset)
{
   size_t len = strlen(charset);

   if (len >= MAX_CHARSET_LEN) {
      TEST_FAIL("charset too long (max %d characters)",
            MAX_CHARSET_LEN - 1);
      return -1;
   }

   for (int i = 0; i < MAX_CHARSET_LEN; i++)
      w[i] = 0.0f;

   for (size_t i = 0; i < len; i++) {
      int offset = str_char_to_int(charset[i]);

      if (offset == -1 || offset >= MAX_CHARSET_LEN) {
         TEST_FAIL("invalid character '%c' in charset",
               charset[i]);
         return -1;
      }

      w[offset] = 1.000f;
   }

   return 0;
}


int test_gen_chars(void)
{
   float weights[MAX_CHARSET_LEN];
   char buf[TEST_MAX_GEN_LEN];
   int ret = 0;

   // make sure charset_def is not too long
   if (strlen(charset_def) > MAX_CHARSET_LEN) {
      TEST_FAIL("charset_def is too long (longer than MAX_CHARSET_LEN)");
      return -1;
   }

   // test with default charset and uniform weights
   ret |= test_gen_create_weights(weights, charset_def);
   ret |= gen_chars(buf, TEST_MAX_GEN_LEN, 3, 6, NULL, NULL);
   ret |= test_gen_analyze(buf, weights, 3, 6);
   if (ret != 0) {
      TEST_FAIL("test with default charset and uniform weights");
      return -1;
   }

   // test with custom charset and uniform weights
   ret |= test_gen_create_weights(weights, "abcde");
   ret |= gen_chars(buf, TEST_MAX_GEN_LEN, 2, 4, NULL, "abcde");
   ret |= test_gen_analyze(buf, weights, 2, 4);
   if (ret != 0) {
      TEST_FAIL("test with custom charset and uniform weights");
      return -1;
   }

   // uniform weights, except favor '?' heavily
   ret |= test_gen_create_weights(weights, charset_def);
   weights[str_char_to_int('?')] = 50;
   ret |= gen_chars(buf, TEST_MAX_GEN_LEN, 4, 8, weights, NULL);
   ret |= test_gen_analyze(buf, weights, 4, 8);
   if (ret != 0) {
      TEST_FAIL("uniform weights, except favor '?' heavily");
      return -1;
   }

   TEST_SUCCESS();
   return 0;
}


// end file test_gen.c

