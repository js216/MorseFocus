/**
 * @file xorshift32.c
 * @brief Simple xorshift32 pseudorandom number generator (PRNG).
 *
 * This PRNG is a fast, lightweight 32-bit generator suitable for general
 * purpose use. It is **not** cryptographically secure.
 *
 * @author Jakob Kastelic
 */

#include "modules/xorshift32.h"
#include <stdint.h>
#include <time.h>

/* Default nonzero seed if user passes zero */
#define DEFAULT_SEED 0xdeadbeefU

uint32_t xorshift32_next(void)
{
   static uint32_t state = 0;
   if (state == 0) {
      /* Auto-seed on first call using time and address entropy */
      uintptr_t addr = (uintptr_t)&state;
      uint32_t time_seed = (uint32_t)time(NULL);
      state = time_seed ^ (uint32_t)addr;
   }

   uint32_t x = state;
   x ^= x << 13UL;
   x ^= x >> 17UL;
   x ^= x << 5UL;
   state = x;
   return x;
}

float xorshift32_rand_float(void)
{
   /* Normalize to [0,1) */
   /* UINT32_MAX + 1.0F == 4294967296.0F */
   return (float)xorshift32_next() / 4294967296.0F;
}

// end file xorshift32.c
