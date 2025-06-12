/**
 * @file cw.c
 * @brief Morse code (CW) playback using miniaudio with Farnsworth timing.
 *
 * @author Jakob Kastelic
 */

#ifdef _WIN32
#include <windows.h>
#else
#define _POSIX_C_SOURCE 199309L
#include <unistd.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "miniaudio.h"
#include "cw.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define INITIAL_SILENCE 250
#define FADE_LEN 100

struct cw_data {
   const char *morse;      // pointer to expanded Morse code string
   int pos;                // current character position in Morse string

   int tone_samples;       // number of samples left in current tone
   int tone_len;           // duration of current tone in samples
   int gap_samples;        // number of samples left in current gap

   int dot_len;            // duration of a dot in samples
   int intra_gap;          // duration of intra-character gap in samples
   int inter_gap;          // duration of inter-character/word gap in samples

   float freq;             // tone frequency in Hz
   float amp;              // tone amplitude from 0 to 1
   float delay;            // initial delay

   unsigned long long total_samples; // total number of samples played
};

static const char *morse_table[128] = {
   ['A'] = ".-",    ['B'] = "-...",  ['C'] = "-.-.",  ['D'] = "-..",
   ['E'] = ".",     ['F'] = "..-.",  ['G'] = "--.",   ['H'] = "....",
   ['I'] = "..",    ['J'] = ".---",  ['K'] = "-.-",   ['L'] = ".-..",
   ['M'] = "--",    ['N'] = "-.",    ['O'] = "---",   ['P'] = ".--.",
   ['Q'] = "--.-",  ['R'] = ".-.",   ['S'] = "...",   ['T'] = "-",
   ['U'] = "..-",   ['V'] = "...-",  ['W'] = ".--",   ['X'] = "-..-",
   ['Y'] = "-.--",  ['Z'] = "--..",

   ['0'] = "-----", ['1'] = ".----", ['2'] = "..---", ['3'] = "...--",
   ['4'] = "....-", ['5'] = ".....", ['6'] = "-....", ['7'] = "--...",
   ['8'] = "---..", ['9'] = "----.",

   ['.'] = ".-.-.-", [','] = "--..--", ['?'] = "..--..", ['\''] = ".----.",
   ['!'] = "-.-.--", ['/'] = "-..-.",  ['('] = "-.--.",  [')'] = "-.--.-",
   ['&'] = ".-...",  [':'] = "---...", [';'] = "-.-.-.", ['='] = "-...-",
   ['+'] = ".-.-.",  ['-'] = "-....-", ['_'] = "..--.-", ['"'] = ".-..-.",
   ['$'] = "...-..-", ['@'] = ".--.-."
};

static void sleep_ms(unsigned int ms)
{
#ifdef _WIN32
   Sleep(ms);
#else
   struct timespec ts;
   ts.tv_sec = ms / 1000;
   ts.tv_nsec = (ms % 1000) * 1000000L;
   nanosleep(&ts, NULL);
#endif
}

void ascii_to_morse_expanded(const char *in, char *out)
{
   size_t pos = 0;
   int first_char = 1;

   while (*in) {
      unsigned char c = (unsigned char)*in;

      if (c == ' ') {
         if (!first_char) {
            out[pos++] = '/'; /* Word gap */
         }
         in++;
         first_char = 1;
         continue;
      }

      if (c >= 'a' && c <= 'z') {
         c -= 32;
      }

      const char *mc = (c < 128) ? morse_table[c] : NULL;
      if (!mc) {
         in++;
         continue;
      }

      if (!first_char) {
         out[pos++] = '|'; /* Character gap */
      }

      for (size_t i = 0; mc[i]; i++) {
         out[pos++] = mc[i];
      }
      in++;
      first_char = 0;
   }
   out[pos] = '\0';
}

int count_units(const char *morse)
{
   int units = 0;
   const char *p = morse;

   while (*p) {
      switch (*p) {
         case '.':
            units += 1;
            if (p[1] == '.' || p[1] == '-')
               units += 1; // intra-char gap
            break;
         case '-':
            units += 3;
            if (p[1] == '.' || p[1] == '-')
               units += 1;
            break;
         case '|':
            units += 3; // char gap
            break;
         case '/':
            if (p[1] != '\0')
               units += 7; // only if not final
            break;
      }
      p++;
   }

   return units;
}

static void start_symbol_tone(struct cw_data *cw, char sym) {
   switch (sym) {
      case '.':
         cw->tone_samples = cw->dot_len;
         cw->gap_samples = cw->intra_gap;
         break;
      case '-':
         cw->tone_samples = 3 * cw->dot_len;
         cw->gap_samples = cw->intra_gap;
         break;
      case '|':
         cw->tone_samples = 0;
         cw->gap_samples = cw->inter_gap * 3;
         break;
      case '/':
         cw->tone_samples = 0;
         cw->gap_samples = cw->inter_gap * 7;
         break;
      case ' ':
      default:
         cw->tone_samples = 0;
         cw->gap_samples = cw->intra_gap;
         break;
   }
   cw->tone_len = cw->tone_samples;
}

static void data_callback(ma_device *pDevice, void *pOutput, const void *pInput, ma_uint32 frameCount)
{
   (void)pInput;
   struct cw_data *cw = (struct cw_data *)pDevice->pUserData;
   float *out = (float *)pOutput;

   static int initial = -1;
   if (initial == -1) {
      initial = cw->delay;
   } else if (initial > 0) {
      initial--;
      memset(out, 0, sizeof(float) * 2 * frameCount);
      return;
   }

   for (ma_uint32 i = 0; i < frameCount; i++) {
      float sample = 0.0f;

      if (cw->tone_samples > 0) {
         unsigned long long tone_played = cw->tone_len - cw->tone_samples;  // samples played in current tone
         float fade_factor = 1.0f;

         // Fade-in over first FADE_LEN samples
         if (tone_played < FADE_LEN) {
            fade_factor = (float)tone_played / (float)FADE_LEN;
         }
         // Fade-out over last FADE_LEN samples
         else if (cw->tone_samples < FADE_LEN) {
            fade_factor = (float)cw->tone_samples / (float)FADE_LEN;
         }

         const float sr = (float)pDevice->sampleRate;
         const float r = (float)(cw->total_samples % pDevice->sampleRate) / sr;
         sample = fade_factor * cw->amp * sinf(2.0f * (float)M_PI * cw->freq * r);

         cw->tone_samples--;
      }

      // when waiting in silence
      else if (cw->gap_samples > 0) {
         sample = 0.0f;
         cw->gap_samples--;
      }

      else if (cw->morse[cw->pos]) {
         char sym = cw->morse[cw->pos++];
         start_symbol_tone(cw, sym);
         sample = 0.0f;
      } else {
         sample = 0.0f;
      }

      out[i * 2] = sample;
      out[i * 2 + 1] = sample;
      cw->total_samples++;
   }
}

int cw_play(const char *str, const float speed1, const float speed2,
      const float freq, const float amp, const float delay)
{
   if (!str || speed1 <= 0 || speed2 <= 0 || freq <= 0 || amp <= 0)
      return -1;

   char *morse = malloc(strlen(str) * 10 + 1);
   if (!morse)
      return -1;

   ascii_to_morse_expanded(str, morse);  // You must define this

   ma_device_config cfg = ma_device_config_init(ma_device_type_playback);
   cfg.playback.format = ma_format_f32;
   cfg.playback.channels = 2;
   cfg.dataCallback = data_callback;
   cfg.sampleRate = 48000; // You can omit this to use the default
   cfg.periodSizeInFrames = 64;
   cfg.periods = 1;

   struct cw_data cw = {0};
   cw.morse = morse;
   cw.freq = freq;
   cw.amp = amp;

   cw.delay = delay * ((float)cfg.sampleRate / (float)cfg.periodSizeInFrames);

   float dot_dur = 60.0f / (50.0f * speed1);  // Dot duration
   float gap_dur = 60.0f / (50.0f * speed2);  // Gap unit duration

   float sr = (float)cfg.sampleRate;
   cw.dot_len = (int)(dot_dur * sr);
   cw.intra_gap = cw.dot_len;
   cw.inter_gap = (int)(gap_dur * sr);

   cfg.pUserData = &cw;

   ma_device dev;
   if (ma_device_init(NULL, &cfg, &dev) != MA_SUCCESS) {
      free(morse);
      return -1;
   }

   if (ma_device_start(&dev) != MA_SUCCESS) {
      ma_device_uninit(&dev);
      free(morse);
      return -1;
   }

   while (cw.morse[cw.pos] || cw.tone_samples > 0 || cw.gap_samples > 0) {
      sleep_ms(10);
   }

   ma_device_uninit(&dev);
   free(morse);

   return (int)((cw.total_samples * 1000ULL) / (unsigned long long)cfg.sampleRate);
}

// end of cw.c

