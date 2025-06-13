#define MA_NO_DECODING            // no decoder support
#define MA_NO_ENCODING            // no encoder support
#define MA_NO_RESOURCE_MANAGER    // no resource manager
#define MA_NO_NODE_GRAPH          // no node graph (for advanced audio graph features)
#define MA_NO_GENERATION          // no signal generators except sine (your sine is manual anyway)
#define MA_NO_VOICE              // no voice API
#define MA_NO_DEVICE_NOTIFICATION // no device change notifications (optional)
#define MA_NO_CAPTURE            // no capture (recording) support
#define MA_NO_ENGINE             // no engine (full engine API)

#define MINIAUDIO_IMPLEMENTATION

#include "miniaudio.h"
