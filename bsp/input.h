#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

typedef enum _bsp_input_event_type {
    INPUT_EVENT_TYPE_NONE       = 0,
    INPUT_EVENT_TYPE_NAVIGATION = 1,  // Keyboard: navigation keys
    INPUT_EVENT_TYPE_KEYBOARD   = 2,  // Keyboard: ASCII/UTF-8
    INPUT_EVENT_TYPE_ACTION     = 3,  // Other actions (e.g. power button)
    INPUT_EVENT_TYPE_SCANCODE   = 4,  // Keyboard: PC scancodes
    INPUT_EVENT_TYPE_LAST,
} bsp_input_event_type_t;

#define BSP_INPUT_NUM_SCANCODES 122

typedef enum _bsp_input_scancode {
    BSP_INPUT_SCANCODE_NONE                = 0x00,
    BSP_INPUT_SCANCODE_ESC                 = 0x01,
    BSP_INPUT_SCANCODE_1                   = 0x02,
    BSP_INPUT_SCANCODE_2                   = 0x03,
    BSP_INPUT_SCANCODE_3                   = 0x04,
    BSP_INPUT_SCANCODE_4                   = 0x05,
    BSP_INPUT_SCANCODE_5                   = 0x06,
    BSP_INPUT_SCANCODE_6                   = 0x07,
    BSP_INPUT_SCANCODE_7                   = 0x08,
    BSP_INPUT_SCANCODE_8                   = 0x09,
    BSP_INPUT_SCANCODE_9                   = 0x0A,
    BSP_INPUT_SCANCODE_0                   = 0x0B,
    BSP_INPUT_SCANCODE_MINUS               = 0x0C,
    BSP_INPUT_SCANCODE_EQUAL               = 0x0D,
    BSP_INPUT_SCANCODE_BACKSPACE           = 0x0E,
    BSP_INPUT_SCANCODE_TAB                 = 0x0F,
    BSP_INPUT_SCANCODE_Q                   = 0x10,
    BSP_INPUT_SCANCODE_W                   = 0x11,
    BSP_INPUT_SCANCODE_E                   = 0x12,
    BSP_INPUT_SCANCODE_R                   = 0x13,
    BSP_INPUT_SCANCODE_T                   = 0x14,
    BSP_INPUT_SCANCODE_Y                   = 0x15,
    BSP_INPUT_SCANCODE_U                   = 0x16,
    BSP_INPUT_SCANCODE_I                   = 0x17,
    BSP_INPUT_SCANCODE_O                   = 0x18,
    BSP_INPUT_SCANCODE_P                   = 0x19,
    BSP_INPUT_SCANCODE_LEFTBRACE           = 0x1A,
    BSP_INPUT_SCANCODE_RIGHTBRACE          = 0x1B,
    BSP_INPUT_SCANCODE_ENTER               = 0x1C,
    BSP_INPUT_SCANCODE_LEFTCTRL            = 0x1D,
    BSP_INPUT_SCANCODE_A                   = 0x1E,
    BSP_INPUT_SCANCODE_S                   = 0x1F,
    BSP_INPUT_SCANCODE_D                   = 0x20,
    BSP_INPUT_SCANCODE_F                   = 0x21,
    BSP_INPUT_SCANCODE_G                   = 0x22,
    BSP_INPUT_SCANCODE_H                   = 0x23,
    BSP_INPUT_SCANCODE_J                   = 0x24,
    BSP_INPUT_SCANCODE_K                   = 0x25,
    BSP_INPUT_SCANCODE_L                   = 0x26,
    BSP_INPUT_SCANCODE_SEMICOLON           = 0x27,
    BSP_INPUT_SCANCODE_APOSTROPHE          = 0x28,
    BSP_INPUT_SCANCODE_GRAVE               = 0x29,
    BSP_INPUT_SCANCODE_LEFTSHIFT           = 0x2A,
    BSP_INPUT_SCANCODE_BACKSLASH           = 0x2B,
    BSP_INPUT_SCANCODE_Z                   = 0x2C,
    BSP_INPUT_SCANCODE_X                   = 0x2D,
    BSP_INPUT_SCANCODE_C                   = 0x2E,
    BSP_INPUT_SCANCODE_V                   = 0x2F,
    BSP_INPUT_SCANCODE_B                   = 0x30,
    BSP_INPUT_SCANCODE_N                   = 0x31,
    BSP_INPUT_SCANCODE_M                   = 0x32,
    BSP_INPUT_SCANCODE_COMMA               = 0x33,
    BSP_INPUT_SCANCODE_DOT                 = 0x34,
    BSP_INPUT_SCANCODE_SLASH               = 0x35,
    BSP_INPUT_SCANCODE_RIGHTSHIFT          = 0x36,
    BSP_INPUT_SCANCODE_KPASTERISK          = 0x37,
    BSP_INPUT_SCANCODE_LEFTALT             = 0x38,
    BSP_INPUT_SCANCODE_SPACE               = 0x39,
    BSP_INPUT_SCANCODE_CAPSLOCK            = 0x3A,
    BSP_INPUT_SCANCODE_F1                  = 0x3B,
    BSP_INPUT_SCANCODE_F2                  = 0x3C,
    BSP_INPUT_SCANCODE_F3                  = 0x3D,
    BSP_INPUT_SCANCODE_F4                  = 0x3E,
    BSP_INPUT_SCANCODE_F5                  = 0x3F,
    BSP_INPUT_SCANCODE_F6                  = 0x40,
    BSP_INPUT_SCANCODE_F7                  = 0x41,
    BSP_INPUT_SCANCODE_F8                  = 0x42,
    BSP_INPUT_SCANCODE_F9                  = 0x43,
    BSP_INPUT_SCANCODE_F10                 = 0x44,
    BSP_INPUT_SCANCODE_NUMLOCK             = 0x45,
    BSP_INPUT_SCANCODE_SCROLLLOCK          = 0x46,
    BSP_INPUT_SCANCODE_KP7                 = 0x47,
    BSP_INPUT_SCANCODE_KP8                 = 0x48,
    BSP_INPUT_SCANCODE_KP9                 = 0x49,
    BSP_INPUT_SCANCODE_KPMINUS             = 0x4A,
    BSP_INPUT_SCANCODE_KP4                 = 0x4B,
    BSP_INPUT_SCANCODE_KP5                 = 0x4C,
    BSP_INPUT_SCANCODE_KP6                 = 0x4D,
    BSP_INPUT_SCANCODE_KPPLUS              = 0x4E,
    BSP_INPUT_SCANCODE_KP1                 = 0x4F,
    BSP_INPUT_SCANCODE_KP2                 = 0x50,
    BSP_INPUT_SCANCODE_KP3                 = 0x51,
    BSP_INPUT_SCANCODE_KP0                 = 0x52,
    BSP_INPUT_SCANCODE_KPDOT               = 0x53,
    BSP_INPUT_SCANCODE_SYSREQ              = 0x54,
    BSP_INPUT_SCANCODE_FN                  = 0x55,
    BSP_INPUT_SCANCODE_F11                 = 0x57,
    BSP_INPUT_SCANCODE_F12                 = 0x58,
    BSP_INPUT_SCANCODE_ESCAPED_PREV_TRACK  = 0xe010,
    BSP_INPUT_SCANCODE_ESCAPED_NEXT_TRACK  = 0xe019,
    BSP_INPUT_SCANCODE_ESCAPED_KPENTER     = 0xe01c,
    BSP_INPUT_SCANCODE_ESCAPED_RCTRL       = 0xe01d,
    BSP_INPUT_SCANCODE_ESCAPED_VOLUME_MUTE = 0xe020,
    BSP_INPUT_SCANCODE_ESCAPED_CALCULATOR  = 0xe021,
    BSP_INPUT_SCANCODE_ESCAPED_PLAY        = 0xe022,
    BSP_INPUT_SCANCODE_ESCAPED_FAKE_LSHIFT = 0xe02a,
    BSP_INPUT_SCANCODE_ESCAPED_VOLUME_DOWN = 0xe02e,
    BSP_INPUT_SCANCODE_ESCAPED_VOLUME_UP   = 0xe030,
    BSP_INPUT_SCANCODE_ESCAPED_KPMINUS     = 0xe035,
    BSP_INPUT_SCANCODE_ESCAPED_FAKE_RSHIFT = 0xe036,
    BSP_INPUT_SCANCODE_ESCAPED_CTRL_PRTSCN = 0xe037,
    BSP_INPUT_SCANCODE_ESCAPED_RALT        = 0xe038,
    BSP_INPUT_SCANCODE_ESCAPED_CTRL_BREAK  = 0xe046,
    BSP_INPUT_SCANCODE_ESCAPED_GREY_HOME   = 0xe047,
    BSP_INPUT_SCANCODE_ESCAPED_GREY_UP     = 0xe048,
    BSP_INPUT_SCANCODE_ESCAPED_GREY_PGUP   = 0xe049,
    BSP_INPUT_SCANCODE_ESCAPED_GREY_LEFT   = 0xe04b,
    BSP_INPUT_SCANCODE_ESCAPED_GREY_RIGHT  = 0xe04d,
    BSP_INPUT_SCANCODE_ESCAPED_GREY_END    = 0xe04f,
    BSP_INPUT_SCANCODE_ESCAPED_GREY_DOWN   = 0xe050,
    BSP_INPUT_SCANCODE_ESCAPED_GREY_PGDN   = 0xe051,
    BSP_INPUT_SCANCODE_ESCAPED_GREY_INSERT = 0xe052,
    BSP_INPUT_SCANCODE_ESCAPED_GREY_DEL    = 0xe053,
    BSP_INPUT_SCANCODE_ESCAPED_LEFTMETA    = 0xe05b,
    BSP_INPUT_SCANCODE_ESCAPED_RIGHTMETA   = 0xe05c,
    BSP_INPUT_SCANCODE_ESCAPED_MENU        = 0xe05d,
    BSP_INPUT_SCANCODE_ESCAPED_SLEEP       = 0xe05f,
    BSP_INPUT_SCANCODE_ESCAPED_STOP        = 0xe068,
    BSP_INPUT_SCANCODE_ESCAPED_FORWARD     = 0xe069,
    BSP_INPUT_SCANCODE_ESCAPED_BACK        = 0xe06a,
    BSP_INPUT_SCANCODE_ESCAPED_MYCOMPUTER  = 0xe06b,
    BSP_INPUT_SCANCODE_ESCAPED_MAIL        = 0xe06c,
    BSP_INPUT_SCANCODE_ESCAPED_MEDIA       = 0xe06d,
} bsp_input_scancode_t;

#define BSP_INPUT_SCANCODE_RELEASE_MODIFIER (0x80)

typedef enum _bsp_input_navigation_key {
    BSP_INPUT_NAVIGATION_KEY_NONE = 0,

    // Navigation keys
    BSP_INPUT_NAVIGATION_KEY_ESC,
    BSP_INPUT_NAVIGATION_KEY_LEFT,
    BSP_INPUT_NAVIGATION_KEY_RIGHT,
    BSP_INPUT_NAVIGATION_KEY_UP,
    BSP_INPUT_NAVIGATION_KEY_DOWN,
    BSP_INPUT_NAVIGATION_KEY_HOME,
    BSP_INPUT_NAVIGATION_KEY_END,
    BSP_INPUT_NAVIGATION_KEY_PGUP,
    BSP_INPUT_NAVIGATION_KEY_PGDN,
    BSP_INPUT_NAVIGATION_KEY_MENU,
    BSP_INPUT_NAVIGATION_KEY_START,
    BSP_INPUT_NAVIGATION_KEY_SELECT,
    BSP_INPUT_NAVIGATION_KEY_RETURN,
    BSP_INPUT_NAVIGATION_KEY_SUPER,
    BSP_INPUT_NAVIGATION_KEY_TAB,
    BSP_INPUT_NAVIGATION_KEY_BACKSPACE,
    BSP_INPUT_NAVIGATION_KEY_SPACE_L,
    BSP_INPUT_NAVIGATION_KEY_SPACE_M,
    BSP_INPUT_NAVIGATION_KEY_SPACE_R,

    // Function keys
    BSP_INPUT_NAVIGATION_KEY_F1,
    BSP_INPUT_NAVIGATION_KEY_F2,
    BSP_INPUT_NAVIGATION_KEY_F3,
    BSP_INPUT_NAVIGATION_KEY_F4,
    BSP_INPUT_NAVIGATION_KEY_F5,
    BSP_INPUT_NAVIGATION_KEY_F6,
    BSP_INPUT_NAVIGATION_KEY_F7,
    BSP_INPUT_NAVIGATION_KEY_F8,
    BSP_INPUT_NAVIGATION_KEY_F9,
    BSP_INPUT_NAVIGATION_KEY_F10,
    BSP_INPUT_NAVIGATION_KEY_F11,
    BSP_INPUT_NAVIGATION_KEY_F12,

    // Gamepad
    BSP_INPUT_NAVIGATION_KEY_GAMEPAD_A,
    BSP_INPUT_NAVIGATION_KEY_GAMEPAD_B,
    BSP_INPUT_NAVIGATION_KEY_GAMEPAD_X,
    BSP_INPUT_NAVIGATION_KEY_GAMEPAD_Y,
    BSP_INPUT_NAVIGATION_KEY_JOYSTICK_PRESS,

    // Volume keys
    BSP_INPUT_NAVIGATION_KEY_VOLUME_UP,
    BSP_INPUT_NAVIGATION_KEY_VOLUME_DOWN,
} bsp_input_navigation_key_t;

typedef enum _bsp_input_action_type {
    BSP_INPUT_ACTION_TYPE_NONE = 0,
    BSP_INPUT_ACTION_TYPE_SD_CARD,
    BSP_INPUT_ACTION_TYPE_AUDIO_JACK,
    BSP_INPUT_ACTION_TYPE_POWER_BUTTON,
    BSP_INPUT_ACTION_TYPE_FPGA_CDONE,
    BSP_INPUT_ACTION_TYPE_PMIC_FAULT,
} bsp_input_action_type_t;

// Modifiers
#define BSP_INPUT_MODIFIER_CAPSLOCK  (1 << 0)
#define BSP_INPUT_MODIFIER_SHIFT_L   (1 << 1)
#define BSP_INPUT_MODIFIER_SHIFT_R   (1 << 2)
#define BSP_INPUT_MODIFIER_SHIFT     (BSP_INPUT_MODIFIER_SHIFT_L | BSP_INPUT_MODIFIER_SHIFT_R)
#define BSP_INPUT_MODIFIER_UPPERCASE (BSP_INPUT_MODIFIER_SHIFT | BSP_INPUT_MODIFIER_CAPSLOCK)
#define BSP_INPUT_MODIFIER_CTRL_L    (1 << 3)
#define BSP_INPUT_MODIFIER_CTRL_R    (1 << 4)
#define BSP_INPUT_MODIFIER_CTRL      (BSP_INPUT_MODIFIER_CTRL_L | BSP_INPUT_MODIFIER_CTRL_R)
#define BSP_INPUT_MODIFIER_ALT_L     (1 << 5)
#define BSP_INPUT_MODIFIER_ALT_R     (1 << 6)
#define BSP_INPUT_MODIFIER_ALT       (BSP_INPUT_MODIFIER_ALT_L | BSP_INPUT_MODIFIER_ALT_R)
#define BSP_INPUT_MODIFIER_SUPER_L   (1 << 7)
#define BSP_INPUT_MODIFIER_SUPER_R   (1 << 8)
#define BSP_INPUT_MODIFIER_SUPER     (BSP_INPUT_MODIFIER_SUPER_L | BSP_INPUT_MODIFIER_SUPER_R)
#define BSP_INPUT_MODIFIER_FUNCTION  (1 << 9)

typedef struct _bsp_input_event_args_navigation {
    bsp_input_navigation_key_t key;
    uint32_t                   modifiers;
    bool                       state;
} bsp_input_event_args_navigation_t;

typedef struct _bsp_input_event_args_keyboard {
    char        ascii;
    char const* utf8;
    uint32_t    modifiers;
} bsp_input_event_args_keyboard_t;

typedef struct _bsp_input_event_args_action {
    bsp_input_action_type_t type;
    bool                    state;
} bsp_input_event_args_action_t;

typedef struct _bsp_input_event_args_scancode {
    bsp_input_scancode_t scancode;
} bsp_input_event_args_scancode_t;

typedef struct _bsp_input_event {
    bsp_input_event_type_t type;
    union {
        bsp_input_event_args_navigation_t args_navigation;
        bsp_input_event_args_keyboard_t   args_keyboard;
        bsp_input_event_args_action_t     args_action;
        bsp_input_event_args_scancode_t   args_scancode;
    };
} bsp_input_event_t;

/// @brief Get the queue handle for the input event queue
/// @return ESP-IDF error code
esp_err_t bsp_input_get_queue(QueueHandle_t* out_queue);

/// @brief Get whether or not the device needs an on-screen keyboard
/// @return true if the device needs an on-screen keyboard and false if it does not
bool bsp_input_needs_on_screen_keyboard(void);

/// @brief Get keyboard backlight brightness
/// @return ESP-IDF error code
esp_err_t bsp_input_get_backlight_brightness(uint8_t* out_percentage);

/// @brief Set keyboard backlight brightness
/// @return ESP-IDF error code
esp_err_t bsp_input_set_backlight_brightness(uint8_t percentage);

/// @brief Read the current state of a navigation key
/// @return ESP-IDF error code
esp_err_t bsp_input_read_navigation_key(bsp_input_navigation_key_t key, bool* out_state);

/// @brief Read the current state of a key by scancode
/// @return ESP-IDF error code
esp_err_t bsp_input_read_scancode(bsp_input_scancode_t key, bool* out_state);

/// @brief Read the current state of an action
/// @return ESP-IDF error code
esp_err_t bsp_input_read_action(bsp_input_action_type_t action, bool* out_state);
