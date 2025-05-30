/**
 * @file test_str.h
 * @brief Test program for the string utility functions.
 *
 * @author Jakob Kastelic
 */

#ifndef TEST_STR_H
#define TEST_STR_H

int test_str_clean(void);
int test_str_file_len(const char *expected_content);
int test_str_read_file(const char *expected_content);
int test_str_prepare_test_file(const char *test_file);
int test_str_char_to_int(void);
int test_str_int_to_char(void);

#endif // TEST_STR_H

// end file test_str.h


