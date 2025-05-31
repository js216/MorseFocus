/**
 * @file test_record.c
 * @brief Test program for the records-related function.
 *
 * @author Jakob Kastelic
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "str.h"
#include "record.h"
#include "test_record.h"

int test_record_read_last(const char *test_file)
{
   // make a test file

   FILE *fp = fopen(test_file, "w");
   if (!fp) {
      printf("FAIL: cannot create test file");
      return 1;
   }

   fprintf(fp, "2025-05-28 12:00:00 1.0 1.0 0.0 0.0 abc 0.1 0.2 0.3\n");
   fprintf(fp, "2025-05-29 13:15:30 2.0 2.5 1.0 1.0 xyz 0.5 0.6 0.7\n");
   fprintf(fp, "2025-05-30 19:39:10 1.0 2.0 3.0 4.0 abcd~!@#$ 0 1 2 3 4 5 "
         "6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 "
         "29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49\n");

   fclose(fp);

   // call function under test

   struct record r = record_read_last(test_file);

   // check the parsing worked

   if (r.valid != 1) {
      printf("FAIL: record not marked as valid\n");
      return -1;
   }

   if (  (2025 != r.datetime.tm_year + 1900) ||
         (5 != r.datetime.tm_mon + 1) ||
         (30 != r.datetime.tm_mday) ||
         (19 != r.datetime.tm_hour) ||
         (39 != r.datetime.tm_min) ||
         (10 != r.datetime.tm_sec)  ) {
      printf("FAIL: wrong date/time read back\n");
      return -1;
   }

   if (  (r.decay != 1.0) || (r.scale != 2.0) ||
         (r.speed1 != 3.0) || (r.speed2 != 4.0)  ) {
      printf("FAIL: wrong decay/scale/speed1/speed2\n");
      return -1;
   }

   if (strncmp(r.charset, "abcd~!@#$", MAX_CHARSET_LEN) != 0) {
      printf("FAIL: wrong charset read back\n");
      return -1;
   }

   for (int i = 0; i < MAX_CHARSET_LEN; ++i) {
      if (r.weights[i] != i) {
         printf("FAIL: wrong weight %f != %f\n", r.weights[i], (float)i);
         return -1;
      }
   }

   printf("SUCCESS: test_record_read_last\n");
   return 0;
}


int test_record_append(const char *test_file)
{
    FILE *fp;
    char line[2048];
    struct record r = {0};

    // fill in known values
    str_ptime("2025-05-31 12:34:56", "%Y-%m-%d %H:%M:%S", &r.datetime);
    r.decay = 1.0f;
    r.scale = 2.0f;
    r.speed1 = 3.0f;
    r.speed2 = 4.0f;
    strcpy(r.charset, "abc");
    for (int i = 0; i < MAX_CHARSET_LEN; ++i)
        r.weights[i] = (float)i;

    // write to file
    if (record_append(test_file, &r) != 0) {
       printf("FAIL: cannot append the record\n");
       return -1;
    }

    // read file and verify contents
    fp = fopen(test_file, "r");
    if(fp == NULL) {
       printf("FAIL: cannot open the test file\n");
       return -1;
    }

    // seek to the last line
    while (fgets(line, sizeof(line), fp))
        ; // keep last line in buffer

    fclose(fp);

    // check beginning of the line
    const char *exp_line = "2025-05-31 12:34:56 1.000 2.000 3.000 4.000 abc";
    if (strncmp(line, exp_line, 47) != 0) {
       printf("FAIL: fixed fields mismatch\n");
       return -1;
    }

    // parse weights from line
    const char *exp_w = "0.000 1.000 2.000 3.000 4.000 5.000 6.000 7.000 "
       "8.000 9.000 10.000 11.000 12.000 13.000 14.000 15.000 16.000 17.000 "
       "18.000 19.000 20.000 21.000 22.000 23.000 24.000 25.000 26.000 27.000 "
       "28.000 29.000 30.000 31.000 32.000 33.000 34.000 35.000 36.000 37.000 "
       "38.000 39.000 40.000 41.000 42.000 43.000 44.000 45.000 46.000 47.000 "
       "48.000 49.000\n";
    if (strcmp(line+48, exp_w) != 0) {
       printf("FAIL: weights mismatch\n");
       return -1;
    }

    printf("SUCCESS: test_record_append\n");
    return 0;
}

// end file test_record.c

