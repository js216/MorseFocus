/**
 * @file test_gen.c
 * @brief Test program for the text generation functions.
 *
 * @author Jakob Kastelic
 */

#include "debug.h"
#include "gen.h"
#include "record.h"
#include "str.h"
#include <ctype.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEST_MAX_WORDS 1000
#define TEST_EPS 0.25

extern int silence_errors;

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
                      "(%.6f vs %.6f)",
                      i, str_int_to_char(i), v1, v2);
            return -1;
         }
      } else {
         float diff = fabsf(v1 - v2);
         if (diff > ep * avg) {
            TEST_FAIL("index %d (ASCII '%c'): diff %.2f%% exceeds "
                      "%.2f%% (%.6f vs %.6f)",
                      i, str_int_to_char(i), 100 * diff / avg, 100 * ep, v1,
                      v2);
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
static int test_gen_analyze(const char *s, const float *f, const int min_word,
                            const int max_word)
{
   float fs[MAX_CHARSET_LEN];
   test_gen_freq(fs, s);
   if (test_gen_check_freq(f, fs, TEST_EPS) != 0) {
      ERROR("frequency mismatch");
      return -1;
   }
   if (test_gen_check_word_len(s, min_word, max_word) != 0) {
      ERROR("word length mismatch");
      return -1;
   }
   return 0;
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
      TEST_FAIL("charset too long (max %d characters)", MAX_CHARSET_LEN - 1);
      return -1;
   }

   for (int i = 0; i < MAX_CHARSET_LEN; i++)
      w[i] = 0.0f;

   for (size_t i = 0; i < len; i++) {
      int offset = str_char_to_int(charset[i]);

      if (offset == -1 || offset >= MAX_CHARSET_LEN) {
         TEST_FAIL("invalid character '%c' in charset", charset[i]);
         return -1;
      }

      w[offset] = 1.000f;
   }

   return 0;
}

int test_gen_chars(void)
{
   float weights[MAX_CHARSET_LEN];
   char buf[GEN_MAX];

   // make sure charset_def is not too long
   if (strlen(charset_def) > MAX_CHARSET_LEN) {
      TEST_FAIL("charset_def is too long (longer than MAX_CHARSET_LEN)");
      return -1;
   }

   // test with default charset and uniform weights
   if ((test_gen_create_weights(weights, charset_def) != 0) ||
       (gen_chars(buf, GEN_MAX, 3, 6, NULL, NULL) != 0) ||
       (test_gen_analyze(buf, weights, 3, 6) != 0)) {
      TEST_FAIL("test with default charset and uniform weights");
      return -1;
   }

   // uniform weights, except favor '?' heavily
   weights[str_char_to_int('?')] = 50;
   if ((gen_chars(buf, GEN_MAX, 4, 8, weights, NULL) != 0) ||
       (test_gen_analyze(buf, weights, 4, 8) != 0)) {
      TEST_FAIL("uniform weights, except favor '?' heavily");
      return -1;
   }

   // test with custom charset and uniform weights
   if ((test_gen_create_weights(weights, "abcde") != 0) ||
       (gen_chars(buf, GEN_MAX, 2, 4, NULL, "abcde") != 0) ||
       (test_gen_analyze(buf, weights, 2, 4) != 0)) {
      TEST_FAIL("test with custom charset and uniform weights");
      return -1;
   }

   TEST_SUCCESS();
   return 0;
}

int test_free_entries(void)
{
   struct WordEntry *entries = malloc(2 * sizeof(struct WordEntry));
   if (!entries) {
      TEST_FAIL("malloc failed");
      return -1;
   }

   entries[0].word = str_dup("hello");
   entries[1].word = str_dup("world");
   free_entries(entries, 2);

   // if valgrind or asan shows no leak, it's passed
   TEST_SUCCESS();
   return 0;
}

int test_compute_total_weight(void)
{
   struct WordEntry entries[3] = {{"a", 1.0}, {"b", 2.5}, {"c", 0.5}};
   float total = compute_total_weight(entries, 3);
   if (total < 3.99 || total > 4.01) {
      TEST_FAIL("total weight incorrect");
      return -1;
   }
   TEST_SUCCESS();
   return 0;
}

int test_select_random_word(void)
{
   struct WordEntry entries[3] = {{"zero", 0.0f}, {"one", 1.0f}, {"two", 2.0f}};
   float total = compute_total_weight(entries, 3);

   if (total < 2.99f || total > 3.01f) {
      TEST_FAIL("unexpected total weight");
      return -1;
   }

   int count_zero = 0, count_one = 0, count_two = 0;
   const int trials = 10000;

   for (int i = 0; i < trials; ++i) {
      const char *word = select_random_word(entries, 3, total);
      if (strcmp(word, "zero") == 0)
         count_zero++;
      else if (strcmp(word, "one") == 0)
         count_one++;
      else if (strcmp(word, "two") == 0)
         count_two++;
      else {
         TEST_FAIL("unexpected word selected");
         return -1;
      }
   }

   if (count_zero > 0) {
      TEST_FAIL("zero-weight word was selected");
      return -1;
   }

   if (count_one == 0 || count_two == 0) {
      TEST_FAIL("non-zero weight words never selected");
      return -1;
   }

   float ratio = (float)count_two / (float)count_one;
   if (ratio < 1.8f || ratio > 2.2f) {
      TEST_FAIL("selection ratio not approximately 2:1");
      return -1;
   }

   TEST_SUCCESS();
   return 0;
}

int test_write_words(const char *test_file)
{
   struct WordEntry entries[3] = {
       {"alpha", 1.0f}, {"beta", 1.0f}, {"gamma", 1.0f}};

   int nw = 5;
   float total_weight = compute_total_weight(entries, 3);

   FILE *fp = fopen(test_file, "w+");
   if (!fp) {
      TEST_FAIL("fopen failed");
      return -1;
   }

   if (write_words(fp, entries, 3, nw, total_weight) != 0) {
      TEST_FAIL("write_words returned failure");
      fclose(fp);
      return -1;
   }

   // rewind
   if (fseek(fp, 0, SEEK_SET) != 0) {
      TEST_FAIL("fseek failed");
      fclose(fp);
      return -1;
   }

   // rewind and read output
   char buffer[256] = {0};
   if (!fgets(buffer, sizeof(buffer), fp)) {
      fclose(fp);
      TEST_FAIL("failed to read back output");
      return -1;
   }

   fclose(fp);

   // count words in output
   int word_count = 0;
   char *tok = strtok(buffer, " \n");
   while (tok) {
      if (strcmp(tok, "alpha") && strcmp(tok, "beta") && strcmp(tok, "gamma")) {
         TEST_FAIL("unexpected word in output");
         return -1;
      }
      word_count++;
      tok = strtok(NULL, " \n");
   }

   if (word_count != nw) {
      TEST_FAIL("unexpected number of words in output");
      return -1;
   }

   TEST_SUCCESS();
   return 0;
}

int test_is_line_too_long(const char *test_file)
{
   char line[8];
   FILE *fp = fopen(test_file, "w+");
   if (!fp) {
      TEST_FAIL("fopen failed");
      return -1;
   }

   fputs("short\nlonglonglonglong\n", fp);

   if (fseek(fp, 0, SEEK_SET) != 0) {
      TEST_FAIL("fseek failed");
      fclose(fp);
      return -1;
   }

   fgets(line, sizeof(line), fp);
   if (is_line_too_long(fp, line)) {
      fclose(fp);
      TEST_FAIL("misdetected short line");
      return -1;
   }

   fgets(line, sizeof(line), fp);
   if (!is_line_too_long(fp, line)) {
      fclose(fp);
      TEST_FAIL("did not detect long line");
      return -1;
   }

   fclose(fp);
   TEST_SUCCESS();
   return 0;
}

int test_validate_word(void)
{
   if (validate_word("validword23490???")) {
      TEST_FAIL("valid word rejected");
      return -1;
   }

   // testing malformed input
   silence_errors = 1;

   if (!validate_word("inv@lid")) {
      TEST_FAIL("invalid word accepted");
      silence_errors = 0;
      return -1;
   }

   // resume printing errors
   silence_errors = 0;

   TEST_SUCCESS();
   return 0;
}

int test_parse_line(void)
{
   char *word;
   float w;
   int hw = -1;

   char line1[] = "testword 1.5"; // writable buffer
   if (parse_line(line1, &word, &w, &hw) != 0) {
      TEST_FAIL("parse_line failed on valid input");
      return -1;
   }

   if (strcmp(word, "testword") != 0 || w != 1.5f || hw != 1) {
      free(word);
      TEST_FAIL("parsed data incorrect");
      return -1;
   }

   free(word);

   hw = -1;
   char line2[] = "simpleword"; // writable buffer
   if (parse_line(line2, &word, &w, &hw) != 0) {
      TEST_FAIL("parse_line failed on simple word");
      return -1;
   }

   if (strcmp(word, "simpleword") != 0 || w != 0.0f || hw != 0) {
      free(word);
      TEST_FAIL("parsed simple word incorrect");
      return -1;
   }

   free(word);

   // testing malformed input
   silence_errors = 1;

   char line3[] = "bad line 1.0 2.0"; // writable buffer
   if (parse_line(line3, &word, &w, &hw) == 0) {
      free(word);
      TEST_FAIL("accepted malformed input");
      silence_errors = 0;
      return -1;
   }

   // resume printing errors
   silence_errors = 0;

   TEST_SUCCESS();
   return 0;
}

int test_parse_word_file(const char *test_file)
{
   FILE *f = fopen(test_file, "w");
   if (!f) {
      TEST_FAIL("could not create test file");
      return -1;
   }

   fputs("apple 1.0\nbanana 2.0\ncherry 0.5\n", f);
   fclose(f);

   struct WordEntry *entries = NULL;
   int count = parse_word_file(test_file, &entries, 3);
   remove(test_file);
   if (count != 3) {
      TEST_FAIL("wrong count");
      return -1;
   }

   if (!entries) {
      TEST_FAIL("entries NULL");
      return -1;
   }

   if (strcmp(entries[0].word, "apple") != 0 ||
       strcmp(entries[1].word, "banana") != 0 ||
       strcmp(entries[2].word, "cherry") != 0) {
      TEST_FAIL("wrong word data");
      return -1;
   }

   free_entries(entries, count);
   TEST_SUCCESS();
   return 0;
}

int test_gen_words(const char *tf1, const char *tf2, const char *tf3)
{
   char valid_word_file[MAX_FILENAME_LEN];
   char nonexistent_file[MAX_FILENAME_LEN];
   char temp_out_file[MAX_FILENAME_LEN];

   // check input lengths and copy with null termination
   if (strlen(tf1) >= MAX_FILENAME_LEN || strlen(tf2) >= MAX_FILENAME_LEN ||
       strlen(tf3) >= MAX_FILENAME_LEN) {
      TEST_FAIL("one or more test filenames too long");
      return -1;
   }

   strncpy(valid_word_file, tf1, MAX_FILENAME_LEN);
   valid_word_file[MAX_FILENAME_LEN - 1] = '\0';

   strncpy(nonexistent_file, tf2, MAX_FILENAME_LEN);
   nonexistent_file[MAX_FILENAME_LEN - 1] = '\0';

   strncpy(temp_out_file, tf3, MAX_FILENAME_LEN);
   temp_out_file[MAX_FILENAME_LEN - 1] = '\0';

   // prepare a valid word file with 3 words (one with weight)
   FILE *f = NULL;
   f = fopen(valid_word_file, "w");
   if (!f) {
      TEST_FAIL("Failed to create test word file");
      return -1;
   }
   fprintf(f, "alpha 0.5\nbeta 1.0\ngamma 0.0\n");
   fclose(f);

   // case 1: valid file, output to file (should succeed)
   int ret;
   ret = gen_words(temp_out_file, valid_word_file, TEST_MAX_WORDS, 3);
   if (ret != 0) {
      TEST_FAIL("gen_words failed on valid file with file output");
      return -1;
   }
   // check that output file exists and is non-empty
   f = fopen(temp_out_file, "r");
   if (!f) {
      TEST_FAIL("Output file not created");
      return -1;
   }
   char buffer[256];
   if (!fgets(buffer, sizeof(buffer), f)) {
      fclose(f);
      TEST_FAIL("Output file is empty");
      return -1;
   }
   fclose(f);

   // testing malformed input
   silence_errors = 1;

   // case 2: non-existent word file (should fail)
   ret = gen_words(NULL, nonexistent_file, 2, 2);
   if (ret == 0) {
      TEST_FAIL("gen_words succeeded with nonexistent word file");
      silence_errors = 0;
      return -1;
   }

   // case 3: nl > number of lines in file (should fail)
   ret = gen_words(NULL, valid_word_file, 2, 10);
   if (ret == 0) {
      TEST_FAIL("gen_words succeeded with nl > lines in file");
      silence_errors = 0;
      return -1;
   }

   // resume printing errors
   silence_errors = 0;

   // cleanup test files
   remove(valid_word_file);
   remove(temp_out_file);

   TEST_SUCCESS();
   return 0;
}

// end file test_gen.c
