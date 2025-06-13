#define MA_NO_DECODING            // no decoder support
#define MA_NO_ENCODING            // no encoder support
#define MA_NO_RESOURCE_MANAGER    // no resource manager
#define MA_NO_NODE_GRAPH          // no node graph (for advanced audio graph features)
#define MA_NO_GENERATION          // no signal generators except sine (your sine is manual anyway)
#define MA_NO_VOICE              // no voice API
#define MA_NO_DEVICE_NOTIFICATION // no device change notifications (optional)
#define MA_NO_CAPTURE            // no capture (recording) support
#define MA_NO_ENGINE             // no engine (full engine API)

#if defined(_WIN32)
    #define MA_NO_WASAPI        0 // enable WASAPI on Windows
    #define MA_NO_DSOUND        1
    #define MA_NO_WINMM         1
#elif defined(__APPLE__)
    #define MA_NO_COREAUDIO     0 // enable CoreAudio on macOS
#elif defined(__linux__)
    #define MA_NO_ALSA          0 // enable ALSA on Linux
    #define MA_NO_PULSEAUDIO    1
    #define MA_NO_JACK          1
#else
    #define MA_NO_NULL          0 // fallback null backend (no real audio)
#endif

#define MINIAUDIO_IMPLEMENTATION

#include "miniaudio.h"
