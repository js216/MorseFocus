/**
 * @file tests.c
 * @brief Runs all tests and returns combined result.
 *
 * @author Jakob Kastelic
 */

#include "debug.h"
#include "str.h"
#include "test_diff.h"
#include "test_gen.h"
#include "test_record.h"
#include "test_str.h"
#include <stdio.h>
#include <string.h>

#define TEST_FILE1 "test_file.txt"
#define TEST_FILE2 "test_file1.txt"
#define TEST_FILE3 "test_file2.txt"

int silence_errors;

static int test_files_check(const char *tf1, const char *tf2, const char *tf3)
{
   FILE *f;

   f = fopen(tf1, "r");
   if (f) {
      fclose(f);
      ERROR("test file '%s' already exists", tf1);
      return -1;
   }

   f = fopen(tf2, "r");
   if (f) {
      fclose(f);
      ERROR("test file '%s' already exists", tf2);
      return -1;
   }

   f = fopen(tf3, "r");
   if (f) {
      fclose(f);
      ERROR("test file '%s' already exists", tf3);
      return -1;
   }

   return 0;
}

int main(void)
{
   int ret = 0;

   ret |= test_files_check(TEST_FILE1, TEST_FILE2, TEST_FILE3);

   ret |= test_str_clean();
   ret |= test_str_prepare_test_file(TEST_FILE1);
   ret |= test_str_file_len(TEST_FILE1);
   ret |= test_str_read_file(TEST_FILE1);
   ret |= test_str_char_to_int();
   ret |= test_str_int_to_char();

   ret |= test_record_load_last(TEST_FILE1);
   ret |= test_record_append(TEST_FILE1);

   ret |= test_diff();

   ret |= test_gen_chars();
   ret |= test_free_entries();
   ret |= test_compute_total_weight();
   ret |= test_select_random_word();
   ret |= test_write_words(TEST_FILE1);
   ret |= test_is_line_too_long(TEST_FILE1);
   ret |= test_validate_word();
   ret |= test_parse_line();
   ret |= test_parse_word_file(TEST_FILE1);
   ret |= test_gen_words(TEST_FILE1, TEST_FILE2, TEST_FILE3);

   remove(TEST_FILE1);
   remove(TEST_FILE2);
   remove(TEST_FILE3);

   return ret;
}

// end file tests.c
