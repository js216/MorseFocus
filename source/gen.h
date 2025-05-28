/**
 * @file gen.h
 * @brief Random pattern generation functions.
 *
 * @author Jakob Kastelic
 */

#ifndef GEN_CHARS_H
#define GEN_CHARS_H

#include <stddef.h>

/**
 * @brief generate a random string of space-separated words with weights
 *
 * @param s         output buffer (must be large enough)
 * @param num_char  total number of characters to generate (excluding null)
 * @param min_word  minimum length of each word (>=1)
 * @param max_word  maximum length of each word (>=min_word)
 * @param weights   array of 95 floats weights for ASCII 33..127, or NULL
 * @param charset   string of characters to draw from, or NULL for default
 *
 * @return 0 on success, -1 on error
 */
int gen_chars(char *s, const size_t num_char, const int min_word,
              const int max_word, const float *warr, const char *charset);

#endif // GEN_CHARS_H

// end file gen.h

