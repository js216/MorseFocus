/**
 * @file str.h
 * @brief String utility functions.
 *
 * @author Jakob Kastelic
 */

#ifndef STR_H
#define STR_H

#include <stddef.h>
#include <stdio.h>
#include <time.h>

#define MAX_CHARSET_LEN 50
#define MAX_WORD_LINE 128
#define MAX_CSV_LEN 4096
#define MAX_FILENAME_LEN 256

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
 * @brief Reads up to max_len - 1 bytes from a file into a buffer and
 * null-terminates it.
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
 * Digits map to 0 to 9, lowercase letters to 10 to35, symbols to 36 to 40.
 *
 * @param ch The character to convert.
 * @return int The corresponding integer value, or -1 if unsupported.
 */
int str_char_to_int(const char ch);

/**
 * @brief Convert an integer back to its corresponding character.
 *
 * Integers 0–9 map to '0'–'9', 10–35 to 'a' to 'z',
 * and 36–40 to the symbols . = , / ?
 *
 * @param i The integer to convert.
 * @return char The corresponding character, or '\0' if out of range.
 */
char str_int_to_char(const int i);

/**
 * @brief Thread-safe, reentrant string tokenizer.
 *
 * Splits the input string into tokens separated by delimiter characters.
 * On the first call, provide the string in `str`. For subsequent calls,
 * pass NULL for `str` and use `saveptr` to continue tokenizing the same string.
 *
 * This function is a replacement for strtok_r where it is unavailable.
 *
 * @param str The string to tokenize on first call or NULL for subsequent calls.
 * @param delim Null-terminated string of delimiter characters.
 * @param saveptr Pointer to a char* variable storing context between calls.
 *
 * @return Pointer to the next token, or NULL if no tokens remain.
 */
char *str_tok(char *str, const char *delim, char **saveptr);

/**
 * @brief Simple portable fallback for strptime for format "%Y-%m-%d %H:%M:%S".
 *
 * Parses a datetime string like "2025-05-31 12:34:56" into a struct tm.
 *
 * @param s Input datetime string.
 * @param format Expected format string (must be exactly "%Y-%m-%d %H:%M:%S").
 * @param tm Pointer to struct tm to fill.
 *
 * @return Pointer to the character after the parsed part on success, or NULL on
 * failure.
 */
char *str_ptime(const char *s, const char *format, struct tm *tm);

/**
 * @brief Check if string contains only allowed characters.
 *
 * @param s String to check.
 * @return 0 on success, -1 if unsupported characters are found
 */
int str_is_clean(const char *s);

/**
 * @brief Duplicate a null-terminated string by allocating new memory.
 *
 * Allocates enough memory for a copy of the string `s`, copies it,
 * and returns a pointer to the new string. Returns NULL if memory
 * allocation fails or if `s` is NULL.
 *
 * @param s Null-terminated input string to duplicate.
 * @return Pointer to duplicated string on success, NULL on failure.
 */
char *str_dup(const char *s);

/**
 * @brief Counts the number of lines in a given text file.
 *
 * This function opens the specified file and counts the number of lines
 * by counting newline ('\n') characters. If the file does not end with
 * a newline but is not empty, the last line is also counted.
 *
 * @param filename The path to the file to count lines in.
 *
 * @return The number of lines in the file on success,
 *         or -1 if the file could not be opened.
 */
int str_file_lines(const char *filename);

#endif // STR_H

// end file str.h
