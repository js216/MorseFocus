/**
 * @file test_gen.h
 * @brief Test program for the text generation functions.
 *
 * @author Jakob Kastelic
 */

#ifndef TEST_GEN_H
#define TEST_GEN_H

int test_gen_chars(void);
int test_gen_clean_charset(void);
int test_free_entries(void);
int test_compute_total_weight(void);
int test_select_random_word(void);
int test_write_words(const char *test_file);
int test_is_line_too_long(const char *test_file);
int test_validate_word(void);
int test_parse_line(void);
int test_parse_word_file(const char *test_file);
int test_gen_words(const char *tf1, const char *tf2, const char *tf3);

#endif // TEST_GEN_H

// end file test_gen.h

