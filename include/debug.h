/**
 * @file debug.h
 * @brief Routines for error handling etc.
 *
 * @author Jakob Kastelic
 */

#ifndef DEBUG_H
#define DEBUG_H

#include <stdbool.h>
#include <stdio.h>

#define ERROR(...)                                                             \
   do {                                                                        \
      if (!debug_is_silent()) {                                                \
         printf("\033[1;31merror:\033[0m %s in %s (line %d): ", __func__,      \
                __FILE__, __LINE__);                                           \
         printf(__VA_ARGS__);                                                  \
         printf("\n");                                                         \
      }                                                                        \
   } while (0)

#define TEST_FAIL(...)                                                         \
   do {                                                                        \
      printf("\033[1;31mFAIL:\033[0m %s in %s (line %d): ", __func__,          \
             __FILE__, __LINE__);                                              \
      printf(__VA_ARGS__);                                                     \
      printf("\n");                                                            \
   } while (0)

#define TEST_SUCCESS()                                                         \
   do {                                                                        \
      printf("\033[32mSUCCESS:\033[0m %s\n", __func__);                        \
   } while (0)

/**
 * @brief Suppress debugging/error messages (or not).
 * @param silent If true, suppress messages; if false, enable them.
 */
void debug_set_silent(bool silent);

/**
 * @brief Get current debug state.
 * @return If true, messages suppressed; if false, they are enabled.
 */
bool debug_is_silent(void);

#endif // DEBUG_H

// end file debug.h
