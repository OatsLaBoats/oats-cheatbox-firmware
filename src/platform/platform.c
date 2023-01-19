#include <string.h>
#include <bsp/board.h>
#include <hardware/gpio.h>
#include <pico/stdlib.h>
#include <tusb.h>

#include "../settings.h"
#include "platform.h"
#include "report_ids.h"

enum  {
    BLINK_NOT_MOUNTED = 250,
    BLINK_MOUNTED = 1000,
    BLINK_SUSPENDED = 2500,
};

// Tiny usb handles the report ID so i dont need to include it in the packet.
typedef struct TU_ATTR_PACKED
{
  u8 modifier;
  u8 keycode[12];
} _NKROKeyboardReport;

typedef struct TU_ATTR_PACKED {
    i8 lx;
    i8 ly; // left stick

    i8 rx;
    i8 ry; // right stick

    i8 lt;
    i8 rt; // left/right trigger
    
    u8 dpad;

    u32 buttons;
} _GamepadReport;

typedef struct {
    InputMode mode;

    // true if there is keyboard input to report
    bool has_keyboard_input;

    // true if there is gamepad input to report
    bool has_gamepad_input;
    
    // States of the physical buttons
    uint32_t b_new;
    uint32_t b_old;

    bool board_button_new;
    bool board_button_old;

    bool clean_report;

    // USB report structs
    _NKROKeyboardReport keyboard;
    _GamepadReport gamepad;
} _DeviceState;

static u32 blink_interval_ms = BLINK_NOT_MOUNTED;
static _DeviceState _device = {0};

static void _init_pin(int pin) {
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_IN);
    gpio_pull_up(pin);
}

static void _init_pins(void) {
    for (int gpio = 0; gpio <= 22; ++gpio) {
        _init_pin(gpio);
    }

    for (int gpio = 26; gpio <= 28; ++gpio) {
        _init_pin(gpio);
    }
}

void platform_init(void) {
    board_init();
    tusb_init();
    _init_pins();

    _device.mode = MODE_KEYBOARD;
}

void platform_set_mode(InputMode mode) {
    _device.clean_report = true;
    _device.mode = mode;
}

InputMode platform_get_mode(void) {
    return _device.mode;
}

static void _led_blinking_task(void)
{
    static u32 start_ms = 0;
    static bool led_state = false;

    if (board_millis() - start_ms < blink_interval_ms) return;
    start_ms += blink_interval_ms;

    board_led_write(led_state);
    led_state = 1 - led_state;
}

static void _send_keyboard_input(void) {
    static bool had_input = false;

    if (_device.clean_report) {
        // Send last clean gamepad report when changing modes
        _device.clean_report = false;
        memset(&_device.gamepad, 0, sizeof(_device.gamepad));
        tud_hid_report(REPORT_ID_GAMEPAD, &_device.gamepad, sizeof(_device.gamepad));
        return;
    }

    if (_device.has_keyboard_input) {
        had_input = true;
        tud_hid_report(REPORT_ID_KEYBOARD, &_device.keyboard, sizeof(_device.keyboard));
    }
    else {
        if (had_input) {
            // Send empty report
            memset(&_device.keyboard, 0, sizeof(_device.keyboard));
            tud_hid_report(REPORT_ID_KEYBOARD, &_device.keyboard, sizeof(_device.keyboard));
        }

        had_input = false;
    }
}

static void _send_gamepad_input(void) {
    static bool had_input = false;

    if (_device.clean_report) {
        // Send a last clean keyboard report when changing modes
        _device.clean_report = false;
        memset(&_device.keyboard, 0, sizeof(_device.keyboard));
        tud_hid_report(REPORT_ID_KEYBOARD, &_device.keyboard, sizeof(_device.keyboard));
        return;
    }

    if (_device.has_gamepad_input) {
        had_input = true;
        tud_hid_report(REPORT_ID_GAMEPAD, &_device.gamepad, sizeof(_device.gamepad));
    }
    else {
        if (had_input) {
            // Send empty report
            memset(&_device.gamepad, 0, sizeof(_device.gamepad));
            tud_hid_report(REPORT_ID_GAMEPAD, &_device.gamepad, sizeof(_device.gamepad));
        }

        had_input = false;
    }
}

static void _hid_task(TaskCallback callback, bool save_power) {
    // timer
    u64 interval_us = POLLING_RATE * 1000;
    static u64 start_us = 0;
    u64 time = time_us_64() - start_us;

    if (time < interval_us) {
        u64 sleep_time = interval_us - time;
        
        // If the sleep time is greater than 50us save power. Otherwise busy wait.
        if (sleep_time > 50 && save_power) {
            sleep_us(sleep_time - 50);
        }

        return;
    }
    start_us += interval_us;

    _device.b_old = _device.b_new;
    _device.b_new = ~gpio_get_all();

    _device.board_button_old = _device.board_button_new;
    _device.board_button_new = board_button_read();

    // If the device is suspended and an input was detected, wake it up
    if (tud_suspended() && has_input()) {
        tud_remote_wakeup();
        return;
    }

    if (!tud_hid_ready()) return;

    // Callback to user input handling code
    callback();

    switch (_device.mode) {
        case MODE_KEYBOARD: _send_keyboard_input(); break;
        case MODE_GAMEPAD:  _send_gamepad_input();  break;
    }

    // Clear everything after consuming them
    _device.has_keyboard_input = false;
    _device.has_gamepad_input = false;
    memset(&_device.keyboard, 0, sizeof(_device.keyboard));
    memset(&_device.gamepad, 0, sizeof(_device.gamepad));
}

void platform_task(TaskCallback callback, bool save_power) {
    tud_task();
    _led_blinking_task();
    _hid_task(callback, save_power);
}

bool has_input(void) {
    return _device.b_new || _device.b_old;
}

bool button_down(int index) {
    if (index > 31) return false;
    return !!(_device.b_new & (1 << index));
}

bool button_up(int index) {
    if (index > 31) return false;
    return !(_device.b_new & (1 << index));
}

bool button_pressed(int index) {
    if (index > 31) return false;
    return (_device.b_new & (1 << index)) && !(_device.b_old & (1 << index));
}

bool button_released(int index) {
    if (index > 31) return false;
    return !(_device.b_new & (1 << index)) && (_device.b_old & (1 << index));
}

bool board_button_down(void) {
    return _device.board_button_new;
}

bool board_button_up(void) {
    return !_device.board_button_new;
}

bool board_button_pressed(void) {
    return _device.board_button_new && !_device.board_button_old;
}

bool board_button_released(void) {
    return !_device.board_button_new && _device.board_button_old;
}

void keyboard_press(KeyCode key) {
    if (
        key > KEY_GUI_RIGHT ||
        key < KEY_A ||
        (key > KEY_KEYPAD_DECIMAL && key < KEY_CONTROL_LEFT)
    ) return;

    _device.has_keyboard_input = true;

    if (key > KEY_KEYPAD_DECIMAL) {
        _device.keyboard.modifier |= 1 << (key - KEY_CONTROL_LEFT);
    }
    else {
        _device.keyboard.keycode[((key - KEY_A) / 8)] |= 1 << ((key - KEY_A) % 8);
    }
}

void keyboard_release(KeyCode key) {
    if (
        key > KEY_GUI_RIGHT ||
        key < KEY_A ||
        (key > KEY_KEYPAD_DECIMAL && key < KEY_CONTROL_LEFT)
    ) return;

    _device.has_keyboard_input = true;

    if (key > KEY_KEYPAD_DECIMAL) {
        _device.keyboard.modifier &= ~(1 << (key - KEY_CONTROL_LEFT));
    }
    else {
        _device.keyboard.keycode[((key - KEY_A) / 8)] &= ~(1 << ((key - KEY_A) % 8));
    }
}

void keyboard_release_all(void) {
    _device.has_keyboard_input = true;
    memset(&_device.keyboard, 0, sizeof(_device.keyboard));
}

void gamepad_left_stick(i8 x, i8 y) {
    _device.has_gamepad_input = true;
    _device.gamepad.lx = x;
    _device.gamepad.ly = y;
}

void gamepad_left_stick_x(i8 x) {
    _device.has_gamepad_input = true;
    _device.gamepad.lx = x;
}

void gamepad_left_stick_y(i8 y) {
    _device.has_gamepad_input = true;
    _device.gamepad.ly = y;
}

void gamepad_right_stick(i8 x, i8 y) {
    _device.has_gamepad_input = true;
    _device.gamepad.rx = x;
    _device.gamepad.ry = y;
}

void gamepad_right_stick_x(i8 x) {
    _device.has_gamepad_input = true;
    _device.gamepad.rx = x;
}

void gamepad_right_stick_y(i8 y) {
    _device.has_gamepad_input = true;
    _device.gamepad.ry = y;
}

void gamepad_left_trigger(i8 strength) {
    _device.has_gamepad_input = true;
    _device.gamepad.rx = strength;
}

void gamepad_right_trigger(i8 strength) {
    _device.has_gamepad_input = true;
    _device.gamepad.ry = strength;
}

void gamepad_dpad(DPadDirection direction) {
    _device.has_gamepad_input = true;
    _device.gamepad.dpad = direction;
}

void gamepad_button_press(u32 button) {
    _device.has_gamepad_input = true;
    _device.gamepad.buttons |= button;
}

void gamepad_button_release(u32 button) {
    _device.has_gamepad_input = true;
    _device.gamepad.buttons &= ~button;
}

// TinyUSB Callbacks
void tud_mount_cb(void) {
    blink_interval_ms = BLINK_MOUNTED;
}

void tud_umount_cb(void) {
    blink_interval_ms = BLINK_NOT_MOUNTED;
}

void tud_suspend_cb(bool remote_wakeup_en) {
    (void) remote_wakeup_en;
    blink_interval_ms = BLINK_SUSPENDED;
}

void tud_resume_cb(void) {
    blink_interval_ms = BLINK_MOUNTED;
}

u16 tud_hid_get_report_cb(u8 itf, u8 report_id, hid_report_type_t report_type, u8* buffer, u16 reqlen)
{
    (void) itf;
    (void) report_id;
    (void) report_type;
    (void) buffer;
    (void) reqlen;    

    return 0;
}

void tud_hid_set_report_cb(u8 itf, u8 report_id, hid_report_type_t report_type, u8 const* buffer, u16 bufsize)
{
    (void) itf;
    (void) report_id;
    (void) report_type;
    (void) buffer;
    (void) bufsize;
}
