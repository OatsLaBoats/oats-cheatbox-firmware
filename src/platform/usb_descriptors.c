/* 
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <stdint.h>
#include <tusb.h>

#include "report_ids.h"
#include "../settings.h"
#include "keycodes.h"

/* A combination of interfaces must have a unique product id, since PC will save device driver after the first plug.
 * Same VID/PID with different interface e.g MSC (first), then CDC (later) will possibly cause system error on PC.
 *
 * Auto ProductID layout's Bitmap:
 *   [MSB]         HID | MSC | CDC          [LSB]
 */
#define _PID_MAP(itf, n)  ( (CFG_TUD_##itf) << (n) )
#define USB_PID           (0x4000 | _PID_MAP(CDC, 0) | _PID_MAP(MSC, 1) | _PID_MAP(HID, 2) | \
                           _PID_MAP(MIDI, 3) | _PID_MAP(VENDOR, 4) )

// Device Descriptor
static tusb_desc_device_t const _desc_device = {
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x200,
    .bDeviceClass       = 0x00,
    .bDeviceSubClass    = 0x00,
    .bDeviceProtocol    = 0x00,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor           = 0xCafe,
    .idProduct          = USB_PID,
    .bcdDevice          = 0x0100,

    // These are just indexes to strings
    .iManufacturer      = 1,
    .iProduct           = 2,
    .iSerialNumber      = 3,

    .bNumConfigurations = 1
};

// Invoked when received GET DEVICE DESCRIPTOR
uint8_t const * tud_descriptor_device_cb(void) {
  return (uint8_t const *) &_desc_device;
}

#define USE_WARATAH_DESCRIPTOR 1

#if !(USE_WARATAH_DESCRIPTOR)

// NKRO keyboard descriptor is just a modified version of the tinyusb boot keyboard
#define TUD_HID_REPORT_DESC_NKRO_KEYBOARD(...) \
  HID_USAGE_PAGE ( HID_USAGE_PAGE_DESKTOP     )                    ,\
  HID_USAGE      ( HID_USAGE_DESKTOP_KEYBOARD )                    ,\
  HID_COLLECTION ( HID_COLLECTION_APPLICATION )                    ,\
    /* Report ID if any */\
    __VA_ARGS__ \
    \
    /* 8 bits Modifier Keys (Shfit, Control, Alt) */ \
    HID_USAGE_PAGE ( HID_USAGE_PAGE_KEYBOARD )                     ,\
      HID_USAGE_MIN    ( KEY_CONTROL_LEFT                                    )  ,\
      HID_USAGE_MAX    ( KEY_CONTROL_RIGHT                                    )  ,\
      HID_LOGICAL_MIN  ( 0                                      )  ,\
      HID_LOGICAL_MAX  ( 1                                      )  ,\
      HID_REPORT_COUNT ( 8                                      )  ,\
      HID_REPORT_SIZE  ( 1                                      )  ,\
      HID_INPUT        ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE )  ,\
      \
    /* 12-byte Keycode bitmap 0x04-0x63. If this doesnt work try adding _N variant of macros  */ \
    HID_USAGE_PAGE ( HID_USAGE_PAGE_KEYBOARD )                     ,\
      HID_USAGE_MIN    ( KEY_A                                   )     ,\
      HID_USAGE_MAX    ( KEY_KEYPAD_DECIMAL                              )     ,\
      HID_LOGICAL_MIN  ( 0                                   )     ,\
      HID_LOGICAL_MAX  ( 1                              )     ,\
      HID_REPORT_COUNT ( KEY_KEYPAD_DECIMAL + 1 - KEY_A                                   )     ,\
      HID_REPORT_SIZE  ( 1                                   )     ,\
      HID_INPUT        ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE )     ,\
  HID_COLLECTION_END \


static uint8_t const desc_hid_report[] =
{
  TUD_HID_REPORT_DESC_NKRO_KEYBOARD(HID_REPORT_ID(REPORT_ID_KEYBOARD)),
  TUD_HID_REPORT_DESC_GAMEPAD(HID_REPORT_ID(REPORT_ID_GAMEPAD)),
};

#else

// HID Report Descriptor
// Composite report descriptor for a NKRO keyboard and a gamepad with 2 sticks, 2 triggers, 1 dpad, select & start buttons, 32 general buttons
// Generated with waratah
static uint8_t const _desc_hid_report[] = {
    0x05, 0x01,          // UsagePage(Generic Desktop[1])
    0x09, 0x06,          // UsageId(Keyboard[6])
    0xA1, 0x01,          // Collection(Application)
    0x85, 0x01,          //     ReportId(1)
    0x05, 0x07,          //     UsagePage(Keyboard/Keypad[7])
    0x19, 0xE0,          //     UsageIdMin(Keyboard LeftControl[224])
    0x29, 0xE7,          //     UsageIdMax(Keyboard Right GUI[231])
    0x15, 0x00,          //     LogicalMinimum(0)
    0x25, 0x01,          //     LogicalMaximum(1)
    0x95, 0x08,          //     ReportCount(8)
    0x75, 0x01,          //     ReportSize(1)
    0x81, 0x02,          //     Input(Data, Variable, Absolute, NoWrap, Linear, PreferredState, NoNullPosition, BitField)
    0x19, 0x04,          //     UsageIdMin(Keyboard A[4])
    0x29, 0x63,          //     UsageIdMax(Keypad Period[99])
    0x95, 0x60,          //     ReportCount(96)
    0x81, 0x02,          //     Input(Data, Variable, Absolute, NoWrap, Linear, PreferredState, NoNullPosition, BitField)
    0x05, 0x08,          //     UsagePage(LED[8])
    0x19, 0x01,          //     UsageIdMin(Num Lock[1])
    0x29, 0x05,          //     UsageIdMax(Kana[5])
    0x95, 0x05,          //     ReportCount(5)
    0x91, 0x02,          //     Output(Data, Variable, Absolute, NoWrap, Linear, PreferredState, NoNullPosition, NonVolatile, BitField)
    0x95, 0x01,          //     ReportCount(1)
    0x75, 0x03,          //     ReportSize(3)
    0x91, 0x03,          //     Output(Constant, Variable, Absolute, NoWrap, Linear, PreferredState, NoNullPosition, NonVolatile, BitField)
    0xC0,                // EndCollection()
    0x05, 0x01,          // UsagePage(Generic Desktop[1])
    0x09, 0x05,          // UsageId(Gamepad[5])
    0xA1, 0x01,          // Collection(Application)
    0x85, 0x02,          //     ReportId(2)
    0x09, 0x30,          //     UsageId(X[48])
    0x15, 0x81,          //     LogicalMinimum(-127)
    0x25, 0x7F,          //     LogicalMaximum(127)
    0x75, 0x08,          //     ReportSize(8)
    0x81, 0x02,          //     Input(Data, Variable, Absolute, NoWrap, Linear, PreferredState, NoNullPosition, BitField)
    0x09, 0x31,          //     UsageId(Y[49])
    0x81, 0x02,          //     Input(Data, Variable, Absolute, NoWrap, Linear, PreferredState, NoNullPosition, BitField)
    0x09, 0x32,          //     UsageId(Z[50])
    0x81, 0x02,          //     Input(Data, Variable, Absolute, NoWrap, Linear, PreferredState, NoNullPosition, BitField)
    0x09, 0x35,          //     UsageId(Rz[53])
    0x81, 0x02,          //     Input(Data, Variable, Absolute, NoWrap, Linear, PreferredState, NoNullPosition, BitField)
    0x09, 0x33,          //     UsageId(Rx[51])
    0x81, 0x02,          //     Input(Data, Variable, Absolute, NoWrap, Linear, PreferredState, NoNullPosition, BitField)
    0x09, 0x34,          //     UsageId(Ry[52])
    0x81, 0x02,          //     Input(Data, Variable, Absolute, NoWrap, Linear, PreferredState, NoNullPosition, BitField)
    0x09, 0x39,          //     UsageId(Hat Switch[57])
    0x46, 0x3B, 0x01,    //     PhysicalMaximum(315)
    0x15, 0x01,          //     LogicalMinimum(1)
    0x25, 0x08,          //     LogicalMaximum(8)
    0x75, 0x04,          //     ReportSize(4)
    0x81, 0x42,          //     Input(Data, Variable, Absolute, NoWrap, Linear, PreferredState, NullState, BitField)
    0x81, 0x03,          //     Input(Constant, Variable, Absolute, NoWrap, Linear, PreferredState, NoNullPosition, BitField)
    0x05, 0x09,          //     UsagePage(Button[9])
    0x19, 0x01,          //     UsageIdMin(Button 1[1])
    0x29, 0x20,          //     UsageIdMax(Button 32[32])
    0x45, 0x00,          //     PhysicalMaximum(0)
    0x15, 0x00,          //     LogicalMinimum(0)
    0x25, 0x01,          //     LogicalMaximum(1)
    0x95, 0x20,          //     ReportCount(32)
    0x75, 0x01,          //     ReportSize(1)
    0x81, 0x02,          //     Input(Data, Variable, Absolute, NoWrap, Linear, PreferredState, NoNullPosition, BitField)
    0xC0,                // EndCollection()
};

#endif

// Invoked when received GET HID REPORT DESCRIPTOR
uint8_t const * tud_hid_descriptor_report_cb(uint8_t instance) {
  (void) instance;
  return _desc_hid_report;
}


// HID interfaces
enum {
  ITF_NUM_HID,
  ITF_NUM_TOTAL
};

#define  CONFIG_TOTAL_LEN  (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN)

// Endpoint address
#define EPNUM_HID   0x81

// Configuration Descriptor
uint8_t const _desc_configuration[] = {
  // Config number, interface count, string index, total length, attribute, power in mA
  TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),

  // Interface number, string index, protocol, report descriptor len, EP In address, size & polling interval
  TUD_HID_DESCRIPTOR(ITF_NUM_HID, 4, HID_ITF_PROTOCOL_NONE, sizeof(_desc_hid_report), EPNUM_HID, CFG_TUD_HID_EP_BUFSIZE, POLLING_RATE)
};

#if TUD_OPT_HIGH_SPEED
// Per USB specs: high speed capable device must report device_qualifier and other_speed_configuration

// other speed configuration
static uint8_t _desc_other_speed_config[CONFIG_TOTAL_LEN];

// device qualifier is mostly similar to device descriptor since we don't change configuration based on speed
static tusb_desc_device_qualifier_t const _desc_device_qualifier = {
  .bLength            = sizeof(tusb_desc_device_qualifier_t),
  .bDescriptorType    = TUSB_DESC_DEVICE_QUALIFIER,
  .bcdUSB             = 0x200,

  .bDeviceClass       = 0x00,
  .bDeviceSubClass    = 0x00,
  .bDeviceProtocol    = 0x00,

  .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
  .bNumConfigurations = 0x01,
  .bReserved          = 0x00
};

// Invoked when received GET DEVICE QUALIFIER DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete.
// device_qualifier descriptor describes information about a high-speed capable device that would
// change if the device were operating at the other speed. If not highspeed capable stall this request.
uint8_t const* tud_descriptor_device_qualifier_cb(void) {
  return (uint8_t const*) &_desc_device_qualifier;
}

// Invoked when received GET OTHER SEED CONFIGURATION DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
// Configuration descriptor in the other speed e.g if high speed then this is for full speed and vice versa
uint8_t const* tud_descriptor_other_speed_configuration_cb(uint8_t index) {
  (void) index; // for multiple configurations

  // other speed config is basically configuration with type = OHER_SPEED_CONFIG
  memcpy(_desc_other_speed_config, _desc_configuration, CONFIG_TOTAL_LEN);
  desc_other_speed_config[1] = TUSB_DESC_OTHER_SPEED_CONFIG;

  // this example use the same configuration for both high and full speed mode
  return _desc_other_speed_config;
}

#endif

// Invoked when received GET CONFIGURATION DESCRIPTOR
// index is for multiple configurations
uint8_t const * tud_descriptor_configuration_cb(uint8_t index) {
  (void) index;
  
  // Use the same configuration for both high and full speed mode
  return _desc_configuration;
}

// String Descriptors
static char const* _string_desc_arr [] = {
  (const char[]) { 0x09, 0x04 }, // 0: is supported language is English (0x0409)
  "Oats",                        // 1: Manufacturer
  "Oats Cheatbox",               // 2: Product
  "696969",                      // 3: Serials, should use chip ID
  "Cheatbox Interface",          // 4: Interface
};

static uint16_t _desc_str[32];

// Invoked when received GET STRING DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
  (void) langid;

  uint8_t chr_count;

  if ( index == 0) {
    memcpy(&_desc_str[1], _string_desc_arr[0], 2);
    chr_count = 1;
  }
  else {
    // Note: the 0xEE index string is a Microsoft OS 1.0 Descriptors.
    // https://docs.microsoft.com/en-us/windows-hardware/drivers/usbcon/microsoft-defined-usb-descriptors

    // Bounds check
    if ( !(index < sizeof(_string_desc_arr)/sizeof(_string_desc_arr[0])) ) return NULL;

    const char* str = _string_desc_arr[index];

    // Cap at max char
    chr_count = (uint8_t) strlen(str);
    if ( chr_count > 31 ) chr_count = 31;

    // Convert ASCII string into UTF-16
    for(uint8_t i=0; i<chr_count; i++) {
      _desc_str[1+i] = str[i];
    }
  }

  // first byte is length (including header), second byte is string type
  _desc_str[0] = (uint16_t) ((TUSB_DESC_STRING << 8 ) | (2*chr_count + 2));

  return _desc_str;
}
