/**
 * @file gen.h
 * @brief Random pattern generation functions.
 *
 * @author Jakob Kastelic
 */

#ifndef GEN_CHARS_H
#define GEN_CHARS_H

#include <stddef.h>
#include <stdio.h>

#define GEN_MAX 100000

struct WordEntry {
   char *word;
   float weight;
};

/**
 * @brief generate a random string of space-separated words with weights
 *
 * @param s output buffer (must be large enough)
 * @param num_char total number of characters to generate (excluding null)
 * @param min_word minimum length of each word (>=1)
 * @param max_word maximum length of each word (>=min_word)
 * @param weights array of at most NUM_WEIGHTS floats weights
 * @param charset string of characters to draw from, or NULL for default
 *
 * @return 0 on success, -1 on error
 */
int gen_chars(char *s, const size_t num_char, const int min_word,
              const int max_word, const float *warr, const char *charset);

/**
 * @brief Generate a sequence of pseudorandom words and write them to an output
 * file or stdout.
 *
 * This function reads up to `nl` lines from the specified `word_file`, where
 * each line contains either a single word or a word followed by a
 * floating-point weight (separated by a space). It then randomly selects `nw`
 * words (with optional weighted probability) and writes them space-separated to
 * the output file or standard output.
 *
 * Valid characters in a word are those for which `str_char_to_int(ch) >= 0`.
 * Lines containing invalid characters, inconsistent use of weights, or
 * malformed formatting will cause an error.
 *
 * If any word line contains a weight, then all lines must. If all weights are
 * zero, words are selected uniformly.
 *
 * @param out_file Path to output file to write generated words. If NULL,
 * output is written to stdout.
 * @param word_file Path to input word list file. If NULL, read from standard
 * input.
 * @param nw Number of random words to generate.
 * @param nl Number of lines (words) to read from the word file.
 *
 * @return 0 on success, -1 on failure.
 */
int gen_words(const char *out_file, const char *word_file, const int nw,
              const int nl);

/**********************************************
 * HELPER FUNCTIONS
 **********************************************/

/**
 * @brief generate a pseudo-random float in the range [0, 1)
 *
 * this function seeds the random number generator on the first call,
 * then returns uniformly distributed floats from 0 (inclusive) up to
 * but not including 1.0.
 *
 * @return a float in [0, 1)
 */
float gen_rand();

/**
 * @brief Free all entries in the given array.
 *
 * @param entries Array of WordEntry structs.
 * @param count Number of entries.
 */
void free_entries(struct WordEntry *entries, int count);

/**
 * @brief Compute the total weight of the word list.
 *
 * @param entries Array of WordEntry.
 * @param count Number of entries.
 * @return Sum of weights as float.
 */
float compute_total_weight(struct WordEntry *entries, int count);

/**
 * @brief Select a word randomly based on weights.
 *
 * @param entries Array of WordEntry.
 * @param count Number of entries.
 * @param total_weight Total weight of all entries.
 * @return Pointer to selected word.
 */
const char *select_random_word(struct WordEntry *entries, int count,
                               float total_weight);

/**
 * @brief Write randomly selected words to the given output.
 *
 * @param out Output file pointer.
 * @param entries Word entries to choose from.
 * @param count Number of entries.
 * @param nw Number of words to generate.
 * @param total_weight Total weight of all entries.
 * @return 0 on success, -1 on error.
 */
int write_words(FILE *out, struct WordEntry *entries, int count, int nw,
                float total_weight);

/**
 * @brief Checks if the read line is too long to fit in the buffer.
 *
 * Reads a line into the buffer, and if there is no newline character
 * found before EOF, it indicates the line exceeds the buffer size.
 *
 * @param fp File pointer (used to check EOF status).
 * @param line Buffer containing the line read.
 * @return 1 if the line is too long, 0 otherwise.
 */
int is_line_too_long(FILE *fp, char *line);

/**
 * @brief Validates that a word contains only allowed characters.
 *
 * Checks each character in the input word with str_char_to_int().
 * Returns 1 if all characters are valid, 0 if any invalid character is found.
 *
 * @param word Null-terminated string to validate.
 * @return 0 if valid, -1 if invalid characters detected.
 */
int validate_word(const char *word);

/**
 * @brief Parses a line into word and optional weight.
 *
 * @param line Input line (modified in-place)
 * @param word_out Output pointer for word (token inside line)
 * @param weight_out Output pointer for weight, set to 0 if none
 * @param has_weight_out Input/output:
 *        -1 = unknown, 0 = no weights so far, 1 = weights so far
 *        updated if line has weight
 * @return 0 on success, -1 on error (e.g. inconsistent weights)
 */
int parse_line(const char *line, char **word_out, float *weight_out,
               int *has_weight_out);

/**
 * @brief Parse word file with optional weights.
 *
 * Reads up to nl lines from word_file. Each line contains a word and
 * optionally a weight number separated by space.
 * Checks validity of characters in words.
 * On success, allocates entries array (must be freed by caller).
 *
 * @param word_file Path to input file. If NULL, will read from standard input.
 * @param entries_out Output pointer to allocated WordEntry array.
 * @param nl Number of lines to read (all if not positive)
 * @return Number of entries read on success, -1 on failure.
 */
int parse_word_file(const char *word_file, struct WordEntry **entries_out,
                    int nl);

#endif // GEN_CHARS_H

// end file gen.h
