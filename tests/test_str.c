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

   printf("original: %s\n", input);
   printf("cleaned : %s\n", output);

   /* expected output:
      "hello world! goodbye "
      (non-printables replaced with spaces, and lowercase)
   */

   /* verify each char: non-printable replaced with space */
   int failed = 0;
   for (size_t i = 0; i < len; i++)
   {
      unsigned char c_in = (unsigned char)input[i];
      unsigned char c_out = (unsigned char)output[i];

      if (c_in < 32 || c_in > 126)
      {
         if (c_out != ' ')
         {
            failed = 1;
            break;
         }
      }
      else
      {
         if (c_out != (char)tolower(c_in))
         {
            failed = 1;
            break;
         }
      }
   }

   if (failed)
      printf("test failed.\n");
   else
      printf("test passed.\n");

   return failed ? -1 : 0;
}


int test_str_file_len(const char *test_file)
{
   int len = str_file_len(test_file);
   size_t expected_len = strlen(test_content);

   if (len != (int)expected_len) {
      printf("str_file_len() test failed: expected %zu, got %d\n",
             expected_len, len);
      return -1;
   }

   printf("str_file_len() passed\n");
   return 0;
}


int test_str_read_file(const char *test_file)
{
   char buf[TEST_MAX_LEN] = {0};
   int read_len = str_read_file(buf, test_file, TEST_MAX_LEN);
   size_t expected_len = strlen(test_content);

   if (read_len != (int)expected_len) {
      printf("str_read_file() test failed: expected %zu, got %d\n",
             expected_len, read_len);
      return -1;
   }

   if (strcmp(buf, test_content) != 0) {
      printf("str_read_file() test failed: contents do not match\n");
      return -1;
   }

   printf("str_read_file() passed\n");
   return 0;
}


int test_str_prepare_test_file(const char *test_file)
{
   FILE *f = fopen(test_file, "wb");
   if (!f) {
      perror("failed to create test file");
      return -1;
   }

   const size_t ret = fwrite(test_content, 1, strlen(test_content), f);
   if (ret != strlen(test_content)) {
      perror("failed to write to test file");
      fclose(f);
      return -1;
   }

   fclose(f);
   return 0;
}


int test_str_char_to_int(void)
{
    int failures = 0;

    for (char ch = '0'; ch <= '9'; ch++) {
        int expected = ch - '0';
        int result = str_char_to_int(ch);
        if (result != expected) {
            printf("  FAIL: '%c' -> %d, expected %d\n",
                   ch, result, expected);
            failures++;
        }
    }

    for (char ch = 'a'; ch <= 'z'; ch++) {
        int expected = 10 + (ch - 'a');
        int result = str_char_to_int(ch);
        if (result != expected) {
            printf("  FAIL: '%c' -> %d, expected %d\n",
                   ch, result, expected);
            failures++;
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
            printf("  FAIL: '%c' -> %d, expected %d\n",
                   symbols[i].ch, result, symbols[i].expected);
            failures++;
        }
    }

    if (str_char_to_int('!') != -1) {
        printf("  FAIL: '!' -> expected -1\n");
        failures++;
    }

    if (failures) {
       return -1;
    } else {
       printf("test_str_char_to_int() passed\n");
       return 0;
    }
}


int test_str_int_to_char(void)
{
    int failures = 0;

    for (int i = 0; i <= 9; i++) {
        char expected = '0' + i;
        char result = str_int_to_char(i);
        if (result != expected) {
            printf("  FAIL: %d -> '%c', expected '%c'\n",
                   i, result, expected);
            failures++;
        }
    }

    for (int i = 10; i <= 35; i++) {
        char expected = 'a' + (i - 10);
        char result = str_int_to_char(i);
        if (result != expected) {
            printf("  FAIL: %d -> '%c', expected '%c'\n",
                   i, result, expected);
            failures++;
        }
    }

    const char expected_symbols[] = { '.', '=', ',', '/', '?' };
    for (int i = 0; i < 5; i++) {
        int index = 36 + i;
        char result = str_int_to_char(index);
        if (result != expected_symbols[i]) {
            printf("  FAIL: %d -> '%c', expected '%c'\n",
                   index, result, expected_symbols[i]);
            failures++;
        }
    }

    if (str_int_to_char(41) != '\0') {
        printf("  FAIL: 41 -> expected '\\0'\n");
        failures++;
    }

    if (failures) {
       return -1;
    } else {
       printf("test_str_int_to_char() passed\n");
       return 0;
    }
}

// end file test_str.c

