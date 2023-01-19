#include "virtual_button.h"
#include "platform/platform.h"

static u64 _state = 0;
static u64 _last_state = 0;

void press(VirtualButton button) {
    _state |= (1 << button);
}

void release(VirtualButton button) {
    _state &= ~(1 << button);
}

void release_all(void) {
    _state = 0;
}

void toggle(VirtualButton button) {
    _state ^= (1 << button);
}

static inline bool _down(VirtualButton button) {
    return !!(_state & (1 << button));
}

static inline bool _pressed(VirtualButton button) {
    return (_state & (1 << button)) && !(_last_state & (1 << button));
}

static DPadDirection _vec2_to_dpad_direction(int x, int y) {
    DPadDirection result = 0;

    if (x == -1 && y == -1)     result = DPAD_DOWN_LEFT;
    else if (x == -1 && y == 0) result = DPAD_LEFT;
    else if (x == -1 && y == 1) result = DPAD_UP_LEFT;
    else if (x == 0 && y == -1) result = DPAD_DOWN;
    else if (x == 0 && y == 0)  result = DPAD_CENTERED;
    else if (x == 0 && y == 1)  result = DPAD_UP;
    else if (x == 1 && y == -1) result = DPAD_DOWN_RIGHT;
    else if (x == 1 && y == 0)  result = DPAD_RIGHT;
    else if (x == 1 && y == 1)  result = DPAD_UP_RIGHT;

    return result;
}

static void _send_keyboard_input(SocdType socd) {
    switch (socd) {
        case SOCD_NATURAL: {
            if (_down(RIGHT)) keyboard_press(KEY_D);
            if (_down(LEFT))  keyboard_press(KEY_A);
            if (_down(UP))    keyboard_press(KEY_W);
            if (_down(DOWN))  keyboard_press(KEY_S);
        } break;

        case SOCD_NEUTRAL: {
            if (_down(RIGHT) && !_down(LEFT)) keyboard_press(KEY_D);
            if (_down(LEFT) && !_down(RIGHT)) keyboard_press(KEY_A);
            if (_down(UP) && !_down(DOWN))    keyboard_press(KEY_W);
            if (_down(DOWN) && !_down(UP))    keyboard_press(KEY_S);
        } break;

        case SOCD_ABSOLUTE: {
            if (_down(RIGHT) && !_down(LEFT)) keyboard_press(KEY_D);
            if (_down(LEFT) && !_down(RIGHT)) keyboard_press(KEY_A);
            if (_down(UP))                    keyboard_press(KEY_W);
            if (_down(DOWN) && !_down(UP))    keyboard_press(KEY_S);
        } break;

        case SOCD_LAST_INPUT: {
            static bool left_was_first = false;
            static bool right_was_first = false;
            static bool both_were_pressed = false;

            if (!left_was_first && !right_was_first && !both_were_pressed) {
                if (_down(LEFT) && _down(RIGHT)) both_were_pressed = true;
                else if (_down(LEFT))               left_was_first = true;
                else if (_down(RIGHT))              right_was_first = true;
            }

            if (both_were_pressed) { // If both were pressed at the same time resolve to neutral and wait untill they are released
                if (!_down(LEFT) || !_down(RIGHT)) both_were_pressed = false;
            }
            else if (left_was_first) { // If left was pressed first and right was pressed after, resolve to right
                if (!_down(LEFT)) left_was_first = false;

                if (_down(RIGHT))     keyboard_press(KEY_D);
                else if (_down(LEFT)) keyboard_press(KEY_A);
            }
            else if (right_was_first) { // If right was pressed first and left was pressed after, resolve to left
                if (!_down(RIGHT)) right_was_first = false;

                if (_down(LEFT))       keyboard_press(KEY_A);
                else if (_down(RIGHT)) keyboard_press(KEY_D);
            }
            
            if (_down(UP))                    keyboard_press(KEY_W);
            if (_down(DOWN) && !_down(UP))    keyboard_press(KEY_S);
        } break;
    }

    if (_down(ATTACK_1)) keyboard_press(KEY_U);
    if (_down(ATTACK_2)) keyboard_press(KEY_I);
    if (_down(ATTACK_3)) keyboard_press(KEY_O);
    if (_down(ATTACK_4)) keyboard_press(KEY_P);
    if (_down(ATTACK_5)) keyboard_press(KEY_H);
    if (_down(ATTACK_6)) keyboard_press(KEY_J);
    if (_down(ATTACK_7)) keyboard_press(KEY_K);
    if (_down(ATTACK_8)) keyboard_press(KEY_L);

    if (_down(MACRO_1))  keyboard_press(KEY_Q);
    if (_down(MACRO_2))  keyboard_press(KEY_C);
    if (_down(MACRO_3))  keyboard_press(KEY_V);
    if (_down(MACRO_4))  keyboard_press(KEY_B);
    if (_down(MACRO_5))  keyboard_press(KEY_N);

    if (_down(UTILITY))  keyboard_press(KEY_9);

    if (_down(EXTRA_1))  keyboard_press(KEY_1);
    if (_down(EXTRA_2))  keyboard_press(KEY_2);
    if (_down(EXTRA_3))  keyboard_press(KEY_3);
    if (_down(EXTRA_4))  keyboard_press(KEY_4);
    if (_down(EXTRA_5))  keyboard_press(KEY_5);
    if (_down(EXTRA_6))  keyboard_press(KEY_6);
    if (_down(EXTRA_7))  keyboard_press(KEY_7);
    if (_down(EXTRA_8))  keyboard_press(KEY_8);

    if (_down(SPECIAL_ESCAPE))    keyboard_press(KEY_ESCAPE);
    if (_down(SPECIAL_ENTER))     keyboard_press(KEY_ENTER);
    if (_down(SPECIAL_BACKSPACE)) keyboard_press(KEY_BACKSPACE);
    if (_down(SPECIAL_SHIFT))     keyboard_press(KEY_SHIFT_LEFT);
    if (_down(SPECIAL_ALT))       keyboard_press(KEY_ALT_LEFT);
    if (_down(SPECIAL_TAB))       keyboard_press(KEY_TAB);
    if (_down(SPECIAL_CONTROL))   keyboard_press(KEY_CONTROL_LEFT);
    if (_down(SPECIAL_PAGE_UP))   keyboard_press(KEY_PAGE_UP);
    if (_down(SPECIAL_PAGE_DOWN)) keyboard_press(KEY_PAGE_DOWN);
}

// Not sure if I should use the dpad or the left joystick for movement.
static void _send_gamepad_input(SocdType socd) {
    int x = 0;
    int y = 0;
    
    switch (socd) {
        case SOCD_NATURAL:
        case SOCD_NEUTRAL: {
            if (_down(RIGHT) && !_down(LEFT)) x += 1;
            if (_down(LEFT) && !_down(RIGHT)) x -= 1;
            if (_down(UP) && !_down(DOWN))    y += 1;
            if (_down(DOWN) && !_down(UP))    y -= 1;
        } break;

        case SOCD_ABSOLUTE: {
            if (_down(RIGHT) && !_down(LEFT)) x += 1;
            if (_down(LEFT) && !_down(RIGHT)) x -= 1;
            if (_down(UP))                    y += 1;
            if (_down(DOWN) && !_down(UP))    y -= 1;
        } break;

        case SOCD_LAST_INPUT: {
            static bool left_was_first = false;
            static bool right_was_first = false;
            static bool both_were_pressed = false;

            if (!left_was_first && !right_was_first && !both_were_pressed) {
                if (_down(LEFT) && _down(RIGHT)) both_were_pressed = true;
                else if (_down(LEFT))               left_was_first = true;
                else if (_down(RIGHT))              right_was_first = true;
            }

            if (both_were_pressed) { // If both were pressed at the same time resolve to neutral and wait untill they are released
                if (!_down(LEFT) || !_down(RIGHT)) both_were_pressed = false;
            }
            else if (left_was_first) { // If left was pressed first and right was pressed after, resolve to right
                if (!_down(LEFT)) left_was_first = false;

                if (_down(RIGHT))     x += 1;
                else if (_down(LEFT)) x -= 1;
            }
            else if (right_was_first) { // If right was pressed first and left was pressed after, resolve to left
                if (!_down(RIGHT)) right_was_first = false;

                if (_down(LEFT))       x -= 1;
                else if (_down(RIGHT)) x += 1;
            }
            
            if (_down(UP))                    y += 1;
            if (_down(DOWN) && !_down(UP))    y -= 1;
        } break;
    }

    gamepad_dpad(_vec2_to_dpad_direction(x, y));
    
    if (_down(ATTACK_1)) gamepad_button_press(GAMEPAD_BUTTON(0));
    if (_down(ATTACK_2)) gamepad_button_press(GAMEPAD_BUTTON(1));
    if (_down(ATTACK_3)) gamepad_button_press(GAMEPAD_BUTTON(2));
    if (_down(ATTACK_4)) gamepad_button_press(GAMEPAD_BUTTON(3));
    if (_down(ATTACK_5)) gamepad_button_press(GAMEPAD_BUTTON(4));
    if (_down(ATTACK_6)) gamepad_button_press(GAMEPAD_BUTTON(5));
    if (_down(ATTACK_7)) gamepad_button_press(GAMEPAD_BUTTON(6));
    if (_down(ATTACK_8)) gamepad_button_press(GAMEPAD_BUTTON(7));

    if (_down(MACRO_1))  gamepad_button_press(GAMEPAD_BUTTON(8));
    if (_down(MACRO_2))  gamepad_button_press(GAMEPAD_BUTTON(9));
    if (_down(MACRO_3))  gamepad_button_press(GAMEPAD_BUTTON(10));
    if (_down(MACRO_4))  gamepad_button_press(GAMEPAD_BUTTON(11));
    if (_down(MACRO_5))  gamepad_button_press(GAMEPAD_BUTTON(12));

    if (_down(UTILITY))  gamepad_button_press(GAMEPAD_BUTTON(13));

    if (_down(EXTRA_1))  gamepad_button_press(GAMEPAD_BUTTON(14));
    if (_down(EXTRA_2))  gamepad_button_press(GAMEPAD_BUTTON(15));
    if (_down(EXTRA_3))  gamepad_button_press(GAMEPAD_BUTTON(16));
    if (_down(EXTRA_4))  gamepad_button_press(GAMEPAD_BUTTON(17));
    if (_down(EXTRA_5))  gamepad_button_press(GAMEPAD_BUTTON(18));
    if (_down(EXTRA_6))  gamepad_button_press(GAMEPAD_BUTTON(19));
    if (_down(EXTRA_7))  gamepad_button_press(GAMEPAD_BUTTON(20));
    if (_down(EXTRA_8))  gamepad_button_press(GAMEPAD_BUTTON(21));
}

void send_inputs(SocdType socd) {
    InputMode mode = platform_get_mode();

    switch (mode) {
        case MODE_KEYBOARD: _send_keyboard_input(socd); break;
        case MODE_GAMEPAD: _send_gamepad_input(socd); break;
    }

    _last_state = _state;
}
