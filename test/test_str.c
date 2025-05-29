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

#define TEST_FILE "test_file.txt"

static const char *test_content =
   "This is a test file.\nIt has multiple lines.\n";

static int test_str_clean(void)
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


static int test_str_file_len(const char *expected_content)
{
   int len = str_file_len(TEST_FILE);
   size_t expected_len = strlen(expected_content);

   if (len != (int)expected_len) {
      printf("str_file_len() test failed: expected %zu, got %d\n",
             expected_len, len);
      return -1;
   }

   printf("str_file_len() passed\n");
   return 0;
}


static int test_str_read_file(const char *expected_content)
{
   char buf[MAX_LEN] = {0};
   int read_len = str_read_file(buf, TEST_FILE, MAX_LEN);
   size_t expected_len = strlen(expected_content);

   if (read_len != (int)expected_len) {
      printf("str_read_file() test failed: expected %zu, got %d\n",
             expected_len, read_len);
      return -1;
   }

   if (strcmp(buf, expected_content) != 0) {
      printf("str_read_file() test failed: contents do not match\n");
      return -1;
   }

   printf("str_read_file() passed\n");
   return 0;
}


static int prepare_test_file(void)
{
   FILE *f = fopen(TEST_FILE, "wb");
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


int main(void)
{
   int result = 0;
   result |= test_str_clean();
   result |= prepare_test_file();
   result |= test_str_file_len(test_content);
   result |= test_str_read_file(test_content);

   remove(TEST_FILE);
   return result;
}

// end file test_str.c

