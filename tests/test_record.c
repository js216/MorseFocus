/**
 * @file test_record.c
 * @brief Test program for the records-related function.
 *
 * @author Jakob Kastelic
 */

#include "test_record.h"
#include "debug.h"
#include "record.h"
#include "str.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int test_record_load_last(const char *test_file)
{
   // make a test file

   FILE *fp = fopen(test_file, "w");
   if (!fp) {
      TEST_FAIL("cannot create test file");
      return -1;
   }

   if (fprintf(fp, "2025-05-28 12:00:00 0.1 0.0 0.0 3 1 abc 0.1 0.2 0.3\n") <
       0) {
      ERROR("failed to write first line");
      if (fclose(fp) != 0) {
         ERROR("failed to close file");
         return -1;
      }
      return -1;
   }

   if (fprintf(fp, "2025-05-29 13:15:30 0.2 1.0 1.0 3 2 xyz 0.5 0.6 0.7\n") <
       0) {
      ERROR("failed to write second line");
      if (fclose(fp) != 0) {
         ERROR("failed to close file");
         return -1;
      }
      return -1;
   }

   if (fprintf(
           fp,
           "2025-05-30 19:39:10 1.0 3.0 4.0 3 300 abcd~!@#$ 0 1 2 3 4 "
           "5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 "
           "29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49\n") <
       0) {
      ERROR("failed to write third line");
      if (fclose(fp) != 0) {
         ERROR("failed to close file");
         return -1;
      }
      return -1;
   }

   if (fclose(fp) != 0) {
      ERROR("failed to close file");
      return -1;
   }

   // call function under test

   struct record r = record_load_last(test_file);

   // check the parsing worked

   if (r.valid != 1) {
      TEST_FAIL("record not marked as valid");
      return -1;
   }

   if ((2025 != r.datetime.tm_year + 1900) || (5 != r.datetime.tm_mon + 1) ||
       (30 != r.datetime.tm_mday) || (19 != r.datetime.tm_hour) ||
       (39 != r.datetime.tm_min) || (10 != r.datetime.tm_sec)) {
      TEST_FAIL("wrong date/time read back");
      return -1;
   }

   if (r.scale != 1.0) {
      TEST_FAIL("wrong scale %f", r.scale);
      return -1;
   }

   if (r.speed1 != 3.0) {
      TEST_FAIL("wrong speed1 %f", r.speed1);
      return -1;
   }

   if (r.speed2 != 4.0) {
      TEST_FAIL("wrong speed2 %f", r.speed2);
      return -1;
   }

   if ((r.dist != 3.0) || (r.len != 300.0) || (r.speed1 != 3.0) ||
       (r.speed2 != 4.0)) {
      TEST_FAIL("wrong dist or len");
      return -1;
   }

   if (strcmp(r.charset, "abcd~!@#$") != 0) {
      TEST_FAIL("wrong charset read back");
      return -1;
   }

   for (int i = 0; i < MAX_CHARSET_LEN; ++i) {
      if (r.weights[i] != (float)i) {
         TEST_FAIL("wrong weight %f != %f", r.weights[i], (float)i);
         return -1;
      }
   }

   TEST_SUCCESS();
   return 0;
}

int test_record_append(const char *test_file)
{
   FILE *fp = NULL;
   char line[2048];
   struct record r = {0};

   // fill in known values
   parse_datetime(&r.datetime, "2025-05-31 12:34:56");
   r.scale = 0.2F;
   r.speed1 = 3.0F;
   r.speed2 = 4.0F;
   r.dist = 5;
   r.len = 6;
   strcpy(r.charset, "abc");
   for (int i = 0; i < MAX_CHARSET_LEN; ++i)
      r.weights[i] = (float)i;
   r.valid = 1;

   // write to file
   if (record_append(test_file, &r) != 0) {
      TEST_FAIL("cannot append the record");
      return -1;
   }

   // read file and verify contents
   fp = fopen(test_file, "r");
   if (fp == NULL) {
      TEST_FAIL("cannot open the test file");
      return -1;
   }

   // seek to the last line
   while (fgets(line, sizeof(line), fp))
      ; // keep last line in buffer

   if (fclose(fp) != 0) {
      ERROR("failed to close file");
      return -1;
   }

   // check beginning of the line
   const char *exp_line = "2025-05-31 12:34:56 0.200 3.0 4.0 5 6 abc";
   if (strncmp(line, exp_line, strlen(exp_line)) != 0) {
      TEST_FAIL("fixed fields mismatch: %s", line);
      return -1;
   }

   // parse weights from line
   const char *exp_w =
       "0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 "
       "21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 "
       "44 45 46 47 48 49\n";
   if (strcmp(line + strlen(exp_line) + 1, exp_w) != 0) {
      TEST_FAIL("weights mismatch, read: %s", line + strlen(exp_line) + 1);
      return -1;
   }

   TEST_SUCCESS();
   return 0;
}

// end file test_record.c
