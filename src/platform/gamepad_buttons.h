#pragma once

// buttons 0-31
#define GAMEPAD_BUTTON(n) (1ul << n)

typedef enum {
    DPAD_CENTERED   = 0,
    DPAD_UP         = 1,
    DPAD_UP_RIGHT   = 2,
    DPAD_RIGHT      = 3,
    DPAD_DOWN_RIGHT = 4,
    DPAD_DOWN       = 5,
    DPAD_DOWN_LEFT  = 6,
    DPAD_LEFT       = 7,
    DPAD_UP_LEFT    = 8,
} DPadDirection;
