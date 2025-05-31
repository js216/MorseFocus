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
 * @param weights   array of at most NUM_WEIGHTS floats weights
 * @param charset   string of characters to draw from, or NULL for default
 *
 * @return 0 on success, -1 on error
 */
int gen_chars(char *s, const size_t num_char, const int min_word,
              const int max_word, const float *warr, const char *charset);

/**
 * @brief Clean a string by converting to lowercase and validating characters.
 *
 * Converts uppercase letters to lowercase and copies to c1, while ensuring
 * that all characters are supported. Copies at most NUM_WEIGHTS characters.
 *
 * @param c1 Output buffer for the cleaned string.
 * @param c2 Input null-terminated string.
 * @return int Returns 0 on success, -1 if unsupported characters are found.
 */
int gen_clean_charset(char *c1, const char* c2);

#endif // GEN_CHARS_H

// end file gen.h

