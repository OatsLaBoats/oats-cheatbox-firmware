#include "settings.h"

#include "profiles/default.h"
#include "profiles/ggst.h"

static int _id_default = INVALID_ID;
static int _id_ggst = INVALID_ID;

static bool _save_power = true;

// Here you create, register and set your default profiles
static void _init_profiles(void) {
    Profile p_default = create_default_profile();
    _id_default = register_profile(p_default);

    Profile p_ggst = create_ggst_profile();
    _id_ggst = register_profile(p_ggst);
    
    // Set the default profile
    select_profile(_id_default);
}

static void _user_task_callback(void) {
    static bool core_util_button_consumed = false;

    // Handle switching profiles
    if (button_down(28) && button_released(17)) { select_profile(_id_default); return; }
    if (button_down(28) && button_released(18)) { select_profile(_id_ggst);    return; }
    if (button_down(28) && button_released(19)) { select_profile(INVALID_ID);  return; }
    if (button_down(28) && button_released(20)) { select_profile(INVALID_ID);  return; }
    if (button_down(28) && button_released(21)) { select_profile(INVALID_ID);  return; }
    if (button_down(28) && button_released(22)) { select_profile(INVALID_ID);  return; }
    if (button_down(28) && button_released(26)) { select_profile(INVALID_ID);  return; }
    if (button_down(28) && button_released(27)) { select_profile(INVALID_ID);  return; }

    Profile *profile = get_active_profile();
    if (profile == NULL) return;

    // Handle switching between the 4 socd settings
    core_util_button_consumed = false;
    if (button_down(16) && button_released(17)) { profile->socd = SOCD_NATURAL; core_util_button_consumed = true; return; }
    if (button_down(16) && button_released(18)) { profile->socd = SOCD_NEUTRAL; core_util_button_consumed = true; return; }
    if (button_down(16) && button_released(19)) { profile->socd = SOCD_ABSOLUTE; core_util_button_consumed = true; return; }
    if (button_down(16) && button_released(20)) { profile->socd = SOCD_LAST_INPUT; core_util_button_consumed = true; return; }

    // Handle input mode switching
    if (button_down(16) && button_released(21)) { profile->mode = MODE_KEYBOARD; core_util_button_consumed = true; return; }
    if (button_down(16) && button_released(22)) { profile->mode = MODE_GAMEPAD; core_util_button_consumed = true; return; }
    if (button_down(16) && button_released(26)) { profile->mode = 0; core_util_button_consumed = true; return; }
    if (button_down(16) && button_released(27)) { profile->mode = 0; core_util_button_consumed = true; return; }
    
    if (button_down(16)) return;

    // Toggle power saving mode for debug purposes
    // if (!core_util_button_consumed && button_released(16)) { _save_power = !_save_power; return; }

    // Update the platform mode because the profile might have changed or been updated
    profile->task(profile);
    
    if (platform_get_mode() != profile->mode) {
        platform_set_mode(profile->mode);
    }

    send_inputs(profile->socd);
}

int main(void) {
    platform_init();
    _init_profiles();

    for (;;) {
        platform_task(_user_task_callback, _save_power);
    }

    return 0;
}
