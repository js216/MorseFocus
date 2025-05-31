/**
 * @file test_gen.c
 * @brief Test program for the text generation functions.
 *
 * @author Jakob Kastelic
 */

#include <stdio.h>
#include <string.h>

#include "str.h"
#include "record.h"
#include "gen.h"

static const char *default_charset = "kmuresnaptlwi.jz=foy,vg5/q92h38b?47c1d60x";

int test_gen_chars(void)
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
   float weights[MAX_CHARSET_LEN] = {0.0f};
   size_t charset_len = strlen(default_charset);
   for (size_t i = 0; i < charset_len; i++) {
      unsigned char ch = default_charset[i];
      const int ch_i = str_char_to_int(ch);
      if (ch_i >= 0)
         weights[ch_i] = 1.0f;
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


int test_gen_clean_charset(void)
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
    char cleaned[MAX_CHARSET_LEN];
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

// end file test_gen.c

