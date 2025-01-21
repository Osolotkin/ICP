#pragma once

#include "error.h"

namespace Log {

    enum Level {
        INFO    = 0x1,
        WARNING = 0x2,
        ERROR   = 0x4
    };

    void log(const Level type, const char* const message);
    void logError(const Err::Err error);

}

