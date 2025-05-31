/**
 * @file test_str.c
 * @brief Test program for the string utility functions.
 *
 * @author Jakob Kastelic
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "str.h"

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
            printf("FAIL: non-alphanum character is not a space\n");
            return -1;
         }
      } else {
         if (c_out != (char)tolower(c_in)) {
            printf("FAIL: non-alphanum character is not a space\n");
            return -1;
         }
      }
   }

   printf("SUCCESS: test_str_clean\n");
   return 0;
}


int test_str_file_len(const char *test_file)
{
   int len = str_file_len(test_file);
   size_t expected_len = strlen(test_content);

   if (len != (int)expected_len) {
      printf("FAIL: expected %zu, got %d\n", expected_len, len);
      return -1;
   }

   printf("SUCCESS: test_str_file_len\n");
   return 0;
}


int test_str_read_file(const char *test_file)
{
   char buf[TEST_MAX_LEN] = {0};
   int read_len = str_read_file(buf, test_file, TEST_MAX_LEN);
   size_t expected_len = strlen(test_content);

   if (read_len != (int)expected_len) {
      printf("FAIL: expected %zu, got %d\n", expected_len, read_len);
      return -1;
   }

   if (strcmp(buf, test_content) != 0) {
      printf("FAIL: contents do not match\n");
      return -1;
   }

   printf("SUCCESS: str_read_file\n");
   return 0;
}


int test_str_prepare_test_file(const char *test_file)
{
   FILE *f = fopen(test_file, "wb");
   if (!f) {
      printf("FAIL: failed to create test file\n");
      return -1;
   }

   const size_t ret = fwrite(test_content, 1, strlen(test_content), f);
   if (ret != strlen(test_content)) {
      printf("FAIL: failed to write to test file\n");
      fclose(f);
      return -1;
   }

   printf("SUCCESS: test_str_prepare_test_file\n");
   fclose(f);
   return 0;
}


int test_str_char_to_int(void)
{
    for (char ch = '0'; ch <= '9'; ch++) {
        int expected = ch - '0';
        int result = str_char_to_int(ch);
        if (result != expected) {
            printf("FAIL: '%c' -> %d, expected %d\n", ch, result, expected);
            return -1;
        }
    }

    for (char ch = 'a'; ch <= 'z'; ch++) {
        int expected = 10 + (ch - 'a');
        int result = str_char_to_int(ch);
        if (result != expected) {
            printf("FAIL: '%c' -> %d, expected %d\n", ch, result, expected);
            return -1;
        }
    }

    struct {
        char ch;
        int expected;
    } symbols[] = {
        { '.', 36 }, { '=', 37 }, { ',', 38 },
        { '/', 39 }, { '?', 40 }
    };

    for (int i = 0; i < 5; i++) {
        int result = str_char_to_int(symbols[i].ch);
        if (result != symbols[i].expected) {
            printf("FAIL: '%c' -> %d, expected %d\n",
                   symbols[i].ch, result, symbols[i].expected);
            return -1;
        }
    }

    if (str_char_to_int('!') != -1) {
        printf("FAIL: '!' -> expected -1\n");
        return -1;
    }

    printf("SUCCESS: test_str_char_to_int\n");
    return 0;
}


int test_str_int_to_char(void)
{
    for (int i = 0; i <= 9; i++) {
        char expected = '0' + i;
        char result = str_int_to_char(i);
        if (result != expected) {
            printf("FAIL: %d -> '%c', expected '%c'\n",
                   i, result, expected);
            return -1;
        }
    }

    for (int i = 10; i <= 35; i++) {
        char expected = 'a' + (i - 10);
        char result = str_int_to_char(i);
        if (result != expected) {
            printf("FAIL: %d -> '%c', expected '%c'\n",
                   i, result, expected);
            return -1;
        }
    }

    const char expected_symbols[] = { '.', '=', ',', '/', '?' };
    for (int i = 0; i < 5; i++) {
        int index = 36 + i;
        char result = str_int_to_char(index);
        if (result != expected_symbols[i]) {
            printf("FAIL: %d -> '%c', expected '%c'\n",
                   index, result, expected_symbols[i]);
            return -1;
        }
    }

    if (str_int_to_char(41) != '\0') {
        printf("FAIL: 41 -> expected '\\0'\n");
        return -1;
    }

    printf("SUCCESS: test_str_int_to_char\n");
    return 0;
}

// end file test_str.c

