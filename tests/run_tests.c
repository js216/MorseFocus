/**
 * @file tests.c
 * @brief Runs all tests and returns combined result.
 *
 * @author Jakob Kastelic
 */

#include <stdio.h>
#include "test_diff.h"
#include "test_gen.h"
#include "test_str.h"
#include "test_weights.h"
#include "test_record.h"

#define TEST_FILE "test_file.txt"

int main(void)
{
   int ret = 0;

   ret |= test_diff();

   ret |= test_gen_chars();
   ret |= test_gen_clean_charset();

   ret |= test_str_clean();
   ret |= test_str_prepare_test_file(TEST_FILE);
   ret |= test_str_file_len(TEST_FILE);
   ret |= test_str_read_file(TEST_FILE);
   ret |= test_str_char_to_int();
   ret |= test_str_int_to_char();

   ret |= test_weights_load_last(TEST_FILE);
   ret |= test_weights_append(TEST_FILE);
   ret |= test_weights_add();

   ret |= test_record_read_last(TEST_FILE);
   ret |= test_record_append(TEST_FILE);

   remove(TEST_FILE);
   return ret;
}

// end file tests.c

