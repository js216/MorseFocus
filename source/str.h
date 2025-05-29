/**
 * @file str.h
 * @brief String utility functions.
 *
 * @author Jakob Kastelic
 */

#ifndef STR_H
#define STR_H

#include <stddef.h>

#define MAX_CHARS 95
#define MAX_LEN 8192

/**
 * @brief Cleans s2 into s1, replacing non-printable characters with spaces, and
 * converting all letters to lowercase.
 *
 * @param s1 Destination buffer (must have space for at least len chars)
 * @param s2 Source buffer to clean (len chars)
 * @param len Number of characters to process
 *
 * @return Number of characters processed (equal to len)
 */
int str_clean(char *s1, const char *s2, size_t len);

/**
 * @brief Returns the length of a file in bytes.
 *
 * @param fname Path to the file.
 * @return Length of file in bytes, or -1 on error.
 */
int str_file_len(const char *fname);

/**
 * @brief Reads up to max_len - 1 bytes from a file into a buffer and null-terminates it.
 *
 * @param buf Destination buffer (must be at least max_len bytes).
 * @param fname Path to the file.
 * @param max_len Maximum number of bytes including null terminator.
 * @return Number of bytes read, or -1 on error.
 */
int str_read_file(char *buf, const char *fname, const size_t max_len);

/**
 * @brief Convert a supported character to its corresponding integer value.
 *
 * Digits map to 0–9, lowercase letters to 10–35, symbols to 36–40.
 *
 * @param ch The character to convert.
 * @return int The corresponding integer value, or -1 if unsupported.
 */
int str_char_to_int(const char ch);

/**
 * @brief Convert an integer back to its corresponding character.
 *
 * Integers 0–9 map to '0'–'9', 10–35 to 'a'–'z',
 * and 36–40 to the symbols . = , / ?
 *
 * @param i The integer to convert.
 * @return char The corresponding character, or '\0' if out of range.
 */
char str_int_to_char(const int i);

/**
 * @brief Write the current local date and time to an open file.
 *
 * The function writes the current date and time in the format
 * "YYYY-MM-DD HH:MM:SS" followed by a newline to the specified file.
 *
 * If the local time cannot be obtained or formatted, an error message
 * is printed to the standard error.
 *
 * @param fp Pointer to an open FILE stream for writing.
 * @return 0 on success and -1 on failure.
 */
int str_write_time(FILE *fp);

/**
 * @brief Skip a given number of space-delimited fields in a string.
 *
 * A "field" is defined as a sequence of non-space characters separated by
 * one or more spaces. This function returns a pointer to the first character
 * after skipping `n` fields.
 *
 * @param s The input C string (must not be NULL).
 * @param n The number of fields to skip.
 * @return char* Pointer to the character after `n` fields,
 *               or to the null terminator if fewer fields exist.
 */
char* str_skip_fields(const char *s, const int n);

#endif // STR_H

// end file str.h

