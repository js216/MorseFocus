/**
 * @file cw.c
 * @brief Morse code (CW) playback using miniaudio with Farnsworth timing.
 *
 * @author Jakob Kastelic
 */

#include "cw.h"
#include "debug.h"
#include "lib/miniaudio.h"
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define INITIAL_SILENCE 250
#define FADE_LEN 100

#define INTER_GAP 1
#define CHAR_GAP 3
#define DIT_DUR 1
#define DAH_DUR 3
#define WORD_BREAK 7

static const char *const morse_table[] = {
    ['A'] = ".-",      ['B'] = "-...",   ['C'] = "-.-.",   ['D'] = "-..",
    ['E'] = ".",       ['F'] = "..-.",   ['G'] = "--.",    ['H'] = "....",
    ['I'] = "..",      ['J'] = ".---",   ['K'] = "-.-",    ['L'] = ".-..",
    ['M'] = "--",      ['N'] = "-.",     ['O'] = "---",    ['P'] = ".--.",
    ['Q'] = "--.-",    ['R'] = ".-.",    ['S'] = "...",    ['T'] = "-",
    ['U'] = "..-",     ['V'] = "...-",   ['W'] = ".--",    ['X'] = "-..-",
    ['Y'] = "-.--",    ['Z'] = "--..",

    ['0'] = "-----",   ['1'] = ".----",  ['2'] = "..---",  ['3'] = "...--",
    ['4'] = "....-",   ['5'] = ".....",  ['6'] = "-....",  ['7'] = "--...",
    ['8'] = "---..",   ['9'] = "----.",

    ['.'] = ".-.-.-",  [','] = "--..--", ['?'] = "..--..", ['\''] = ".----.",
    ['!'] = "-.-.--",  ['/'] = "-..-.",  ['('] = "-.--.",  [')'] = "-.--.-",
    ['&'] = ".-...",   [':'] = "---...", [';'] = "-.-.-.", ['='] = "-...-",
    ['+'] = ".-.-.",   ['-'] = "-....-", ['_'] = "..--.-", ['"'] = ".-..-.",
    ['$'] = "...-..-", ['@'] = ".--.-."};

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

      c = toupper(c);

      const int tbl_size = sizeof(morse_table) / sizeof(morse_table[0]);
      const char *mc = (c < tbl_size) ? morse_table[c] : NULL;
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
            units += INTER_GAP;
         break;
      case '-':
         units += DAH_DUR;
         if (p[1] == '.' || p[1] == '-')
            units += INTER_GAP;
         break;
      case '|':
         units += CHAR_GAP;
         break;
      case '/':
         if (p[1] != '\0')
            units += WORD_BREAK;
         break;
      default:
         ERROR("invalid case");
         return -1;
      }
      p++;
   }

   return units;
}

static void start_symbol_tone(struct cw_data *cw, char sym)
{
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

// Callback from miniaudio.h library, cannot change prototype:
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
static void data_callback(ma_device *pDevice, void *pOutput, const void *pInput,
                          ma_uint32 frameCount)
{
   (void)pInput;
   struct cw_data *cw = (struct cw_data *)pDevice->pUserData;
   float *out = (float *)pOutput;

   static int initial = -1;
   if (initial == -1) {
      initial = cw->delay_frames;
   } else if (initial > 0) {
      initial--;
      memset(out, 0, sizeof(float) * 2 * frameCount);
      return;
   }

   for (size_t i = 0; i < frameCount; i++) {
      float sample = 0.0F;

      if (cw->tone_samples > 0) {
         unsigned long long tone_played =
             cw->tone_len - cw->tone_samples; // samples played in current tone
         float fade_factor = 1.0F;

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
         sample =
             fade_factor * cw->amp * sinf(2.0F * (float)M_PI * cw->freq * r);

         cw->tone_samples--;
      }

      // when waiting in silence
      else if (cw->gap_samples > 0) {
         sample = 0.0F;
         cw->gap_samples--;
      }

      else if (cw->morse[cw->pos]) {
         char sym = cw->morse[cw->pos++];
         start_symbol_tone(cw, sym);
         sample = 0.0F;
      } else {
         sample = 0.0F;
      }

      out[i * 2] = sample;
      out[(i * 2) + 1] = sample;
      cw->total_samples++;
   }
}

static int setup_audio_device(struct cw_data *cw, ma_device *dev)
{
   ma_device_config cfg = ma_device_config_init(ma_device_type_playback);
   cfg.playback.format = ma_format_f32;
   cfg.playback.channels = 2;
   cfg.sampleRate = 48000;
   cfg.periodSizeInFrames = 64;
   cfg.periods = 1;
   cfg.dataCallback = data_callback;

   const float sr = (float)cfg.sampleRate;
   const float ratio = sr / (float)cfg.periodSizeInFrames;

   // Calculate delay_frames if not set yet (optional)
   if (cw->delay_frames == 0 && cw->delay_sec > 0.0F) {
      cw->delay_frames = (int)(cw->delay_sec * ratio);
   }

   float dot_dur = 60.0F / (50.0F * cw->speed1);
   float gap_dur = 60.0F / (50.0F * cw->speed2);

   cw->dot_len = (int)(dot_dur * sr);
   cw->intra_gap = cw->dot_len;
   cw->inter_gap = (int)(gap_dur * sr);

   cfg.pUserData = cw;

   if (ma_device_init(NULL, &cfg, dev) != MA_SUCCESS)
      return -1;

   if (ma_device_start(dev) != MA_SUCCESS) {
      ma_device_uninit(dev);
      return -1;
   }

   return (int)cfg.sampleRate;
}

static char *prepare_morse(const char *str)
{
   char *morse = malloc((strlen(str) * 10) + 1);
   if (!morse) {
      ERROR("out of memory");
      return NULL;
   }

   ascii_to_morse_expanded(str, morse);
   return morse;
}

static int wait_for_playback_to_finish(struct cw_data *cw)
{
   while (cw->morse[cw->pos] || cw->tone_samples > 0 || cw->gap_samples > 0) {
      ma_sleep(10);
   }

   return 0;
}

int cw_play(const char *str, struct cw_data *cw)
{
   if (!str || !cw) {
      ERROR("invalid parameters given");
      return -1;
   }

   if (cw->speed1 <= 0 || cw->speed2 <= 0 || cw->freq <= 0 || cw->amp <= 0) {
      ERROR("invalid parameters given");
      return -1;
   }

   if (cw->speed1 < cw->speed2) {
      ERROR("speed1 must be equal or greater than speed2");
      return -1;
   }

   char *morse = prepare_morse(str);
   if (!morse) {
      return -1;
   }

   ma_device dev;

   // Copy morse pointer into the struct for later use
   cw->morse = morse;

   const int sr = setup_audio_device(cw, &dev);
   if (sr < 0) {
      ERROR("audio device setup failed");
      free(morse);
      return -1;
   }

   wait_for_playback_to_finish(cw);

   ma_device_uninit(&dev);
   free(morse);

   return (int)((cw->total_samples * 1000ULL) / sr);
}

float cw_duration(const char *str, const float speed1, const float speed2)
{
   if (!str || speed1 <= 0.0F || speed2 <= 0.0F || speed1 < speed2)
      return -1.0F;

   size_t len = strlen(str);
   char *morse = malloc((len * 10) + 1);
   if (!morse)
      return -1.0F;

   ascii_to_morse_expanded(str, morse);
   const char *p = morse;

   float dot_dur = 60.0F / (50.0F * speed1);
   float gap_dur = 60.0F / (50.0F * speed2);

   float total = 0.0F;

   while (*p) {
      switch (*p) {
      case '.':
         total += dot_dur;
         if (p[1] == '.' || p[1] == '-')
            total += dot_dur;
         break;
      case '-':
         total += 3 * dot_dur;
         if (p[1] == '.' || p[1] == '-')
            total += dot_dur;
         break;
      case '|':
         total += 3 * gap_dur;
         break;
      case '/':
         if (p[1] != '\0')
            total += 7 * gap_dur;
         break;
      default:
         free(morse);
         return -1.0F;
      }
      p++;
   }

   free(morse);
   return total;
}

// end of cw.c
