#pragma once

#include <corecrt_math_defines.h>
#include "ControlledUnit.h"

#define KEEP_ANGLE_IN_RANGE(angle) {\
    if ((angle) > 2 * M_PI) {\
        angle = (angle) - 2 * M_PI;\
    } else if ((angle) < 0) {\
        angle = 2 * M_PI + (angle);\
    }\
}

struct Player : ControlledUnit {

};
