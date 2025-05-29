/**
 * @file diff.h
 * @brief Levenshtein distance computation and character frequency tracking.
 *
 * @author Jakob Kastelic
 */

#ifndef LEV_DIFF_H
#define LEV_DIFF_H

#include <stddef.h>

/**
 * @brief Computes the Levenshtein distance between two strings and records
 * character-level insertions and deletions.
 *
 * This function calculates the Levenshtein distance between two null-terminated
 * strings `s1` and `s2`, using a dynamic programming approach. It returns the
 * number of single-character edits (insertions, deletions, or substitutions)
 * required to transform one string into the other.
 *
 * The function also records the frequency of insertions and deletions for each
 * distinct character involved in the transformation (excluding substitutions)
 * into the `warr` array.
 *
 * @param warr A pointer to an array of at most 95 integers. Each element
 * corresponds to a subset of printable ASCII characters. This array is updated
 * to count the number of insertions or deletions for each character.
 *
 * @param s1 A pointer to the first null-terminated input string.
 *
 * @param s2 A pointer to the second null-terminated input string.
 *
 * @return The Levenshtein distance between `s1` and `s2`.
 *
 * @note The strings are assumed to contain only printable ASCII characters
 * (codes 32 through 126). The character `' '` (ASCII 32) is not tracked in the
 * `warr` array.
 */
int lev_diff(int *warr, const char *s1, const char *s2);

#endif /* LEV_DIFF_H */

// end file diff.h

