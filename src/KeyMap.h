#pragma once

namespace KeyMap {

    // 86 -> V
    // 265 -> UP_KEY
    // 263 -> LEFT_KEY
    // 264 -> DOWN_KEY
    // 262 -> RIGHT_KEY
    // 68 -> D_KEY
    // 65 -> A_KEY
    // 49 -> 1
    // 50 -> 2
    // 51 -> 3
    // 32 -> SPACE
    // 70 -> F
    // 67 -> C
    // 87 -> W
    // 83 -> S
    // 88 -> X

    enum KeyMap {

        V_SYNC          = 86,
        ENABLE_MOUSE    = 67,
        FULL_SCREEN     = 70,

        MOVE_FORWARD    = 265,
        MOVE_BACK       = 264,
        MOVE_LEFT       = 65,
        MOVE_RIGHT      = 68,

        ROTATE_UP       = 87,
        ROTATE_DOWN     = 83,
        ROTATE_LEFT     = 263,
        ROTATE_RIGHT    = 262,
        
        FIRE            = 341,
        INTERACT        = 32,

        CAM_1           = 49,
        CAM_2           = 50,
        CAM_3           = 51,

        FACE_TRACKING   = 88,

    };

}
