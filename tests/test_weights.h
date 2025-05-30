/**
 * @file test_weights.h
 * @brief Test program for the weights functions.
 *
 * @author Jakob Kastelic
 */

#ifndef TEST_WEIGHTS_H
#define TEST_WEIGHTS_H

#include <stddef.h>

int create_test_file(const char *filename);
int test_weights_load_last(const char *filename);
int read_last_line(char *buf, size_t size, const char *fname);
int test_weights_append(const char *fname);
int test_weights_add(void);

#endif // TEST_WEIGHTS_H

// end file test_weights.h

