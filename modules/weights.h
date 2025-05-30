/**
 * @file weights.h
 * @brief Manipulation of character weight factors.
 *
 * @author Jakob Kastelic
 */

#ifndef WEIGHTS_H
#define WEIGHTS_H

#define WEIGHTS_SKIP 3

/**
 * @brief Load weights from the last line of a text file.
 *
 * @param weights pointer to output array of floats
 * @param fname path to file
 * @param max_num maximum number of weights to read
 * @return number of weights read, or -1 on error
 */
int weights_load_last(float *weights, const char *fname, const int max_num);

/**
 * @brief Appends a line of space-separated weights to a file.
 *
 * This function writes `num_weights` floating point values from the `weights`
 * array to a new line at the end of the file specified by `fname`.
 *
 * @param f Path to the output file.
 * @param w Array of floating point values to append.
 * @param nw Number of weights to write.
 * @param d Number with which all weights are multiplied before writing.
 *
 * @return 0 on success, -1 on error (e.g., file open failure).
 */
int weights_append(const char *f, const float *w, const int nw, const float d);

/**
 * @brief Adds two arrays of weights element-wise into a third array.
 *
 * The output array can be the same as either (or both) of the input arrays.
 *
 * @param w output array where sums are stored
 * @param w1 first input weights array
 * @param w2 second input weights array
 * @param len number of elements to add
 * @return int always 0 (success)
 */
int weights_add(float *w, const float *w1, const float *w2, const int len);

/**
 * @brief Print the weights, if they are nonzero.
 * @param weights Array of num weights.
 * @param num Number of weights to print from the array.
 */
void weights_printout(const float *weights, const int num);

#endif /* WEIGHTS_H */

// end file weights.h

