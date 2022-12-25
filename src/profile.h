#pragma once

#include "platform/platform.h"
#include "virtual_button.h"

#define INVALID_ID -1

typedef struct Profile {
    void (*task)(struct Profile *self);

    SocdType socd;
    InputMode mode;
} Profile;

// Registers a profile and returns its id (from 0 to MAX_PROFILES - 1). Returns INVALID_ID on error.
// The profile is coppied into an internal buffer no need to preserve it outside.
int register_profile(Profile profile);
void select_profile(int id);
Profile *get_active_profile(void);
