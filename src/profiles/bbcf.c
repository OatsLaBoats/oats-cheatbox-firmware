#include "bbcf.h"

static void _task(Profile *self) {
    release_all();

    if (button_down(1))                    press(LEFT);
    if (button_down(2))                    press(DOWN);
    if (button_down(3))                    press(RIGHT);
    if (button_down(0) || button_down(12)) press(UP);

    if (button_down(4))  press(ATTACK_1);
    if (button_down(5))  press(ATTACK_2);
    if (button_down(6))  press(ATTACK_3);
    if (button_down(7))  press(ATTACK_4);
    if (button_down(8))  press(ATTACK_5);
    if (button_down(9))  press(ATTACK_6);
    if (button_down(10)) press(ATTACK_7);
    if (button_down(11)) press(ATTACK_8);

    if (button_down(13)) press(MACRO_1);
    if (button_down(14)) press(MACRO_2);
    if (button_down(15)) press(MACRO_3);

    if (button_down(28)) press(UTILITY);

    // You can't rebind these so they need dedicated buttons.
    if (button_down(17)) press(SPECIAL_ESCAPE);

    if (button_down(19)) press(EXTRA_3);
    if (button_down(20)) press(EXTRA_4);
    if (button_down(21)) press(SPECIAL_ENTER);
    if (button_down(22)) press(EXTRA_6);
    if (button_down(26)) press(EXTRA_7);
    if (button_down(27)) press(EXTRA_8);
}

Profile create_bbcf_profile(void) {
    return (Profile) {
        .task = _task,
        .socd = SOCD_ABSOLUTE,
        .mode = MODE_KEYBOARD,
    };
}
