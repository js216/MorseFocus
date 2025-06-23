/**
 * @file debug.c
 * @brief Routines for error handling etc.
 *
 * @author Jakob Kastelic
 */

#include "debug.h"
#include <stdbool.h>

static bool silence_errors = 0;

void debug_set_silent(bool silent)
{
   silence_errors = silent;
}

bool debug_is_silent(void)
{
   return silence_errors;
}

// end file debug.c
