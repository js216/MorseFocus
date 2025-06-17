/**
 * @file xorshift32.h
 * @brief Simple xorshift32 pseudorandom number generator (PRNG).
 *
 * This PRNG is a fast, lightweight 32-bit generator suitable for general
 * purpose use. It is **not** cryptographically secure.
 *
 * @author Jakob Kastelic
 */

#ifndef XORSHIFT32_H
#define XORSHIFT32_H

#include <stdint.h>

/**
 * @brief Seed the xorshift32 PRNG.
 *
 * @param seed Nonzero seed value to initialize the generator.
 *             If zero is passed, a default seed will be used.
 */
void xorshift32_seed(uint32_t seed);

/**
 * @brief Generate a pseudorandom 32-bit unsigned integer.
 *
 * @return uint32_t A 32-bit pseudorandom number.
 */
uint32_t xorshift32_next(void);

/**
 * @brief Generate a pseudorandom floating point number in [0,1).
 *
 * @return float A float in the range [0, 1), uniformly distributed.
 */
float xorshift32_rand_float(void);

#endif /* XORSHIFT32_H */

// end file xorshift32.h
