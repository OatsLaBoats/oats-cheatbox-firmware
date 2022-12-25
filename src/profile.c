#include "profile.h"
#include "settings.h"

static Profile profiles[MAX_PROFILES] = {0};
static int profile_count = 0;
static int active_profile = INVALID_ID;

int register_profile(Profile profile) {
    if (profile_count >= MAX_PROFILES) return INVALID_ID;
    profiles[profile_count++] = profile;
    return profile_count - 1;
}

void select_profile(int id) {
    if (id >= profile_count || id < 0) return;
    active_profile = id;
}

Profile *get_active_profile(void) {
    if (active_profile == INVALID_ID) return NULL;
    return &profiles[active_profile];
}
