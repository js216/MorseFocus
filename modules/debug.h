/**
 * @file debug.h
 * @brief Routines for error handling etc.
 *
 * @author Jakob Kastelic
 */

#include <stdio.h>

extern int silence_errors;

#define ERROR(...) \
    do { \
        if (!silence_errors) { \
            printf("\033[1;31merror:\033[0m %s in %s (line %d): ", \
                   __func__, __FILE__, __LINE__); \
            printf(__VA_ARGS__); \
            printf("\n"); \
        } \
    } while (0)


#define TEST_FAIL(...) \
    do { \
        printf("\033[1;31mFAIL:\033[0m %s in %s (line %d): ", \
               __func__, __FILE__, __LINE__); \
        printf(__VA_ARGS__); \
        printf("\n"); \
    } while (0)

#define TEST_SUCCESS() \
    do { \
        printf("\033[32mSUCCESS:\033[0m %s\n", __func__); \
    } while (0)

// end file debug.h

