#pragma once

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include <stdbool.h>
#include <stdint.h>

typedef enum _bsp_input_event_type {
    INPUT_EVENT_TYPE_NONE       = 0,
    INPUT_EVENT_TYPE_NAVIGATION = 1,
    INPUT_EVENT_TYPE_KEYBOARD   = 2,
    INPUT_EVENT_TYPE_ACTION     = 3,
    INPUT_EVENT_TYPE_LAST,
} bsp_input_event_type_t;

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
    BSP_INPUT_NAVIGATION_KEY_RETURN,
    BSP_INPUT_NAVIGATION_KEY_SUPER,
    BSP_INPUT_NAVIGATION_KEY_TAB,
    BSP_INPUT_NAVIGATION_KEY_BACKSPACE,


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

    // Volume keys
    BSP_INPUT_NAVIGATION_KEY_VOLUME_UP,
    BSP_INPUT_NAVIGATION_KEY_VOLUME_DOWN,
} bsp_input_navigation_key_t;

typedef enum _bsp_input_action_type {
    BSP_INPUT_ACTION_TYPE_NONE = 0,
    BSP_INPUT_ACTION_TYPE_SD_CARD,
    BSP_INPUT_ACTION_TYPE_AUDIO_JACK,
    BSP_INPUT_ACTION_TYPE_POWER_BUTTON,
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
    char const *utf8;
    uint32_t    modifiers;
} bsp_input_event_args_keyboard_t;

typedef struct _bsp_input_event_args_action {
    bsp_input_action_type_t type;
    bool                    state;
} bsp_input_event_args_action_t;

typedef struct _bsp_input_event {
    bsp_input_event_type_t type;
    union {
        bsp_input_event_args_navigation_t args_navigation;
        bsp_input_event_args_keyboard_t   args_keyboard;
        bsp_input_event_args_action_t     args_action;
    };
} bsp_input_event_t;

/// @brief Initialize the BSP input subsystem
/// @return ESP-IDF error code
esp_err_t bsp_input_initialize(void);

/// @brief Get the queue handle for the input event queue
/// @return ESP-IDF error code
esp_err_t bsp_input_get_queue(QueueHandle_t *out_queue);

/// @brief Get whether or not the device needs an on-screen keyboard
/// @return true if the device needs an on-screen keyboard and false if it does not
bool needs_on_screen_keyboard();
