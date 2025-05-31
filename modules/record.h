/**
 * @file record.h
 * @brief Manipulation of practice records.
 *
 * @author Jakob Kastelic
 */

#ifndef RECORD_H
#define RECORD_H

#include <time.h>

#define MAX_CHARSET_LEN 50

struct record {
   int valid;
   struct tm datetime;
   float decay;
   float scale;
   float speed1;
   float speed2;
   char charset[MAX_CHARSET_LEN];
   float weights[MAX_CHARSET_LEN];
};

/**
 * @brief Reads the last line of a text file and parses it into a struct record.
 *
 * The file must contain lines in the following format:
 * @verbatim
 * YYYY-MM-DD HH:MM:SS decay scale speed1 speed2 charset w1 w2 w3 ...
 * @endverbatim
 *
 * - `charset` must be a single contiguous string (no spaces), with a length
 *   between 1 and MAX_CHARSET_LEN characters (inclusive).
 *
 * - The weights (`w1`, `w2`, ...) must be at least one and at most
 *   MAX_CHARSET_LEN floating-point numbers.
 *
 * If the line is malformed or validation fails, the function prints an error
 * to stderr and returns an invalid struct record (field `valid` is set to 0).
 *
 * @param filename Path to the input text file.
 *
 * @return A struct record parsed from the last line of the file.
 */
struct record record_read_last(const char *filename);

/**
 * @brief Appends a record to the given file in text format.
 *
 * Format: YYYY-MM-DD HH:MM:SS decay scale speed1 speed2 charset w1 ...
 *
 * @param path Path to the output file.
 * @param r Pointer to the record to write.
 * @return 0 on success, -1 on error (with error message to stderr).
 */
int record_append(const char *path, const struct record *r);

#endif // RECORD_H

// end file record.h

