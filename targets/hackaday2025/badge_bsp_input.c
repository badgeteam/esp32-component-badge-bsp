// SPDX-FileCopyrightText: 2026 Nicolai Electronics
// SPDX-License-Identifier: MIT

#include <stdint.h>
#include <stdio.h>
#include "badge_bsp_input_hooks.h"
#include "bsp/i2c.h"
#include "bsp/input.h"
#include "driver/gpio.h"
#include "esp_check.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "hackaday2025_hardware.h"
#include "tca8418.h"

static char const* TAG = "BSP INPUT";

static QueueHandle_t    event_queue       = NULL;
static bool             prev_button_state = false;
static tca8418_handle_t tca8418_handle    = {0};
static uint32_t         active_modifiers  = 0;
static bool             super_used        = false;

typedef enum {
    HACKADAY2025_KEY_F1       = 2,
    HACKADAY2025_KEY_NUM_PLUS = 3,
    HACKADAY2025_KEY_NUM_9    = 4,
    HACKADAY2025_KEY_NUM_8    = 5,
    HACKADAY2025_KEY_NUM_7    = 6,
    HACKADAY2025_KEY_F2       = 7,
    HACKADAY2025_KEY_F3       = 8,
    HACKADAY2025_KEY_F4       = 9,
    HACKADAY2025_KEY_F5       = 10,

    HACKADAY2025_KEY_ESC = 11,
    HACKADAY2025_KEY_Q   = 12,
    HACKADAY2025_KEY_W   = 13,
    HACKADAY2025_KEY_E   = 14,
    HACKADAY2025_KEY_R   = 15,
    HACKADAY2025_KEY_T   = 16,
    HACKADAY2025_KEY_Y   = 17,
    HACKADAY2025_KEY_U   = 18,
    HACKADAY2025_KEY_I   = 19,
    HACKADAY2025_KEY_O   = 20,

    HACKADAY2025_KEY_TAB = 21,
    HACKADAY2025_KEY_A   = 22,
    HACKADAY2025_KEY_S   = 23,
    HACKADAY2025_KEY_D   = 24,
    HACKADAY2025_KEY_F   = 25,
    HACKADAY2025_KEY_G   = 26,
    HACKADAY2025_KEY_H   = 27,
    HACKADAY2025_KEY_J   = 28,
    HACKADAY2025_KEY_K   = 29,
    HACKADAY2025_KEY_L   = 30,

    HACKADAY2025_KEY_LEFT_SHIFT = 31,
    HACKADAY2025_KEY_Z          = 32,
    HACKADAY2025_KEY_X          = 33,
    HACKADAY2025_KEY_C          = 34,
    HACKADAY2025_KEY_V          = 35,
    HACKADAY2025_KEY_B          = 36,
    HACKADAY2025_KEY_N          = 37,
    HACKADAY2025_KEY_M          = 38,
    HACKADAY2025_KEY_COMMA      = 39,
    HACKADAY2025_KEY_DOT        = 40,

    HACKADAY2025_KEY_CTRL      = 41,
    HACKADAY2025_KEY_SUPER     = 42,
    HACKADAY2025_KEY_LEFT_ALT  = 43,
    HACKADAY2025_KEY_BACKSLASH = 44,
    HACKADAY2025_KEY_SPACE     = 45,
    HACKADAY2025_KEY_RIGHT     = 47,
    HACKADAY2025_KEY_DOWN      = 48,
    HACKADAY2025_KEY_LEFT      = 49,
    HACKADAY2025_KEY_RIGHT_ALT = 50,

    HACKADAY2025_KEY_NUM_MINUS     = 53,
    HACKADAY2025_KEY_NUM_6         = 54,
    HACKADAY2025_KEY_NUM_5         = 55,
    HACKADAY2025_KEY_NUM_4         = 56,
    HACKADAY2025_KEY_RIGHT_BRACKET = 57,
    HACKADAY2025_KEY_LEFT_BRACKET  = 58,
    HACKADAY2025_KEY_P             = 59,

    HACKADAY2025_KEY_NUM_ASTERISK = 63,
    HACKADAY2025_KEY_NUM_3        = 64,
    HACKADAY2025_KEY_NUM_2        = 65,
    HACKADAY2025_KEY_NUM_1        = 66,
    HACKADAY2025_KEY_ENTER        = 67,
    HACKADAY2025_KEY_APOSTROPHE   = 68,
    HACKADAY2025_KEY_SEMICOLON    = 69,

    HACKADAY2025_KEY_NUM_SLASH   = 73,
    HACKADAY2025_KEY_NUM_EQUALS  = 74,
    HACKADAY2025_KEY_NUM_DOT     = 75,
    HACKADAY2025_KEY_NUM_0       = 76,
    HACKADAY2025_KEY_RIGHT_SHIFT = 77,
    HACKADAY2025_KEY_UP          = 78,
    HACKADAY2025_KEY_BACKSPACE   = 79,
} hackaday2025_keys_t;

static void send_navigation_event(bsp_input_navigation_key_t key, bool state, uint32_t modifiers) {
    bsp_input_event_t event = {
        .type                      = INPUT_EVENT_TYPE_NAVIGATION,
        .args_navigation.key       = key,
        .args_navigation.modifiers = modifiers,
        .args_navigation.state     = state,
    };
    // Offer to hooks first; if consumed, don't queue
    if (!bsp_input_hooks_process(&event)) {
        xQueueSend(event_queue, &event, 0);
    }
}

static void send_keyboard_event(char ascii, char const* utf8, uint32_t modifiers) {
    bsp_input_event_t event = {
        .type                    = INPUT_EVENT_TYPE_KEYBOARD,
        .args_keyboard.ascii     = ascii,
        .args_keyboard.modifiers = modifiers,
    };
    strlcpy(event.args_keyboard.utf8, utf8, sizeof(event.args_keyboard.utf8));
    // Offer to hooks first; if consumed, don't queue
    if (!bsp_input_hooks_process(&event)) {
        xQueueSend(event_queue, &event, 0);
    }
}

static void send_action_event(bsp_input_action_type_t action, bool state) {
    bsp_input_event_t event = {
        .type              = INPUT_EVENT_TYPE_ACTION,
        .args_action.type  = action,
        .args_action.state = state,
    };
    // Offer to hooks first; if consumed, don't queue
    if (!bsp_input_hooks_process(&event)) {
        xQueueSend(event_queue, &event, 0);
    }
}

static void send_scancode_event(bsp_input_scancode_t scancode, bool state) {
    bsp_input_event_t event = {
        .type                   = INPUT_EVENT_TYPE_SCANCODE,
        .args_scancode.scancode = scancode | (state ? 0 : BSP_INPUT_SCANCODE_RELEASE_MODIFIER),
    };
    // Offer to hooks first; if consumed, don't queue
    if (!bsp_input_hooks_process(&event)) {
        xQueueSend(event_queue, &event, 0);
    }
}

static void handle_keyboard_text_entry(char ascii, char ascii_shift, char const* utf8, char const* utf8_shift,
                                       char const* utf8_alt, char const* utf8_shift_alt, uint32_t modifiers) {
    char              value_ascii = (modifiers & BSP_INPUT_MODIFIER_SHIFT) ? ascii_shift : ascii;
    char const*       value_utf8  = (modifiers & BSP_INPUT_MODIFIER_ALT_R)
                                        ? ((modifiers & BSP_INPUT_MODIFIER_SHIFT) ? utf8_shift_alt : utf8_alt)
                                        : ((modifiers & BSP_INPUT_MODIFIER_SHIFT) ? utf8_shift : utf8);
    bsp_input_event_t event       = {
        .type                    = INPUT_EVENT_TYPE_KEYBOARD,
        .args_keyboard.ascii     = value_ascii,
        .args_keyboard.modifiers = modifiers,
    };
    if (value_utf8) {
        strlcpy(event.args_keyboard.utf8, value_utf8, sizeof(event.args_keyboard.utf8));
    } else {
        event.args_keyboard.utf8[0] = value_ascii;
        event.args_keyboard.utf8[1] = 0;
    }
    xQueueSend(event_queue, &event, 0);
}

IRAM_ATTR static void button_interrupt_handler(void* pvParameters) {
    bool state = !gpio_get_level(BSP_GPIO_BTN);  // GPIO is active low
    if (state != prev_button_state) {
        prev_button_state                = state;
        bsp_input_event_t scancode_event = {
            .type                   = INPUT_EVENT_TYPE_SCANCODE,
            .args_scancode.scancode = BSP_INPUT_SCANCODE_ENTER | (state ? 0 : BSP_INPUT_SCANCODE_RELEASE_MODIFIER),
        };
        xQueueSendFromISR(event_queue, &scancode_event, false);
        bsp_input_event_t navigation_event = {
            .type                      = INPUT_EVENT_TYPE_NAVIGATION,
            .args_navigation.key       = BSP_INPUT_NAVIGATION_KEY_RETURN,
            .args_navigation.modifiers = 0,
            .args_navigation.state     = state,
        };
        xQueueSendFromISR(event_queue, &navigation_event, false);
    }
}

static void tca8418_cad_callback(tca8418_handle_t* handle) {
    ESP_LOGI(TAG, "Ctrl-Alt-Del key sequence detected\r\n");
}

static void tca8418_overflow_callback(tca8418_handle_t* handle) {
    ESP_LOGW(TAG, "key event FIFO overflow\r\n");
}

static void tca8418_lock_callback(tca8418_handle_t* handle) {
    ESP_LOGI(TAG, "keypad lock state changed\r\n");
}

static void tca8418_gpi_callback(tca8418_handle_t* handle) {
    ESP_LOGI(TAG, "GPI state changed\r\n");
}

static void tca8418_key_callback(tca8418_handle_t* handle) {
    // Drain the key event FIFO (up to 10 entries deep)
    for (int i = 0; i < 10; i++) {
        bool    pressed = false;
        uint8_t code    = 0;
        if (tca8418_get_key_event_a(handle, &pressed, &code) != ESP_OK || code == 0) {
            break;
        }

        hackaday2025_keys_t key = (hackaday2025_keys_t)code;

        if (key != HACKADAY2025_KEY_SUPER) {
            super_used = true;
        }

        switch (key) {
            case HACKADAY2025_KEY_F1:
                send_navigation_event(BSP_INPUT_NAVIGATION_KEY_F1, pressed, active_modifiers);
                break;
            case HACKADAY2025_KEY_NUM_PLUS:
                send_scancode_event(BSP_INPUT_SCANCODE_KPPLUS, pressed);
                handle_keyboard_text_entry('+', '+', "+", "+", "+", "+", active_modifiers);
                break;
            case HACKADAY2025_KEY_NUM_9:
                send_scancode_event(BSP_INPUT_SCANCODE_9, pressed);
                handle_keyboard_text_entry('9', '(', "9", "(", "‘", "̆", active_modifiers);
                break;
            case HACKADAY2025_KEY_NUM_8:
                send_scancode_event(BSP_INPUT_SCANCODE_8, pressed);
                handle_keyboard_text_entry('8', '*', "8", "*", "¾", "̨", active_modifiers);
                break;
            case HACKADAY2025_KEY_NUM_7:
                send_scancode_event(BSP_INPUT_SCANCODE_7, pressed);
                handle_keyboard_text_entry('7', '&', "7", "&", "½", "̛", active_modifiers);
                break;
            case HACKADAY2025_KEY_F2:
                send_navigation_event(BSP_INPUT_NAVIGATION_KEY_F2, pressed, active_modifiers);
                break;
            case HACKADAY2025_KEY_F3:
                send_navigation_event(BSP_INPUT_NAVIGATION_KEY_F3, pressed, active_modifiers);
                break;
            case HACKADAY2025_KEY_F4:
                send_navigation_event(BSP_INPUT_NAVIGATION_KEY_F4, pressed, active_modifiers);
                break;
            case HACKADAY2025_KEY_F5:
                send_navigation_event(BSP_INPUT_NAVIGATION_KEY_F5, pressed, active_modifiers);
                break;
            case HACKADAY2025_KEY_ESC:
                send_navigation_event(BSP_INPUT_NAVIGATION_KEY_ESC, pressed, active_modifiers);
                break;
            case HACKADAY2025_KEY_Q:
                send_scancode_event(BSP_INPUT_SCANCODE_Q, pressed);
                handle_keyboard_text_entry('q', 'Q', "q", "Q", "ä", "Ä", active_modifiers);
                break;
            case HACKADAY2025_KEY_W:
                send_scancode_event(BSP_INPUT_SCANCODE_W, pressed);
                handle_keyboard_text_entry('w', 'W', "w", "W", "å", "Å", active_modifiers);
                break;
            case HACKADAY2025_KEY_E:
                send_scancode_event(BSP_INPUT_SCANCODE_E, pressed);
                handle_keyboard_text_entry('e', 'E', "e", "E", "é", "É", active_modifiers);
                break;
            case HACKADAY2025_KEY_R:
                send_scancode_event(BSP_INPUT_SCANCODE_R, pressed);
                handle_keyboard_text_entry('r', 'R', "r", "R", "®", "™", active_modifiers);
                break;
            case HACKADAY2025_KEY_T:
                send_scancode_event(BSP_INPUT_SCANCODE_T, pressed);
                handle_keyboard_text_entry('t', 'T', "t", "T", "þ", "Þ", active_modifiers);
                break;
            case HACKADAY2025_KEY_Y:
                send_scancode_event(BSP_INPUT_SCANCODE_Y, pressed);
                handle_keyboard_text_entry('y', 'Y', "y", "Y", "ü", "Ü", active_modifiers);
                break;
            case HACKADAY2025_KEY_U:
                send_scancode_event(BSP_INPUT_SCANCODE_U, pressed);
                handle_keyboard_text_entry('u', 'U', "u", "U", "ú", "Ú", active_modifiers);
                break;
            case HACKADAY2025_KEY_I:
                send_scancode_event(BSP_INPUT_SCANCODE_I, pressed);
                handle_keyboard_text_entry('i', 'I', "i", "I", "í", "Í", active_modifiers);
                break;
            case HACKADAY2025_KEY_O:
                send_scancode_event(BSP_INPUT_SCANCODE_O, pressed);
                handle_keyboard_text_entry('o', 'O', "o", "O", "ó", "Ó", active_modifiers);
                break;
            case HACKADAY2025_KEY_TAB:
                send_navigation_event(BSP_INPUT_NAVIGATION_KEY_TAB, pressed, active_modifiers);
                break;
            case HACKADAY2025_KEY_A:
                send_scancode_event(BSP_INPUT_SCANCODE_A, pressed);
                handle_keyboard_text_entry('a', 'A', "a", "A", "á", "Á", active_modifiers);
                break;
            case HACKADAY2025_KEY_S:
                send_scancode_event(BSP_INPUT_SCANCODE_S, pressed);
                handle_keyboard_text_entry('s', 'S', "s", "S", "ß", "§", active_modifiers);
                break;
            case HACKADAY2025_KEY_D:
                send_scancode_event(BSP_INPUT_SCANCODE_D, pressed);
                handle_keyboard_text_entry('d', 'D', "d", "D", "ð", "Ð", active_modifiers);
                break;
            case HACKADAY2025_KEY_F:
                send_scancode_event(BSP_INPUT_SCANCODE_F, pressed);
                handle_keyboard_text_entry('f', 'F', "f", "F", "ë", "Ë", active_modifiers);
                break;
            case HACKADAY2025_KEY_G:
                send_scancode_event(BSP_INPUT_SCANCODE_G, pressed);
                handle_keyboard_text_entry('g', 'G', "g", "G", "g", "G", active_modifiers);
                break;
            case HACKADAY2025_KEY_H:
                send_scancode_event(BSP_INPUT_SCANCODE_H, pressed);
                handle_keyboard_text_entry('h', 'H', "h", "H", "h", "H", active_modifiers);
                break;
            case HACKADAY2025_KEY_J:
                send_scancode_event(BSP_INPUT_SCANCODE_J, pressed);
                handle_keyboard_text_entry('j', 'J', "j", "J", "ï", "Ï", active_modifiers);
                break;
            case HACKADAY2025_KEY_K:
                send_scancode_event(BSP_INPUT_SCANCODE_K, pressed);
                handle_keyboard_text_entry('k', 'K', "k", "K", "œ", "Œ", active_modifiers);
                break;
            case HACKADAY2025_KEY_L:
                send_scancode_event(BSP_INPUT_SCANCODE_L, pressed);
                handle_keyboard_text_entry('l', 'L', "l", "L", "ø", "L", active_modifiers);
                break;
            case HACKADAY2025_KEY_LEFT_SHIFT:
                if (pressed) {
                    active_modifiers |= BSP_INPUT_MODIFIER_SHIFT_L;
                } else {
                    active_modifiers &= ~(BSP_INPUT_MODIFIER_SHIFT_L);
                }
                break;
            case HACKADAY2025_KEY_Z:
                send_scancode_event(BSP_INPUT_SCANCODE_Z, pressed);
                handle_keyboard_text_entry('z', 'Z', "z", "Z", "æ", "Æ", active_modifiers);
                break;
            case HACKADAY2025_KEY_X:
                send_scancode_event(BSP_INPUT_SCANCODE_X, pressed);
                handle_keyboard_text_entry('x', 'X', "x", "X", "·", " ̵", active_modifiers);
                break;
            case HACKADAY2025_KEY_C:
                send_scancode_event(BSP_INPUT_SCANCODE_C, pressed);
                handle_keyboard_text_entry('c', 'C', "c", "C", "©", "¢", active_modifiers);
                break;
            case HACKADAY2025_KEY_V:
                send_scancode_event(BSP_INPUT_SCANCODE_V, pressed);
                handle_keyboard_text_entry('v', 'V', "v", "V", "v", "V", active_modifiers);
                break;
            case HACKADAY2025_KEY_B:
                send_scancode_event(BSP_INPUT_SCANCODE_B, pressed);
                handle_keyboard_text_entry('b', 'B', "b", "B", "b", "B", active_modifiers);
                break;
            case HACKADAY2025_KEY_N:
                send_scancode_event(BSP_INPUT_SCANCODE_N, pressed);
                handle_keyboard_text_entry('n', 'N', "n", "N", "ñ", "Ñ", active_modifiers);
                break;
            case HACKADAY2025_KEY_M:
                send_scancode_event(BSP_INPUT_SCANCODE_M, pressed);
                handle_keyboard_text_entry('m', 'M', "m", "M", "µ", "±", active_modifiers);
                break;
            case HACKADAY2025_KEY_COMMA:
                send_scancode_event(BSP_INPUT_SCANCODE_COMMA, pressed);
                handle_keyboard_text_entry(',', '<', ",", "<", "̧", "̌", active_modifiers);
                break;
            case HACKADAY2025_KEY_DOT:
                send_scancode_event(BSP_INPUT_SCANCODE_DOT, pressed);
                handle_keyboard_text_entry('.', '>', ".", ">", "̇", "̌", active_modifiers);
                break;
            case HACKADAY2025_KEY_CTRL:
                if (pressed) {
                    active_modifiers |= BSP_INPUT_MODIFIER_CTRL_L;
                } else {
                    active_modifiers &= ~(BSP_INPUT_MODIFIER_CTRL_L);
                }
                break;
            case HACKADAY2025_KEY_SUPER:
                if (pressed) {
                    active_modifiers |= BSP_INPUT_MODIFIER_SUPER_L;
                    super_used        = false;
                } else {
                    active_modifiers &= ~(BSP_INPUT_MODIFIER_SUPER_L);
                    if (!super_used) {
                        send_navigation_event(BSP_INPUT_NAVIGATION_KEY_SUPER, true, active_modifiers);
                        send_navigation_event(BSP_INPUT_NAVIGATION_KEY_SUPER, false, active_modifiers);
                    }
                }
                break;
            case HACKADAY2025_KEY_LEFT_ALT:
                if (pressed) {
                    active_modifiers |= BSP_INPUT_MODIFIER_ALT_L;
                } else {
                    active_modifiers &= ~(BSP_INPUT_MODIFIER_ALT_L);
                }
                break;
            case HACKADAY2025_KEY_BACKSLASH:
                send_scancode_event(BSP_INPUT_SCANCODE_BACKSLASH, pressed);
                break;
            case HACKADAY2025_KEY_SPACE:
                send_navigation_event(BSP_INPUT_NAVIGATION_KEY_SPACE_M, pressed, active_modifiers);
                break;
            case HACKADAY2025_KEY_RIGHT:
                send_navigation_event(BSP_INPUT_NAVIGATION_KEY_RIGHT, pressed, active_modifiers);
                break;
            case HACKADAY2025_KEY_DOWN:
                send_navigation_event(BSP_INPUT_NAVIGATION_KEY_DOWN, pressed, active_modifiers);
                break;
            case HACKADAY2025_KEY_LEFT:
                send_navigation_event(BSP_INPUT_NAVIGATION_KEY_LEFT, pressed, active_modifiers);
                break;
            case HACKADAY2025_KEY_RIGHT_ALT:
                if (pressed) {
                    active_modifiers |= BSP_INPUT_MODIFIER_ALT_R;
                } else {
                    active_modifiers &= ~(BSP_INPUT_MODIFIER_ALT_R);
                }
                break;
            case HACKADAY2025_KEY_NUM_MINUS:
                send_scancode_event(BSP_INPUT_SCANCODE_KPMINUS, pressed);
                break;
            case HACKADAY2025_KEY_NUM_6:
                send_scancode_event(BSP_INPUT_SCANCODE_6, pressed);
                handle_keyboard_text_entry('6', '^', "6", "^", "¼", "̂", active_modifiers);
                break;
            case HACKADAY2025_KEY_NUM_5:
                send_scancode_event(BSP_INPUT_SCANCODE_5, pressed);
                handle_keyboard_text_entry('5', '%', "5", "%", "€", "¸", active_modifiers);
                break;
            case HACKADAY2025_KEY_NUM_4:
                send_scancode_event(BSP_INPUT_SCANCODE_4, pressed);
                handle_keyboard_text_entry('4', '$', "4", "$", "¤", "£", active_modifiers);
                break;
            case HACKADAY2025_KEY_RIGHT_BRACKET:
                send_scancode_event(BSP_INPUT_SCANCODE_RIGHTBRACE, pressed);
                handle_keyboard_text_entry(']', '}', "]", "}", "»", "”", active_modifiers);
                break;
            case HACKADAY2025_KEY_LEFT_BRACKET:
                send_scancode_event(BSP_INPUT_SCANCODE_LEFTBRACE, pressed);
                handle_keyboard_text_entry('[', '{', "[", "{", "«", "“", active_modifiers);
                break;
            case HACKADAY2025_KEY_P:
                send_scancode_event(BSP_INPUT_SCANCODE_P, pressed);
                handle_keyboard_text_entry('p', 'P', "p", "P", "ö", "Ö", active_modifiers);
                break;
            case HACKADAY2025_KEY_NUM_ASTERISK:
                send_scancode_event(BSP_INPUT_SCANCODE_KPASTERISK, pressed);
                handle_keyboard_text_entry('*', '*', "*", "*", "*", "*", active_modifiers);
                break;
            case HACKADAY2025_KEY_NUM_3:
                send_scancode_event(BSP_INPUT_SCANCODE_3, pressed);
                handle_keyboard_text_entry('3', '#', "3", "#", "³", "̄", active_modifiers);
                break;
            case HACKADAY2025_KEY_NUM_2:
                send_scancode_event(BSP_INPUT_SCANCODE_2, pressed);
                handle_keyboard_text_entry('2', '@', "2", "@", "²", "̋", active_modifiers);
                break;
            case HACKADAY2025_KEY_NUM_1:
                send_scancode_event(BSP_INPUT_SCANCODE_1, pressed);
                handle_keyboard_text_entry('1', '!', "1", "!", "¡", "¹", active_modifiers);
                break;
            case HACKADAY2025_KEY_ENTER:
                send_navigation_event(BSP_INPUT_NAVIGATION_KEY_RETURN, pressed, active_modifiers);
                break;
            case HACKADAY2025_KEY_APOSTROPHE:
                send_scancode_event(BSP_INPUT_SCANCODE_APOSTROPHE, pressed);
                handle_keyboard_text_entry('\'', '"', "'", "\"", "́", "̈", active_modifiers);
                break;
            case HACKADAY2025_KEY_SEMICOLON:
                send_scancode_event(BSP_INPUT_SCANCODE_SEMICOLON, pressed);
                handle_keyboard_text_entry(';', ':', ";", ":", "̨", "̈", active_modifiers);
                break;
            case HACKADAY2025_KEY_NUM_SLASH:
                send_scancode_event(BSP_INPUT_SCANCODE_ESCAPED_GREY_KPSLASH, pressed);
                break;
            case HACKADAY2025_KEY_NUM_EQUALS:
                send_scancode_event(BSP_INPUT_SCANCODE_EQUAL, pressed);
                handle_keyboard_text_entry('=', '+', "=", "+", "̋", "̛", active_modifiers);
                break;
            case HACKADAY2025_KEY_NUM_DOT:
                send_scancode_event(BSP_INPUT_SCANCODE_KPDOT, pressed);
                handle_keyboard_text_entry('.', '.', ".", ".", ".", ".", active_modifiers);
                break;
            case HACKADAY2025_KEY_NUM_0:
                send_scancode_event(BSP_INPUT_SCANCODE_KP0, pressed);
                handle_keyboard_text_entry('0', ')', "0", ")", "’", "̊", active_modifiers);
                break;
            case HACKADAY2025_KEY_RIGHT_SHIFT:
                if (pressed) {
                    active_modifiers |= BSP_INPUT_MODIFIER_SHIFT_R;
                } else {
                    active_modifiers &= ~(BSP_INPUT_MODIFIER_SHIFT_R);
                }
                break;
            case HACKADAY2025_KEY_UP:
                send_navigation_event(BSP_INPUT_NAVIGATION_KEY_UP, pressed, active_modifiers);
                break;
            case HACKADAY2025_KEY_BACKSPACE:
                send_navigation_event(BSP_INPUT_NAVIGATION_KEY_BACKSPACE, pressed, active_modifiers);
                break;
            default:
                ESP_LOGW(TAG, "Unmapped key pressed: %u", code);
        }
    }
}

esp_err_t bsp_input_initialize(void) {
    if (event_queue == NULL) {
        event_queue = xQueueCreate(32, sizeof(bsp_input_event_t));
        ESP_RETURN_ON_FALSE(event_queue, ESP_ERR_NO_MEM, TAG, "Failed to create input event queue");
    }

    gpio_config_t int_pin_cfg = {
        .pin_bit_mask = BIT64(BSP_GPIO_BTN),
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = true,
        .pull_down_en = false,
        .intr_type    = GPIO_INTR_ANYEDGE,
    };
    ESP_RETURN_ON_ERROR(gpio_config(&int_pin_cfg), TAG, "Failed to configure button GPIO");
    ESP_RETURN_ON_ERROR(gpio_isr_handler_add(BSP_GPIO_BTN, button_interrupt_handler, NULL), TAG,
                        "Failed to add interrupt handler for button GPIO");

    i2c_master_bus_handle_t i2c_handle;
    bsp_i2c_primary_bus_get_handle(&i2c_handle);

    esp_err_t res = tca8418_initialize(&tca8418_handle, i2c_handle, 48, 13);
    if (res != ESP_OK) {
        return res;
    }

    // Use all row and column pins as keypad (key scan) pins
    ESP_RETURN_ON_ERROR(tca8418_set_kp_gpio1(&tca8418_handle, true, true, true, true, true, true, true, true), TAG,
                        "Failed to configure keypad rows");
    ESP_RETURN_ON_ERROR(tca8418_set_kp_gpio2(&tca8418_handle, true, true, true, true, true, true, true, true), TAG,
                        "Failed to configure keypad columns 0-7");
    ESP_RETURN_ON_ERROR(tca8418_set_kp_gpio3(&tca8418_handle, true, true), TAG,
                        "Failed to configure keypad columns 8-9");

    ESP_RETURN_ON_ERROR(tca8418_set_cad_callback(&tca8418_handle, tca8418_cad_callback), TAG,
                        "Failed to set CAD callback");
    ESP_RETURN_ON_ERROR(tca8418_set_lock_callback(&tca8418_handle, tca8418_lock_callback), TAG,
                        "Failed to set lock callback");
    ESP_RETURN_ON_ERROR(tca8418_set_gpi_callback(&tca8418_handle, tca8418_gpi_callback), TAG,
                        "Failed to set GPI callback");
    ESP_RETURN_ON_ERROR(tca8418_set_overflow_callback(&tca8418_handle, tca8418_overflow_callback), TAG,
                        "Failed to set overflow callback");
    ESP_RETURN_ON_ERROR(tca8418_set_key_callback(&tca8418_handle, tca8418_key_callback), TAG,
                        "Failed to set key callback");

    // Enable all interrupt sources (overflow, keypad lock, GPI, key events)
    ESP_RETURN_ON_ERROR(tca8418_set_cfg(&tca8418_handle, false, false, false, false, true, true, true, true), TAG,
                        "Failed to configure TCA8418 interrupts");

    return ESP_OK;
}

esp_err_t bsp_input_get_queue(QueueHandle_t* out_queue) {
    if (out_queue == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    if (event_queue == NULL) {
        return ESP_FAIL;
    }
    *out_queue = event_queue;
    return ESP_OK;
}

bool bsp_input_needs_on_screen_keyboard(void) {
    return true;
}

esp_err_t bsp_input_get_backlight_brightness(uint8_t* out_percentage) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t bsp_input_set_backlight_brightness(uint8_t percentage) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t bsp_input_read_navigation_key(bsp_input_navigation_key_t key, bool* out_state) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t bsp_input_read_scancode(bsp_input_scancode_t key, bool* out_state) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t bsp_input_read_action(bsp_input_action_type_t action, bool* out_state) {
    return ESP_ERR_NOT_SUPPORTED;
}
