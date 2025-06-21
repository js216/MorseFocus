/**
 * @file tests.c
 * @brief Runs all tests and returns combined result.
 *
 * @author Jakob Kastelic
 */

#include <stdio.h>
#include <string.h>

#include "modules/debug.h"

#include "test_cw.h"
#include "test_diff.h"
#include "test_gen.h"
#include "test_record.h"
#include "test_str.h"

#define TEST_FILE1 "test_file.txt"
#define TEST_FILE2 "test_file1.txt"
#define TEST_FILE3 "test_file2.txt"

int silence_errors;

static int test_files_check(const char *tf1, const char *tf2, const char *tf3)
{
   FILE *f = NULL;
   int ret = 0;

   f = fopen(tf1, "r");
   if (f) {
      ERROR("test file '%s' already exists", tf1);
      ret = -1;
      goto cleanup;
   }

   f = fopen(tf2, "r");
   if (f) {
      ERROR("test file '%s' already exists", tf2);
      ret = -1;
      goto cleanup;
   }

   f = fopen(tf3, "r");
   if (f) {
      ERROR("test file '%s' already exists", tf3);
      ret = -1;
      goto cleanup;
   }

cleanup:
   if (f) {
      if (fclose(f) != 0) {
         ERROR("failed to close file");
         ret = -1;
      }
   }

   return ret;
}

int main(void)
{
   int ret = 0;

   ret = ret || test_files_check(TEST_FILE1, TEST_FILE2, TEST_FILE3);

   ret = ret || test_str_clean();
   ret = ret || test_str_prepare_test_file(TEST_FILE1);
   ret = ret || test_str_file_len(TEST_FILE1);
   ret = ret || test_str_read_file(TEST_FILE1);
   ret = ret || test_str_char_to_int();
   ret = ret || test_str_int_to_char();

   ret = ret || test_record_load_last(TEST_FILE1);
   ret = ret || test_record_append(TEST_FILE1);

   ret = ret || test_diff();

   ret = ret || test_gen_chars();
   ret = ret || test_free_entries();
   ret = ret || test_compute_total_weight();
   ret = ret || test_select_random_word();
   ret = ret || test_write_words(TEST_FILE1);
   ret = ret || test_is_line_too_long(TEST_FILE1);
   ret = ret || test_validate_word();
   ret = ret || test_parse_line();
   ret = ret || test_parse_word_file(TEST_FILE1);
   ret = ret || test_gen_words(TEST_FILE1, TEST_FILE2, TEST_FILE3);

   ret = ret || test_ascii_to_morse_expanded();
   ret = ret || test_count_units();

   return ret;
}

// end file tests.c
