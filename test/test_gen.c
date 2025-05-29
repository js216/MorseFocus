/**
 * @file test_gen.c
 * @brief Test program for the text generation functions.
 *
 * @author Jakob Kastelic
 */

#include <stdio.h>
#include <string.h>

#include "gen.h"
#include "str.h"

#define TEST_FILE "test_file.txt"

static const char *default_charset = "kmuresnaptlwi.jz=foy,vg5/q92h38b?47c1d60x";

static int test_gen_chars(void)
{
   char buf[101];
   int ret;

   // test with default charset and uniform weights
   ret = gen_chars(buf, 75, 3, 6, NULL, NULL);
   printf("gen_chars default, uniform weights, returned %d\n", ret);
   printf("result: \"%s\"\n", buf);
   printf("length: %zu\n", strlen(buf));
   printf("\n");

   // test with custom charset and uniform weights
   ret = gen_chars(buf, 50, 2, 4, NULL, "abcde");
   printf("gen_chars custom charset, returned %d\n", ret);
   printf("result: \"%s\"\n", buf);
   printf("length: %zu\n", strlen(buf));
   printf("\n");

   // set nonzero weights only for chars in default_charset
   float weights[MAX_CHARS] = {0.0f};
   size_t charset_len = strlen(default_charset);
   for (size_t i = 0; i < charset_len; ++i)
   {
      unsigned char ch = default_charset[i];
      if (ch >= 33 && ch <= 126)
         weights[ch - 33] = 1.0f;
   }

   // test with weighted chars favoring '!' (ASCII 33) heavily
   weights[20] = 50.0f;

   ret = gen_chars(buf, 80, 4, 8, weights, NULL);
   printf("gen_chars weighted, returned %d\n", ret);
   printf("result: \"%s\"\n", buf);
   printf("length: %zu\n", strlen(buf));
   printf("\n");

   return 0;
}


static int test_gen_char_supported(void)
{
    struct {
        char ch;
        int expected;
    } tests[] = {
        { 'a', 1 },
        { 'z', 1 },
        { 'A', 0 },
        { '9', 1 },
        { '=', 1 },
        { '.', 1 },
        { '?', 1 },
        { '/', 1 },
        { ',', 1 },
        { '@', 0 },
        { '^', 0 },
        { '\n', 0 },
    };

    int failures = 0;
    printf("Testing gen_char_supported:\n");

    for (size_t i = 0; i < sizeof(tests)/sizeof(tests[0]); i++) {
        int result = gen_char_supported(tests[i].ch);
        if (result != tests[i].expected) {
            printf("  FAIL: '%c' -> expected %d, got %d\n", tests[i].ch,
                  tests[i].expected, result);
            failures++;
        }
    }

    return failures ? -1 : 0;
}


static int test_gen_clean_charset(void)
{
    struct {
        const char *input;
        const char *expected;  // NULL means the input should be rejected
    } tests[] = {
        { "HelloWorld123", "helloworld123" },
        { "Test=OK/yes?", "test=ok/yes?" },
        { "Invalid^Char", NULL },  // contains ^
        { "MixED.CaSe123", "mixed.case123" },
        { "fine,string", "fine,string" },
        { "Bad#", NULL },  // contains #
    };

    int failures = 0;
    char cleaned[MAX_CHARS];
    printf("\nTesting gen_clean_charset:\n");

    for (size_t i = 0; i < sizeof(tests)/sizeof(tests[0]); i++) {
        int result = gen_clean_charset(cleaned, tests[i].input);

        if (tests[i].expected == NULL) {
            if (result == 0) {
                printf(
                   "  FAIL: \"%s\" -> expected rejection, but was accepted\n",
                   tests[i].input);
                failures++;
            }
        } else {
            if (result != 0) {
                printf(
                   "  FAIL: \"%s\" -> expected success, but was rejected\n",
                   tests[i].input);
                failures++;
            } else if (strcmp(cleaned, tests[i].expected) != 0) {
                printf("  FAIL: \"%s\" -> expected \"%s\", got \"%s\"\n",
                       tests[i].input, tests[i].expected, cleaned);
                failures++;
            }
        }
    }

    return failures ? -1 : 0;
}

int main(void)
{
   int result = 0;
   result |= test_gen_chars();
   result |= test_gen_char_supported();
   result |= test_gen_clean_charset();

   remove(TEST_FILE);
   return result;
}


// end file test_gen.c

