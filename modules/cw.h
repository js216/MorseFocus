/**
 * @file cw.h
 * @brief Generating Morse code audio.
 *
 * @author Jakob Kastelic
 */

#ifndef CW_H
#define CW_H

struct cw_data {
   const char *morse; // pointer to expanded Morse code string
   int pos;           // current character position in Morse string

   int tone_samples; // number of samples left in current tone
   int tone_len;     // duration of current tone in samples
   int gap_samples;  // number of samples left in current gap

   int dot_len;   // duration of a dot in samples
   int intra_gap; // duration of intra-character gap in samples
   int inter_gap; // duration of inter-character/word gap in samples

   float freq;       // tone frequency in Hz
   float amp;        // tone amplitude from 0 to 1
   int delay_frames; // initial delay, in frames

   float delay_sec; // initial delay, in seconds
   float speed1;    // Farnsworth speed 1 (WPM)
   float speed2;    // Farnsworth speed 2 (WPM)

   unsigned long long total_samples; // total number of samples played
};

/**
 * @brief Convert an ASCII string to an expanded Morse code string.
 *
 * This function converts each ASCII character in the input string to its
 * Morse code representation using dots ('.') and dashes ('-'). It inserts
 * character gaps ('|') between letters and word gaps ('/') between words.
 *
 * @param[in] in  Null-terminated ASCII input string to convert.
 * @param[out] out Buffer to receive the expanded Morse code string. It must
 *                 be large enough to hold the result.
 */
void ascii_to_morse_expanded(const char *in, char *out);

/**
 * @brief Count total Morse code time units in an expanded Morse code string.
 *
 * This function computes the total number of time units required to transmit
 * the given expanded Morse code string, including dots, dashes, intra-character
 * gaps, character gaps, and word gaps.
 *
 * The input Morse code string uses the symbols:
 * - '.' for dot
 * - '-' for dash
 * - '|' for character gap
 * - '/' for word gap
 *
 * @param[in] morse Null-terminated expanded Morse code string.
 * @return Total number of time units for transmitting the Morse code, or -1 on
 * error.
 */
int count_units(const char *morse);

/**
 * @brief Play a Morse code string as audio using miniaudio.
 *
 * @param str Null-terminated input string to transmit (ASCII).
 * @param cw Pointer to a configured cw_data struct with playback parameters:
 *           - freq: Audio tone frequency in Hz.
 *           - amp: Audio tone amplitude from 0 to 1.
 *           - delay_sec: Initial delay in seconds.
 *           - speed1: Character speed in words per minute (WPM).
 *           - speed2: Farnsworth speed in WPM (<= speed1).
 *           - morse: (will be set internally)
 * @return Total playback duration in milliseconds, or -1 on error.
 */
int cw_play(const char *str, struct cw_data *cw);

/**
 * @brief Compute the CW transmission duration in seconds.
 *
 * Uses the same timing rules as cw_play() without audio playback.
 *
 * @param str     Input text string to encode and transmit as CW.
 * @param speed1  Character speed (WPM).
 * @param speed2  Farnsworth spacing speed (WPM).
 * @return        Transmission time in seconds, or -1.0f on error.
 */
float cw_duration(const char *str, const float speed1, const float speed2);

#endif // CW_H

// end file cw.h
