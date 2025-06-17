/**
 * @file gen.c
 * @brief Random pattern generation functions.
 *
 * @author Jakob Kastelic
 */

#include "gen.h"
#include "debug.h"
#include "record.h"
#include "str.h"
#include "xorshift32.h"
#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static int check_max_limit(const char *name, int val)
{
   if (val > GEN_MAX) {
      ERROR("%s too large", name);
      return -1;
   }
   return 0;
}

static int validate_params(const size_t num_char, int min_word, int max_word)
{
   if (min_word < 1 || max_word < 1 || min_word > max_word) {
      ERROR("invalid word size range: min=%d, max=%d", min_word, max_word);
      return -1;
   }

   if (num_char < 2) {
      ERROR("refuse to generate < 2 characters");
      return -1;
   }

   if (num_char > GEN_MAX) {
      ERROR("too many chars requested");
      return -1;
   }

   if (check_max_limit("min word", min_word) != 0)
      return -1;

   if (check_max_limit("max word", max_word) != 0)
      return -1;

   return 0;
}

static int validate_charset_weights(const float *weights, const char *charset,
                                    float **out_tmp_weights, float *out_sum)
{
   size_t charset_len = strlen(charset);
   float *tmp_weights = malloc(sizeof(float) * charset_len);
   if (!tmp_weights) {
      ERROR("cannot allocate temp weights array");
      return -1;
   }

   float sum = 0.0F;
   for (size_t j = 0; j < charset_len; j++) {
      char ch = charset[j];
      int k = str_char_to_int(ch);
      if (k < 0) {
         ERROR("character '%c' (ASCII %d) is invalid", ch == '\0' ? ' ' : ch,
               ch);
         free(tmp_weights);
         return -1;
      }
      float w = weights[k];
      tmp_weights[j] = w;
      sum += w;
   }

   if (sum == 0.0F) {
      ERROR("weights sum to zero");
      free(tmp_weights);
      return -1;
   }

   *out_tmp_weights = tmp_weights;
   *out_sum = sum;
   return 0;
}

static int allocate_and_fill_cdf(float **cdf, const float *tmp_weights,
                                 size_t charset_len, float sum)
{
   *cdf = malloc(sizeof(float) * charset_len);
   if (!*cdf) {
      ERROR("cannot allocate cdf");
      return -1;
   }

   float accum = 0.0F;
   for (size_t j = 0; j < charset_len; j++) {
      accum += tmp_weights[j];
      (*cdf)[j] = accum / sum;
   }
   return 0;
}

static int prepare_cdf(float **cdf, const float *weights, const char *charset)
{
   float *tmp_weights = NULL;
   float sum = 0.0F;

   if (validate_charset_weights(weights, charset, &tmp_weights, &sum) != 0)
      return -1;

   size_t charset_len = strlen(charset);
   if (allocate_and_fill_cdf(cdf, tmp_weights, charset_len, sum) != 0) {
      free(tmp_weights);
      return -1;
   }

   free(tmp_weights);
   return 0;
}

/**
 * @brief Pick a random character from charset uniformly.
 */
static char pick_random_char(const char *charset)
{
   size_t charset_len = strlen(charset);
   if (charset_len > INT_MAX) {
      ERROR("charset too long");
      return -1;
   }

   int idx = (int)(xorshift32_rand_float() * (float)charset_len);

   if (idx >= (int)charset_len)
      idx = (int)charset_len - 1;
   return charset[idx];
}

/**
 * @brief Pick a weighted random character from charset using cdf.
 */
static char pick_weighted_char(const char *charset, const float *cdf)
{
   size_t charset_len = strlen(charset);
   float r = xorshift32_rand_float();
   size_t lo = 0;
   size_t hi = charset_len - 1;
   while (lo < hi) {
      size_t mid = (lo + hi) / 2;
      if (cdf[mid] < r)
         lo = mid + 1;
      else
         hi = mid;
   }
   return charset[lo];
}

int gen_chars(char *s, const size_t num_char, const int min_word,
              const int max_word, const float *weights, const char *charset)
{
   if (validate_params(num_char, min_word, max_word) != 0)
      return -1;

   const char *default_charset = "kmuresnaptlwi.jz=foy,vg5/q92h38b?47c1d60x";
   if (!charset)
      charset = default_charset;

   if (str_is_clean(charset) != 0) {
      ERROR("charset contains unsupported characters");
      return -1;
   }

   size_t charset_len = strlen(charset);
   if (charset_len == 0) {
      ERROR("empty charset");
      return -1;
   }

   float *cdf = NULL;
   if (weights) {
      if (prepare_cdf(&cdf, weights, charset) != 0)
         return -1;
   }

   size_t written = 0;

   while (written < num_char - 1) {
      const int range = max_word - min_word + 1;
      int wlen = min_word + (int)(xorshift32_rand_float() * (float)range);
      if (wlen > (int)(num_char - 1 - written))
         wlen = (int)(num_char - 1 - written);

      for (int i = 0; i < wlen && written < num_char - 2; i++) {
         char ch = '\0';
         if (!weights) {
            ch = pick_random_char(charset);
         } else {
            ch = pick_weighted_char(charset, cdf);
         }
         s[written++] = ch;
      }

      if (written < num_char - 2)
         s[written++] = ' ';
      else
         break;
   }

   s[written] = '\0';

   free(cdf);

   return 0;
}

void free_entries(struct WordEntry *entries, int count)
{
   for (int i = 0; i < count; ++i) {
      free(entries[i].word);
   }
   free(entries);
}

float compute_total_weight(struct WordEntry *entries, int count)
{
   float total = 0.0F;
   for (int i = 0; i < count; ++i) {
      total += entries[i].weight;
   }
   return total;
}

const char *select_random_word(struct WordEntry *entries, int count,
                               float total_weight)
{
   float r = xorshift32_rand_float() *
             (total_weight > 0.0F ? total_weight : (float)count);
   float accum = 0.0F;
   for (int i = 0; i < count; ++i) {
      float w = (total_weight > 0.0F) ? entries[i].weight : 1.0F;
      accum += w;
      if (r < accum) {
         return entries[i].word;
      }
   }
   return entries[count - 1].word; // fallback
}

int write_words(FILE *out, struct WordEntry *entries, int count, int nw,
                float total_weight)
{
   for (int i = 0; i < nw; ++i) {
      const char *word = select_random_word(entries, count, total_weight);
      if (fprintf(out, "%s", word) < 0)
         return -1;

      if (i < nw - 1) {
         if (fprintf(out, " ") < 0)
            return -1;
      }
   }

   if (fprintf(out, "\n") < 0)
      return -1;

   return 0;
}

int is_line_too_long(FILE *fp, char *line)
{
   // if no newline in buffer and not EOF, line too long
   return (strchr(line, '\n') == NULL && !feof(fp));
}

int validate_word(const char *word)
{
   for (int i = 0; word[i] != '\0'; ++i) {
      if (str_char_to_int(word[i]) < 0) {
         ERROR("invalid char found '%d' (ASCII '%c')", word[i],
               word[i] == '\0' ? ' ' : word[i]);
         return -1;
      }
   }
   return 0;
}

int parse_line(const char *line, char **word_out, float *weight_out,
               int *has_weight_out)
{
   char linecopy[MAX_WORD_LINE];
   strncpy(linecopy, line, sizeof(linecopy) - 1);
   linecopy[sizeof(linecopy) - 1] = '\0';

   char *word = strtok(linecopy, " ");
   char *weight_str = strtok(NULL, " ");

   if (!word) {
      ERROR("empty line");
      return -1;
   }

   if (weight_str) {
      if (*has_weight_out == 0) {
         ERROR("inconsistent weight presence, or space in word");
         return -1;
      }
      *has_weight_out = 1;

      char *endptr = NULL;
      float w = strtof(weight_str, &endptr);
      if (endptr == weight_str || *endptr != '\0') {
         ERROR("invalid weight format");
         return -1;
      }
      *weight_out = w;
   } else {
      if (*has_weight_out == 1) {
         ERROR("inconsistent weight presence");
         return -1;
      }
      *has_weight_out = 0;
      *weight_out = 0.0F;
   }

   *word_out = str_dup(word);
   if (!*word_out) {
      ERROR("memory allocation failed");
      return -1;
   }

   return 0;
}

int parse_word_file(const char *word_file, struct WordEntry **entries_out,
                    int nl)
{
   FILE *fp = stdin;
   if (word_file) {
      fp = fopen(word_file, "r");
      if (!fp) {
         ERROR("could not open word file");
         return -1;
      }
   }

   struct WordEntry *entries = NULL;
   int capacity = (nl > 0) ? nl : 16; // start small and grow
   entries = calloc(capacity, sizeof(struct WordEntry));
   if (!entries) {
      if (word_file) {
         if (fclose(fp) != 0) {
            ERROR("failed to close file");
            return -1;
         }
      }
      ERROR("memory allocation failed");
      return -1;
   }

   int count = 0;
   int has_weight = -1;
   char line[MAX_WORD_LINE];

   while (fgets(line, sizeof(line), fp)) {
      if (nl > 0 && count >= nl)
         break;

      if (is_line_too_long(fp, line)) {
         ERROR("line too long");
         goto fail;
      }

      line[strcspn(line, "\r\n")] = 0;

      char *word = NULL;
      float weight = 0.0F;
      if (parse_line(line, &word, &weight, &has_weight) < 0)
         goto fail;

      if (validate_word(word)) {
         ERROR("invalid character in word");
         goto fail;
      }

      if (count >= capacity) {
         capacity *= 2;
         struct WordEntry *new_entries =
             realloc(entries, capacity * sizeof(struct WordEntry));
         if (!new_entries) {
            ERROR("memory reallocation failed");
            goto fail;
         }
         entries = new_entries;
      }

      entries[count].word = str_dup(word);
      if (!entries[count].word) {
         ERROR("memory allocation failed for word");
         goto fail;
      }
      entries[count].weight = weight;
      count++;
   }

   if (word_file) {
      if (fclose(fp) != 0) {
         ERROR("failed to close file");
         free_entries(entries, count);
         return -1;
      }
   }

   if (nl > 0 && count < nl) {
      ERROR("not enough lines in file");
      free_entries(entries, count);
      *entries_out = NULL;
      return -1;
   }

   *entries_out = entries;
   return count;

fail:
   free_entries(entries, count);
   *entries_out = NULL;
   if (word_file) {
      if (fclose(fp) != 0) {
         ERROR("failed to close file");
         return -1;
      }
   }
   return -1;
}

int gen_words(const char *out_file, const char *word_file, const int nw,
              const int nl)
{
   struct WordEntry *entries = NULL;
   int count = parse_word_file(word_file, &entries, nl);
   if (count < 0) {
      ERROR("could not parse file");
      return -1;
   }

   float total_weight = compute_total_weight(entries, count);

   FILE *out = stdout;
   if (out_file) {
      out = fopen(out_file, "w");
      if (!out) {
         ERROR("could not open output file");
         free_entries(entries, count);
         return -1;
      }
   }

   int status = write_words(out, entries, count, nw, total_weight);

   free_entries(entries, count);

   if (out_file) {
      if (fclose(out) != 0) {
         ERROR("failed to close file");
         return -1;
      }
   }

   return status;
}

// end file gen.c
