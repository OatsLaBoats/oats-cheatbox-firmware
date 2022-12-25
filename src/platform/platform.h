#pragma once

#include "keycodes.h"
#include "gamepad_buttons.h"

typedef void (*TaskCallback)(void);

// Add some more input modes (xinput, DInput)
typedef enum {
     MODE_KEYBOARD,
     MODE_GAMEPAD,
} InputMode;

void platform_init(void);
void platform_set_mode(InputMode mode);
InputMode platform_get_mode(void);
void platform_task(TaskCallback callback, bool save_power);

// Physical button map
/*
              16
         17 18 19 20
         21 22 26 27
              28
 00 01 02 03      04 05 06 07
                  08 09 10 11   
        12 13    14 15
*/

// Returns true if any physical button is pressed or released
bool has_input(void);

// Physical button functions
bool button_down(int index);
bool button_up(int index);
bool button_pressed(int index);
bool button_released(int index);

// Bootsel button
bool board_button_down(void);
bool board_button_up(void);
bool board_button_pressed(void);
bool board_button_released(void);

void keyboard_press(KeyCode key);
void keyboard_release(KeyCode key);
void keyboard_release_all(void);

void gamepad_left_stick(i8 x, i8 y);
void gamepad_right_stick(i8 x, i8 y);
void gamepad_left_trigger(u8 strength);
void gamepad_right_trigger(u8 strength);
void gamepad_dpad(DPadDirection direction);
void gamepad_start(bool state);
void gamepad_select(bool state);
void gamepad_button_press(u32 button);
void gamepad_button_release(u32 button);
