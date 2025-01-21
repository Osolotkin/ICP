
#pragma once

#define ERR_STR(e) ((char*) Err::str[-e])
#define ERR_PSTR(e) ((char**) &(Err::str[-e]))

namespace Err {
    
    enum Err {

        OK                              = 0,
        MALLOC                          = -1,
        NO_VIDEO_CAPTURE_SOURCE         = -2,
        VIDEO_CAPTURE_NO_FRAME          = -3,
        UNSUPPORTED_COMPRESSION_FORMAT  = -4,

    };

    const char* const str[] = {

        "OK",
        "Malloc doesnt feel good...",
        "No video capture source avaliable...",
        "Video capture failed to load a frame...",
        "Unsupported compression format..."

    };

}
