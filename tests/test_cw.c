/**
 * @file test_cw.c
 * @brief Test Morse code generation.
 *
 * @author Jakob Kastelic
 */

#include "cw.h"
#include "debug.h"
#include <string.h>

int test_ascii_to_morse_expanded(void)
{
   struct {
      const char *input;
      const char *expected;
   } tests[] = {{"PARIS", ".--.|.-|.-.|..|..."},
                {"SOS", "...|---|..."},
                {"HELLO WORLD", "....|.|.-..|.-..|---/.--|---|.-.|.-..|-.."},
                {"", ""},
                {"123", ".----|..---|...--"}};

   char buf[256];

   for (size_t i = 0; i < sizeof(tests) / sizeof(tests[0]); i++) {
      ascii_to_morse_expanded(tests[i].input, buf);
      if (strcmp(buf, tests[i].expected) != 0) {
         TEST_FAIL("ascii_to_morse_expanded failed:\n  input: \"%s\"\n"
                   "  expected: \"%s\"\n  got: \"%s\"",
                   tests[i].input, tests[i].expected, buf);
         return -1;
      }
   }

   TEST_SUCCESS();
   return 0;
}

int test_count_units(void)
{
   struct {
      const char *morse;
      int expected_units;
   } tests[] = {
       // PARIS
       {".--.|.-|.-.|..|...", 43},

       // SOS
       {"...|---|...", 27},

       // HELLO WORLD
       {"....|.|.-..|.-..|---/.--|---|.-.|.-..|-..", 111},

       // blank
       {"", 0},

       // 123
       {".----|..---|...--", 51},

       // THE
       {"-|....|.", 17},

       // QUICK
       {"--.-|..-|..|-.-.|-.-", 55},

       // BROWN
       {"-...|.-.|---|.--|-.", 53},

       // FOX
       {"..-.|---|-..-", 37},

       // JUMPS
       {".---|..-|--|.--.|...", 55},

       // OVER
       {"---|...-|.|.-.", 37},

       // LAZY
       {".-..|.-|--..|-.--", 47},

       // DOG
       {"-..|---|--.", 33},

       // THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG
       {"-|....|.|/--.-|..-|..|-.-.|-.-/-...|.-.|---|.--|-/..-.|---|-..-/"
        ".---|..-|--|.--.|.../---|...-|.|.-.|/-|....|.|/.-..|.-|--..|-.-"
        "-/-..|---|--.",
        414},
   };

   for (size_t i = 0; i < sizeof(tests) / sizeof(tests[0]); i++) {
      int units = count_units(tests[i].morse);
      if (units != tests[i].expected_units) {
         TEST_FAIL("count_units failed:\n  morse: \"%s\"\n"
                   "  expected: %d\n  got: %d",
                   tests[i].morse, tests[i].expected_units, units);
         return -1;
      }
   }

   TEST_SUCCESS();
   return 0;
}

// end file test_cw.c
