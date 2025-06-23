/**
 * @file test_str.c
 * @brief Test program for the string utility functions.
 *
 * @author Jakob Kastelic
 */

#include "test_str.h"
#include "debug.h"
#include "str.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEST_MAX_LEN 100

static const char *test_content =
    "This is a test file.\nIt has multiple lines.\n";

int test_str_clean(void)
{
   const char input[] = "Hello\x08bWorld!\x07Goodye\x01";
   size_t len = strlen(input);

   char output[len + 1];
   output[len] = '\0';

   str_clean(output, input, len);

   /* verify each char: non-printable replaced with space */
   for (size_t i = 0; i < len; i++) {
      unsigned char c_in = (unsigned char)input[i];
      unsigned char c_out = (unsigned char)output[i];

      if (c_in < 32 || c_in > 126) {
         if (c_out != ' ') {
            TEST_FAIL("non-alphanum character is not a space");
            return -1;
         }
      } else {
         if (c_out != (unsigned char)tolower(c_in)) {
            TEST_FAIL("non-alphanum character is not a space");
            return -1;
         }
      }
   }

   TEST_SUCCESS();
   return 0;
}

int test_str_file_len(const char *test_file)
{
   int len = str_file_len(test_file);
   size_t expected_len = strlen(test_content);

   if (len != (int)expected_len) {
      TEST_FAIL("expected %zu, got %d", expected_len, len);
      return -1;
   }

   TEST_SUCCESS();
   return 0;
}

int test_str_read_file(const char *test_file)
{
   char buf[TEST_MAX_LEN] = {0};
   int read_len = str_read_file(buf, test_file, TEST_MAX_LEN);
   size_t expected_len = strlen(test_content);

   if (read_len != (int)expected_len) {
      TEST_FAIL("expected %zu, got %d", expected_len, read_len);
      return -1;
   }

   if (strcmp(buf, test_content) != 0) {
      TEST_FAIL("contents do not match");
      return -1;
   }

   TEST_SUCCESS();
   return 0;
}

int test_str_prepare_test_file(const char *test_file)
{
   FILE *f = fopen(test_file, "wb");
   if (!f) {
      TEST_FAIL("failed to create test file");
      return -1;
   }

   const size_t ret = fwrite(test_content, 1, strlen(test_content), f);
   if (ret != strlen(test_content)) {
      TEST_FAIL("failed to write to test file");
      if (fclose(f) != 0) {
         ERROR("failed to close file");
         return -1;
      }
      return -1;
   }

   TEST_SUCCESS();
   if (fclose(f) != 0) {
      ERROR("failed to close file");
      return -1;
   }
   return 0;
}

int test_str_char_to_int(void)
{
   for (int ch = '0'; ch <= '9'; ch++) {
      int expected = ch - '0';
      int result = str_char_to_int((char)ch);
      if (result != expected) {
         TEST_FAIL("'%c' -> %d, expected %d", ch, result, expected);
         return -1;
      }
   }

   for (int ch = 'a'; ch <= 'z'; ch++) {
      int expected = 10 + (ch - 'a');
      int result = str_char_to_int((char)ch);
      if (result != expected) {
         TEST_FAIL("'%c' -> %d, expected %d", ch, result, expected);
         return -1;
      }
   }

   struct {
      char ch;
      int expected;
   } symbols[] = {{'.', 36}, {'=', 37}, {',', 38}, {'/', 39}, {'?', 40}};

   for (int i = 0; i < 5; i++) {
      int result = str_char_to_int(symbols[i].ch);
      if (result != symbols[i].expected) {
         TEST_FAIL("'%c' -> %d, expected %d", symbols[i].ch, result,
                   symbols[i].expected);
         return -1;
      }
   }

   if (str_char_to_int('!') != -1) {
      TEST_FAIL("'!' -> expected -1");
      return -1;
   }

   TEST_SUCCESS();
   return 0;
}

int test_str_int_to_char(void)
{
   for (int i = 0; i <= 9; i++) {
      char expected = (char)('0' + i);
      char result = str_int_to_char(i);
      if (result != expected) {
         TEST_FAIL("%d -> '%c', expected '%c'", i, result, expected);
         return -1;
      }
   }

   for (int i = 10; i <= 35; i++) {
      char expected = (char)('a' + (i - 10));
      char result = str_int_to_char(i);
      if (result != expected) {
         TEST_FAIL("%d -> '%c', expected '%c'", i, result, expected);
         return -1;
      }
   }

   const char expected_symbols[] = {'.', '=', ',', '/', '?', '\''};
   for (int i = 0; i < 6; i++) {
      int index = 36 + i;
      char result = str_int_to_char(index);
      if (result != expected_symbols[i]) {
         TEST_FAIL("%d -> '%c', expected '%c'", index, result,
                   expected_symbols[i]);
         return -1;
      }
   }

   if (str_int_to_char(42) != '\0') {
      TEST_FAIL("42 -> expected '0'");
      return -1;
   }

   TEST_SUCCESS();
   return 0;
}

// end file test_str.c
