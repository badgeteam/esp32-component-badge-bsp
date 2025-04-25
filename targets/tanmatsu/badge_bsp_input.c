// Board support package API: Tanmatsu implementation
// SPDX-FileCopyrightText: 2025 Nicolai Electronics
// SPDX-License-Identifier: MIT

#include <inttypes.h>
#include <stdint.h>
#include <string.h>
#include "bsp/input.h"
#include "bsp/tanmatsu.h"
#include "driver/gpio.h"
#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/queue.h"
#include "hal/gpio_types.h"
#include "tanmatsu_coprocessor.h"
#include "tanmatsu_hardware.h"

static char const* TAG = "BSP INPUT";

static QueueHandle_t event_queue              = NULL;
static TaskHandle_t  key_repeat_thread_handle = NULL;

static bool     key_repeat_wait      = false;
static bool     key_repeat_fast      = false;
static char     key_repeat_ascii     = '\0';
static char     key_repeat_utf8[5]   = {0};
static uint32_t key_repeat_modifiers = 0;

static bool prev_volume_down_state = false;

IRAM_ATTR static void volume_down_gpio_interrupt_handler(void* pvParameters) {
    bool state = !gpio_get_level(BSP_GPIO_BTN_VOLUME_DOWN);  // GPIO is active low
    if (state != prev_volume_down_state) {
        prev_volume_down_state           = state;
        bsp_input_event_t scancode_event = {
            .type = INPUT_EVENT_TYPE_SCANCODE,
            .args_scancode.scancode =
                BSP_INPUT_SCANCODE_ESCAPED_VOLUME_DOWN | (state ? 0 : BSP_INPUT_SCANCODE_RELEASE_MODIFIER),
        };
        xQueueSendFromISR(event_queue, &scancode_event, false);
        bsp_input_event_t navigation_event = {
            .type                      = INPUT_EVENT_TYPE_NAVIGATION,
            .args_navigation.key       = BSP_INPUT_NAVIGATION_KEY_VOLUME_DOWN,
            .args_navigation.modifiers = 0,
            .args_navigation.state     = state,
        };
        xQueueSendFromISR(event_queue, &navigation_event, false);
    }
}

static void send_navigation_event(bsp_input_navigation_key_t key, bool state, uint32_t modifiers) {
    bsp_input_event_t event = {
        .type                      = INPUT_EVENT_TYPE_NAVIGATION,
        .args_navigation.key       = key,
        .args_navigation.modifiers = modifiers,
        .args_navigation.state     = state,
    };
    xQueueSend(event_queue, &event, 0);
}

static void send_keyboard_event(char ascii, char const* utf8, uint32_t modifiers) {
    bsp_input_event_t event = {
        .type                    = INPUT_EVENT_TYPE_KEYBOARD,
        .args_keyboard.ascii     = ascii,
        .args_keyboard.utf8      = utf8,
        .args_keyboard.modifiers = modifiers,
    };
    xQueueSend(event_queue, &event, 0);
}

static void send_action_event(bsp_input_action_type_t action, bool state) {
    bsp_input_event_t event = {
        .type              = INPUT_EVENT_TYPE_ACTION,
        .args_action.type  = action,
        .args_action.state = state,
    };
    xQueueSend(event_queue, &event, 0);
}

static void send_scancode_event(bsp_input_scancode_t scancode, bool state) {
    bsp_input_event_t event = {
        .type                   = INPUT_EVENT_TYPE_SCANCODE,
        .args_scancode.scancode = scancode | (state ? 0 : BSP_INPUT_SCANCODE_RELEASE_MODIFIER),
    };
    xQueueSend(event_queue, &event, 0);
}

static void handle_keyboard_text_entry(bool curr_state, bool prev_state, char ascii, char ascii_shift, char const* utf8,
                                       char const* utf8_shift, char const* utf8_alt, char const* utf8_shift_alt,
                                       uint32_t modifiers) {
    if (curr_state && (!prev_state)) {
        // Key pressed
        char              value_ascii = (modifiers & BSP_INPUT_MODIFIER_SHIFT) ? ascii_shift : ascii;
        char const*       value_utf8  = (modifiers & BSP_INPUT_MODIFIER_ALT_R)
                                            ? ((modifiers & BSP_INPUT_MODIFIER_SHIFT) ? utf8_shift_alt : utf8_alt)
                                            : ((modifiers & BSP_INPUT_MODIFIER_SHIFT) ? utf8_shift : utf8);
        bsp_input_event_t event       = {
                  .type                    = INPUT_EVENT_TYPE_KEYBOARD,
                  .args_keyboard.ascii     = value_ascii,
                  .args_keyboard.utf8      = value_utf8,
                  .args_keyboard.modifiers = modifiers,
        };
        xQueueSend(event_queue, &event, 0);
        key_repeat_ascii = value_ascii;
        strncpy(key_repeat_utf8, value_utf8, sizeof(key_repeat_utf8) - 1);
        key_repeat_modifiers = modifiers;
        key_repeat_wait      = true;
        key_repeat_fast      = false;
    } else if (((!curr_state) && (prev_state)) || ((key_repeat_ascii != '\0') && (key_repeat_modifiers |= modifiers))) {
        key_repeat_ascii = '\0';
        memset(key_repeat_utf8, '\0', sizeof(key_repeat_utf8));
        key_repeat_modifiers = modifiers;
        key_repeat_wait      = false;
        key_repeat_fast      = false;
    }
}

void bsp_internal_coprocessor_keyboard_callback(tanmatsu_coprocessor_handle_t handle,
                                                tanmatsu_coprocessor_keys_t*  prev_keys,
                                                tanmatsu_coprocessor_keys_t*  keys) {
    static bool meta_key_modifier_used = false;

    // Modifier keys
    uint32_t modifiers = 0;
    if (keys->key_shift_l) {
        modifiers |= BSP_INPUT_MODIFIER_SHIFT_L;
    }
    if (keys->key_shift_r) {
        modifiers |= BSP_INPUT_MODIFIER_SHIFT_L;
    }
    if (keys->key_ctrl) {
        modifiers |= BSP_INPUT_MODIFIER_CTRL_L;
    }
    if (keys->key_alt_l) {
        modifiers |= BSP_INPUT_MODIFIER_ALT_L;
    }
    if (keys->key_alt_r) {
        modifiers |= BSP_INPUT_MODIFIER_ALT_R;
    }
    if (keys->key_meta) {
        modifiers |= BSP_INPUT_MODIFIER_SUPER_L;
    }
    if (keys->key_fn) {
        modifiers |= BSP_INPUT_MODIFIER_FUNCTION;
    }

    // Navigation keys
    for (uint8_t i = 0; i < TANMATSU_COPROCESSOR_KEYBOARD_NUM_REGS; i++) {
        uint8_t value = keys->raw[i];
        if (i == 2) {
            value &= ~(1 << 5);  // Ignore meta key
        }
        if (value) {
            meta_key_modifier_used = true;
            break;
        }
    }
    if (keys->key_meta && (!prev_keys->key_meta)) {
        meta_key_modifier_used = false;
    } else if ((!keys->key_meta) && prev_keys->key_meta) {
        if (!meta_key_modifier_used) {
            send_navigation_event(BSP_INPUT_NAVIGATION_KEY_SUPER, true, modifiers);
            send_navigation_event(BSP_INPUT_NAVIGATION_KEY_SUPER, false, modifiers);
        }
    }

    if (keys->key_meta != prev_keys->key_meta) {
        send_scancode_event(BSP_INPUT_SCANCODE_ESCAPED_LEFTMETA, keys->key_meta);
    }

    if (keys->key_esc != prev_keys->key_esc) {
        send_scancode_event(BSP_INPUT_SCANCODE_ESC, keys->key_esc);
        send_navigation_event(BSP_INPUT_NAVIGATION_KEY_ESC, keys->key_esc, modifiers);
    }
    if (keys->key_f1 != prev_keys->key_f1) {
        send_scancode_event(BSP_INPUT_SCANCODE_F1, keys->key_f1);
        send_navigation_event(BSP_INPUT_NAVIGATION_KEY_F1, keys->key_f1, modifiers);
    }
    if (keys->key_f2 != prev_keys->key_f2) {
        send_scancode_event(BSP_INPUT_SCANCODE_F2, keys->key_f2);
        send_navigation_event(BSP_INPUT_NAVIGATION_KEY_F2, keys->key_f2, modifiers);
    }
    if (keys->key_f3 != prev_keys->key_f3) {
        send_scancode_event(BSP_INPUT_SCANCODE_F3, keys->key_f3);
        send_navigation_event(BSP_INPUT_NAVIGATION_KEY_F3, keys->key_f3, modifiers);
    }
    if (keys->key_f4 != prev_keys->key_f4) {
        send_scancode_event(BSP_INPUT_SCANCODE_F4, keys->key_f4);
        send_navigation_event(BSP_INPUT_NAVIGATION_KEY_F4, keys->key_f4, modifiers);
    }
    if (keys->key_f5 != prev_keys->key_f5) {
        send_scancode_event(BSP_INPUT_SCANCODE_F5, keys->key_f5);
        send_navigation_event(BSP_INPUT_NAVIGATION_KEY_F5, keys->key_f5, modifiers);
    }
    if (keys->key_f6 != prev_keys->key_f6) {
        send_scancode_event(BSP_INPUT_SCANCODE_F6, keys->key_f6);
        send_navigation_event(BSP_INPUT_NAVIGATION_KEY_F6, keys->key_f6, modifiers);
    }
    if (keys->key_return != prev_keys->key_return) {
        send_scancode_event(BSP_INPUT_SCANCODE_ENTER, keys->key_return);
        send_navigation_event(BSP_INPUT_NAVIGATION_KEY_RETURN, keys->key_return, modifiers);
    }
    if (keys->key_up != prev_keys->key_up) {
        send_scancode_event(BSP_INPUT_SCANCODE_ESCAPED_GREY_UP, keys->key_up);
        send_navigation_event(BSP_INPUT_NAVIGATION_KEY_UP, keys->key_up, modifiers);
    }
    if (keys->key_left != prev_keys->key_left) {
        send_scancode_event(BSP_INPUT_SCANCODE_ESCAPED_GREY_LEFT, keys->key_left);
        send_navigation_event(BSP_INPUT_NAVIGATION_KEY_LEFT, keys->key_left, modifiers);
    }
    if (keys->key_down != prev_keys->key_down) {
        send_scancode_event(BSP_INPUT_SCANCODE_ESCAPED_GREY_DOWN, keys->key_down);
        send_navigation_event(BSP_INPUT_NAVIGATION_KEY_DOWN, keys->key_down, modifiers);
    }
    if (keys->key_right != prev_keys->key_right) {
        send_scancode_event(BSP_INPUT_SCANCODE_ESCAPED_GREY_RIGHT, keys->key_right);
        send_navigation_event(BSP_INPUT_NAVIGATION_KEY_RIGHT, keys->key_right, modifiers);
    }
    if (keys->key_volume_up != prev_keys->key_volume_up) {
        send_scancode_event(BSP_INPUT_SCANCODE_ESCAPED_VOLUME_UP, keys->key_volume_up);
        send_navigation_event(BSP_INPUT_NAVIGATION_KEY_VOLUME_UP, keys->key_volume_up, modifiers);
    }
    if (keys->key_tab != prev_keys->key_tab) {
        send_scancode_event(BSP_INPUT_SCANCODE_TAB, keys->key_tab);
        send_navigation_event(BSP_INPUT_NAVIGATION_KEY_TAB, keys->key_tab, modifiers);
    }
    if (keys->key_backspace != prev_keys->key_backspace) {
        send_scancode_event(BSP_INPUT_SCANCODE_BACKSPACE, keys->key_backspace);
        send_navigation_event(BSP_INPUT_NAVIGATION_KEY_BACKSPACE, keys->key_backspace, modifiers);
    }
    if (keys->key_space_l != prev_keys->key_space_l) {
        send_navigation_event(BSP_INPUT_NAVIGATION_KEY_SPACE_L, keys->key_space_l, modifiers);
    }
    if (keys->key_space_m != prev_keys->key_space_m) {
        send_navigation_event(BSP_INPUT_NAVIGATION_KEY_SPACE_M, keys->key_space_m, modifiers);
    }
    if (keys->key_space_r != prev_keys->key_space_r) {
        send_navigation_event(BSP_INPUT_NAVIGATION_KEY_SPACE_R, keys->key_space_r, modifiers);
    }

    // Text entry keys (scancode)
    if (keys->key_tilde != prev_keys->key_tilde) send_scancode_event(BSP_INPUT_SCANCODE_GRAVE, keys->key_tilde);
    if (keys->key_shift_l != prev_keys->key_shift_l)
        send_scancode_event(BSP_INPUT_SCANCODE_LEFTSHIFT, keys->key_shift_l);
    if (keys->key_1 != prev_keys->key_1) send_scancode_event(BSP_INPUT_SCANCODE_1, keys->key_1);
    if (keys->key_2 != prev_keys->key_2) send_scancode_event(BSP_INPUT_SCANCODE_2, keys->key_2);
    if (keys->key_3 != prev_keys->key_3) send_scancode_event(BSP_INPUT_SCANCODE_3, keys->key_3);
    if (keys->key_4 != prev_keys->key_4) send_scancode_event(BSP_INPUT_SCANCODE_4, keys->key_4);
    if (keys->key_5 != prev_keys->key_5) send_scancode_event(BSP_INPUT_SCANCODE_5, keys->key_5);
    if (keys->key_6 != prev_keys->key_6) send_scancode_event(BSP_INPUT_SCANCODE_6, keys->key_6);
    if (keys->key_7 != prev_keys->key_7) send_scancode_event(BSP_INPUT_SCANCODE_7, keys->key_7);
    if (keys->key_8 != prev_keys->key_8) send_scancode_event(BSP_INPUT_SCANCODE_8, keys->key_8);
    if (keys->key_9 != prev_keys->key_9) send_scancode_event(BSP_INPUT_SCANCODE_9, keys->key_9);
    if (keys->key_0 != prev_keys->key_0) send_scancode_event(BSP_INPUT_SCANCODE_0, keys->key_0);
    if (keys->key_minus != prev_keys->key_minus) send_scancode_event(BSP_INPUT_SCANCODE_MINUS, keys->key_minus);
    if (keys->key_equals != prev_keys->key_equals) send_scancode_event(BSP_INPUT_SCANCODE_EQUAL, keys->key_equals);
    if (keys->key_q != prev_keys->key_q) send_scancode_event(BSP_INPUT_SCANCODE_Q, keys->key_q);
    if (keys->key_w != prev_keys->key_w) send_scancode_event(BSP_INPUT_SCANCODE_W, keys->key_w);
    if (keys->key_e != prev_keys->key_e) send_scancode_event(BSP_INPUT_SCANCODE_E, keys->key_e);
    if (keys->key_r != prev_keys->key_r) send_scancode_event(BSP_INPUT_SCANCODE_R, keys->key_r);
    if (keys->key_t != prev_keys->key_t) send_scancode_event(BSP_INPUT_SCANCODE_T, keys->key_t);
    if (keys->key_y != prev_keys->key_y) send_scancode_event(BSP_INPUT_SCANCODE_Y, keys->key_y);
    if (keys->key_u != prev_keys->key_u) send_scancode_event(BSP_INPUT_SCANCODE_U, keys->key_u);
    if (keys->key_i != prev_keys->key_i) send_scancode_event(BSP_INPUT_SCANCODE_I, keys->key_i);
    if (keys->key_o != prev_keys->key_o) send_scancode_event(BSP_INPUT_SCANCODE_O, keys->key_o);
    if (keys->key_p != prev_keys->key_p) send_scancode_event(BSP_INPUT_SCANCODE_P, keys->key_p);
    if (keys->key_sqbracket_open != prev_keys->key_sqbracket_open)
        send_scancode_event(BSP_INPUT_SCANCODE_LEFTBRACE, keys->key_sqbracket_open);
    if (keys->key_sqbracket_close != prev_keys->key_sqbracket_close)
        send_scancode_event(BSP_INPUT_SCANCODE_RIGHTBRACE, keys->key_sqbracket_close);
    if (keys->key_a != prev_keys->key_a) send_scancode_event(BSP_INPUT_SCANCODE_A, keys->key_a);
    if (keys->key_s != prev_keys->key_s) send_scancode_event(BSP_INPUT_SCANCODE_S, keys->key_s);
    if (keys->key_d != prev_keys->key_d) send_scancode_event(BSP_INPUT_SCANCODE_D, keys->key_d);
    if (keys->key_f != prev_keys->key_f) send_scancode_event(BSP_INPUT_SCANCODE_F, keys->key_f);
    if (keys->key_g != prev_keys->key_g) send_scancode_event(BSP_INPUT_SCANCODE_G, keys->key_g);
    if (keys->key_h != prev_keys->key_h) send_scancode_event(BSP_INPUT_SCANCODE_H, keys->key_h);
    if (keys->key_j != prev_keys->key_j) send_scancode_event(BSP_INPUT_SCANCODE_J, keys->key_j);
    if (keys->key_k != prev_keys->key_k) send_scancode_event(BSP_INPUT_SCANCODE_K, keys->key_k);
    if (keys->key_l != prev_keys->key_l) send_scancode_event(BSP_INPUT_SCANCODE_L, keys->key_l);
    if (keys->key_semicolon != prev_keys->key_semicolon)
        send_scancode_event(BSP_INPUT_SCANCODE_SEMICOLON, keys->key_semicolon);
    if (keys->key_quote != prev_keys->key_quote) send_scancode_event(BSP_INPUT_SCANCODE_APOSTROPHE, keys->key_quote);
    if (keys->key_z != prev_keys->key_z) send_scancode_event(BSP_INPUT_SCANCODE_Z, keys->key_z);
    if (keys->key_x != prev_keys->key_x) send_scancode_event(BSP_INPUT_SCANCODE_X, keys->key_x);
    if (keys->key_c != prev_keys->key_c) send_scancode_event(BSP_INPUT_SCANCODE_C, keys->key_c);
    if (keys->key_v != prev_keys->key_v) send_scancode_event(BSP_INPUT_SCANCODE_V, keys->key_v);
    if (keys->key_b != prev_keys->key_b) send_scancode_event(BSP_INPUT_SCANCODE_B, keys->key_b);
    if (keys->key_n != prev_keys->key_n) send_scancode_event(BSP_INPUT_SCANCODE_N, keys->key_n);
    if (keys->key_m != prev_keys->key_m) send_scancode_event(BSP_INPUT_SCANCODE_M, keys->key_m);
    if (keys->key_comma != prev_keys->key_comma) send_scancode_event(BSP_INPUT_SCANCODE_COMMA, keys->key_comma);
    if (keys->key_dot != prev_keys->key_dot) send_scancode_event(BSP_INPUT_SCANCODE_DOT, keys->key_dot);
    if (keys->key_slash != prev_keys->key_slash) send_scancode_event(BSP_INPUT_SCANCODE_SLASH, keys->key_slash);
    if (keys->key_shift_r != prev_keys->key_shift_r)
        send_scancode_event(BSP_INPUT_SCANCODE_RIGHTSHIFT, keys->key_shift_r);
    if (keys->key_backslash != prev_keys->key_backslash)
        send_scancode_event(BSP_INPUT_SCANCODE_BACKSLASH, keys->key_backslash);
    if ((keys->key_space_l | keys->key_space_m | keys->key_space_r) !=
        (prev_keys->key_space_l | prev_keys->key_space_m | prev_keys->key_space_r))
        send_scancode_event(BSP_INPUT_SCANCODE_SPACE, keys->key_space_l | keys->key_space_m | keys->key_space_r);
    if (keys->key_fn != prev_keys->key_fn) {
        send_scancode_event(BSP_INPUT_SCANCODE_FN, keys->key_fn);
    }
    if (keys->key_ctrl != prev_keys->key_ctrl) {
        send_scancode_event(BSP_INPUT_SCANCODE_LEFTCTRL, keys->key_ctrl);
    }
    if (keys->key_alt_l != prev_keys->key_alt_l) {
        send_scancode_event(BSP_INPUT_SCANCODE_LEFTALT, keys->key_alt_l);
    }
    if (keys->key_alt_r != prev_keys->key_alt_r) {
        send_scancode_event(BSP_INPUT_SCANCODE_ESCAPED_RALT, keys->key_alt_r);
    }

    // Text entry keys (ASCII / UTF8)
    handle_keyboard_text_entry(keys->key_backspace, prev_keys->key_backspace, '\b', '\b', "\b", "\b", "\b", "\b",
                               modifiers);
    handle_keyboard_text_entry(keys->key_tilde, prev_keys->key_tilde, '`', '~', "`", "~", "`", "~", modifiers);
    handle_keyboard_text_entry(keys->key_1, prev_keys->key_1, '1', '!', "1", "!", "¡", "¹", modifiers);
    handle_keyboard_text_entry(keys->key_2, prev_keys->key_2, '2', '@', "2", "@", "²", "̋", modifiers);
    handle_keyboard_text_entry(keys->key_3, prev_keys->key_3, '3', '#', "3", "#", "³", "̄", modifiers);
    handle_keyboard_text_entry(keys->key_4, prev_keys->key_4, '4', '$', "4", "$", "¤", "£", modifiers);
    handle_keyboard_text_entry(keys->key_5, prev_keys->key_5, '5', '%', "5", "%", "€", "¸", modifiers);
    handle_keyboard_text_entry(keys->key_6, prev_keys->key_6, '6', '^', "6", "^", "¼", "̂", modifiers);
    handle_keyboard_text_entry(keys->key_7, prev_keys->key_7, '7', '&', "7", "&", "½", "̛", modifiers);
    handle_keyboard_text_entry(keys->key_8, prev_keys->key_8, '8', '*', "8", "*", "¾", "̨", modifiers);
    handle_keyboard_text_entry(keys->key_9, prev_keys->key_9, '9', '(', "9", "(", "‘", "̆", modifiers);
    handle_keyboard_text_entry(keys->key_0, prev_keys->key_0, '0', ')', "0", ")", "’", "̊", modifiers);
    handle_keyboard_text_entry(keys->key_minus, prev_keys->key_minus, '-', '_', "-", "_", "¥", "̣", modifiers);
    handle_keyboard_text_entry(keys->key_equals, prev_keys->key_equals, '=', '+', "=", "+", "̋", "̛", modifiers);
    handle_keyboard_text_entry(keys->key_tab, prev_keys->key_tab, '\t', '\t', "\t", "\t", "\t", "\t", modifiers);
    handle_keyboard_text_entry(keys->key_q, prev_keys->key_q, 'q', 'Q', "q", "Q", "ä", "Ä", modifiers);
    handle_keyboard_text_entry(keys->key_w, prev_keys->key_w, 'w', 'W', "w", "W", "å", "Å", modifiers);
    handle_keyboard_text_entry(keys->key_e, prev_keys->key_e, 'e', 'E', "e", "E", "é", "É", modifiers);
    handle_keyboard_text_entry(keys->key_r, prev_keys->key_r, 'r', 'R', "r", "R", "®", "™", modifiers);
    handle_keyboard_text_entry(keys->key_t, prev_keys->key_t, 't', 'T', "t", "T", "þ", "Þ", modifiers);
    handle_keyboard_text_entry(keys->key_y, prev_keys->key_y, 'y', 'Y', "y", "Y", "ü", "Ü", modifiers);
    handle_keyboard_text_entry(keys->key_u, prev_keys->key_u, 'u', 'U', "u", "U", "ú", "Ú", modifiers);
    handle_keyboard_text_entry(keys->key_i, prev_keys->key_i, 'i', 'I', "i", "I", "í", "Í", modifiers);
    handle_keyboard_text_entry(keys->key_o, prev_keys->key_o, 'o', 'O', "o", "O", "ó", "Ó", modifiers);
    handle_keyboard_text_entry(keys->key_p, prev_keys->key_p, 'p', 'P', "p", "P", "ö", "Ö", modifiers);
    handle_keyboard_text_entry(keys->key_sqbracket_open, prev_keys->key_sqbracket_open, '[', '{', "[", "{", "«", "“",
                               modifiers);
    handle_keyboard_text_entry(keys->key_sqbracket_close, prev_keys->key_sqbracket_close, ']', '}', "]", "}", "»", "”",
                               modifiers);
    handle_keyboard_text_entry(keys->key_a, prev_keys->key_a, 'a', 'A', "a", "A", "á", "Á", modifiers);
    handle_keyboard_text_entry(keys->key_s, prev_keys->key_s, 's', 'S', "s", "S", "ß", "§", modifiers);
    handle_keyboard_text_entry(keys->key_d, prev_keys->key_d, 'd', 'D', "d", "D", "ð", "Ð", modifiers);
    handle_keyboard_text_entry(keys->key_f, prev_keys->key_f, 'f', 'F', "f", "F", "ë", "Ë", modifiers);
    handle_keyboard_text_entry(keys->key_g, prev_keys->key_g, 'g', 'G', "g", "G", "g", "G", modifiers);
    handle_keyboard_text_entry(keys->key_h, prev_keys->key_h, 'h', 'H', "h", "H", "h", "H", modifiers);
    handle_keyboard_text_entry(keys->key_j, prev_keys->key_j, 'j', 'J', "j", "J", "ï", "Ï", modifiers);
    handle_keyboard_text_entry(keys->key_k, prev_keys->key_k, 'k', 'K', "k", "K", "œ", "Œ", modifiers);
    handle_keyboard_text_entry(keys->key_l, prev_keys->key_l, 'l', 'L', "l", "L", "ø", "L", modifiers);
    handle_keyboard_text_entry(keys->key_semicolon, prev_keys->key_semicolon, ';', ':', ";", ":", "̨", "̈", modifiers);
    handle_keyboard_text_entry(keys->key_quote, prev_keys->key_quote, '\'', '"', "'", "\"", "́", "̈", modifiers);
    handle_keyboard_text_entry(keys->key_z, prev_keys->key_z, 'z', 'Z', "z", "Z", "æ", "Æ", modifiers);
    handle_keyboard_text_entry(keys->key_x, prev_keys->key_x, 'x', 'X', "x", "X", "·", " ̵", modifiers);
    handle_keyboard_text_entry(keys->key_c, prev_keys->key_c, 'c', 'C', "c", "C", "©", "¢", modifiers);
    handle_keyboard_text_entry(keys->key_v, prev_keys->key_v, 'v', 'V', "v", "V", "v", "V", modifiers);
    handle_keyboard_text_entry(keys->key_b, prev_keys->key_b, 'b', 'B', "b", "B", "b", "B", modifiers);
    handle_keyboard_text_entry(keys->key_n, prev_keys->key_n, 'n', 'N', "n", "N", "ñ", "Ñ", modifiers);
    handle_keyboard_text_entry(keys->key_m, prev_keys->key_m, 'm', 'M', "m", "M", "µ", "±", modifiers);
    handle_keyboard_text_entry(keys->key_comma, prev_keys->key_comma, ',', '<', ",", "<", "̧", "̌", modifiers);
    handle_keyboard_text_entry(keys->key_dot, prev_keys->key_dot, '.', '>', ".", ">", "̇", "̌", modifiers);
    handle_keyboard_text_entry(keys->key_slash, prev_keys->key_slash, '/', '?', "/", "?", "¿", "̉", modifiers);
    handle_keyboard_text_entry(keys->key_backslash, prev_keys->key_backslash, '\\', '|', "\\", "|", "¬", "¦",
                               modifiers);
    handle_keyboard_text_entry(keys->key_space_l | keys->key_space_m | keys->key_space_r,
                               prev_keys->key_space_l | prev_keys->key_space_m | prev_keys->key_space_r, ' ', ' ', " ",
                               " ", " ", " ", modifiers);
}

void bsp_internal_coprocessor_input_callback(tanmatsu_coprocessor_handle_t  handle,
                                             tanmatsu_coprocessor_inputs_t* prev_inputs,
                                             tanmatsu_coprocessor_inputs_t* inputs) {
    if (inputs->sd_card_detect != prev_inputs->sd_card_detect) {
        send_action_event(BSP_INPUT_ACTION_TYPE_SD_CARD, inputs->sd_card_detect);
    }

    if (inputs->headphone_detect != prev_inputs->headphone_detect) {
        send_action_event(BSP_INPUT_ACTION_TYPE_AUDIO_JACK, inputs->headphone_detect);
    }

    if (inputs->power_button != prev_inputs->power_button) {
        send_action_event(BSP_INPUT_ACTION_TYPE_POWER_BUTTON, inputs->power_button);
    }
}

void bsp_internal_coprocessor_faults_callback(tanmatsu_coprocessor_handle_t       handle,
                                              tanmatsu_coprocessor_pmic_faults_t* prev_faults,
                                              tanmatsu_coprocessor_pmic_faults_t* faults) {
    ESP_LOGE(TAG, "Faults changed: %s%s%s%s%s%s%s%s%s\r\n", faults->watchdog ? "WATCHDOG " : "",
             faults->boost ? "BOOST " : "", faults->chrg_input ? "CHRG_INPUT " : "",
             faults->chrg_thermal ? "CHRG_THERMAL " : "", faults->chrg_safety ? "CHRG_SAFETY " : "",
             faults->batt_ovp ? "BATT_OVP " : "", faults->ntc_cold ? "NTC_COLD " : "",
             faults->ntc_hot ? "NTC_HOT " : "", faults->ntc_boost ? "NTC_BOOST " : "");
}

/*static void key_repeat_thread(void* ignored) {
    (void)ignored;
    while (1) {
        if (key_repeat_wait) {
            key_repeat_fast = false;
            key_repeat_wait = false;
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }
        if (key_repeat_ascii != '\0') {
            // ESP_LOGI(TAG, "Repeat text: %c / %s", key_repeat_ascii, key_repeat_utf8);
            bsp_input_event_t event = {
                .type                    = INPUT_EVENT_TYPE_KEYBOARD,
                .args_keyboard.ascii     = key_repeat_ascii,
                .args_keyboard.utf8      = key_repeat_utf8,
                .args_keyboard.modifiers = key_repeat_modifiers,
            };
            xQueueSend(event_queue, &event, 0);
        }
        vTaskDelay(pdMS_TO_TICKS(key_repeat_fast ? 100 : 200));
        key_repeat_fast = true;
    }
}*/

esp_err_t bsp_input_initialize(void) {
    if (event_queue == NULL) {
        event_queue = xQueueCreate(32, sizeof(bsp_input_event_t));
        ESP_RETURN_ON_FALSE(event_queue, ESP_ERR_NO_MEM, TAG, "Failed to create input event queue");
    }

    /*if (key_repeat_thread_handle == NULL) {
        xTaskCreate(key_repeat_thread, "Key repeat thread", 4096, NULL, tskIDLE_PRIORITY, &key_repeat_thread_handle);
        ESP_RETURN_ON_FALSE(key_repeat_thread_handle, ESP_ERR_NO_MEM, TAG, "Failed to create key repeat task");
    }*/

    gpio_config_t int_pin_cfg = {
        .pin_bit_mask = BIT64(BSP_GPIO_BTN_VOLUME_DOWN),
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = false,
        .pull_down_en = false,
        .intr_type    = GPIO_INTR_ANYEDGE,
    };
    ESP_RETURN_ON_ERROR(gpio_config(&int_pin_cfg), TAG, "Failed to configure volume down button GPIO");
    ESP_RETURN_ON_ERROR(gpio_isr_handler_add(BSP_GPIO_BTN_VOLUME_DOWN, volume_down_gpio_interrupt_handler, NULL), TAG,
                        "Failed to add interrupt handler for volume down button GPIO");

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
    return false;
}
esp_err_t bsp_input_get_backlight_brightness(uint8_t* out_percentage) {
    ESP_RETURN_ON_FALSE(out_percentage, ESP_ERR_INVALID_ARG, TAG, "Percentage output argument is NULL");
    tanmatsu_coprocessor_handle_t handle = NULL;
    ESP_RETURN_ON_ERROR(bsp_tanmatsu_coprocessor_get_handle(&handle), TAG, "Failed to get coprocessor handle");
    uint8_t raw_value;
    ESP_RETURN_ON_ERROR(tanmatsu_coprocessor_get_keyboard_backlight(handle, &raw_value), TAG,
                        "Failed to get keyboard backlight brightness");
    *out_percentage = (raw_value * 100) / 255;
    return ESP_OK;
}

esp_err_t bsp_input_set_backlight_brightness(uint8_t percentage) {
    tanmatsu_coprocessor_handle_t handle = NULL;
    ESP_RETURN_ON_ERROR(bsp_tanmatsu_coprocessor_get_handle(&handle), TAG, "Failed to get coprocessor handle");
    ESP_RETURN_ON_ERROR(tanmatsu_coprocessor_set_keyboard_backlight(handle, (percentage * 255) / 100), TAG,
                        "Failed to configure keyboard backlight brightness");
    return ESP_OK;
}

esp_err_t bsp_input_read_navigation_key(bsp_input_navigation_key_t key, bool* out_state) {
    tanmatsu_coprocessor_handle_t handle = NULL;
    ESP_RETURN_ON_ERROR(bsp_tanmatsu_coprocessor_get_handle(&handle), TAG, "Failed to get coprocessor handle");
    tanmatsu_coprocessor_keys_t keys;
    if (key != BSP_GPIO_BTN_VOLUME_DOWN) {
        ESP_RETURN_ON_ERROR(tanmatsu_coprocessor_get_keyboard_keys(handle, &keys), TAG, "Failed to read keyboard keys");
    }
    switch (key) {
        case BSP_INPUT_NAVIGATION_KEY_ESC:
            *out_state = keys.key_esc;
            break;
        case BSP_INPUT_NAVIGATION_KEY_LEFT:
            *out_state = keys.key_left;
            break;
        case BSP_INPUT_NAVIGATION_KEY_RIGHT:
            *out_state = keys.key_right;
            break;
        case BSP_INPUT_NAVIGATION_KEY_UP:
            *out_state = keys.key_up;
            break;
        case BSP_INPUT_NAVIGATION_KEY_DOWN:
            *out_state = keys.key_down;
            break;
        case BSP_INPUT_NAVIGATION_KEY_RETURN:
            *out_state = keys.key_return;
            break;
        case BSP_INPUT_NAVIGATION_KEY_SUPER:
            *out_state = keys.key_meta;
            break;
        case BSP_INPUT_NAVIGATION_KEY_TAB:
            *out_state = keys.key_tab;
            break;
        case BSP_INPUT_NAVIGATION_KEY_BACKSPACE:
            *out_state = keys.key_backspace;
            break;
        case BSP_INPUT_NAVIGATION_KEY_SPACE_L:
            *out_state = keys.key_space_l;
            break;
        case BSP_INPUT_NAVIGATION_KEY_SPACE_M:
            *out_state = keys.key_space_m;
            break;
        case BSP_INPUT_NAVIGATION_KEY_SPACE_R:
            *out_state = keys.key_space_r;
            break;
        case BSP_INPUT_NAVIGATION_KEY_F1:
            *out_state = keys.key_f1;
            break;
        case BSP_INPUT_NAVIGATION_KEY_F2:
            *out_state = keys.key_f2;
            break;
        case BSP_INPUT_NAVIGATION_KEY_F3:
            *out_state = keys.key_f3;
            break;
        case BSP_INPUT_NAVIGATION_KEY_F4:
            *out_state = keys.key_f4;
            break;
        case BSP_INPUT_NAVIGATION_KEY_F5:
            *out_state = keys.key_f5;
            break;
        case BSP_INPUT_NAVIGATION_KEY_F6:
            *out_state = keys.key_f6;
            break;
        case BSP_INPUT_NAVIGATION_KEY_VOLUME_UP:
            *out_state = keys.key_volume_up;
            break;
        case BSP_INPUT_NAVIGATION_KEY_VOLUME_DOWN:
            *out_state = !gpio_get_level(BSP_GPIO_BTN_VOLUME_DOWN);
            break;
        default:
            *out_state = false;
            break;
    }
    return ESP_OK;
}

esp_err_t bsp_input_read_action(bsp_input_action_type_t action, bool* out_state) {
    tanmatsu_coprocessor_handle_t handle = NULL;
    ESP_RETURN_ON_ERROR(bsp_tanmatsu_coprocessor_get_handle(&handle), TAG, "Failed to get coprocessor handle");
    tanmatsu_coprocessor_inputs_t inputs;
    ESP_RETURN_ON_ERROR(tanmatsu_coprocessor_get_inputs(handle, &inputs), TAG, "Failed to read inputs");
    switch (action) {
        case BSP_INPUT_ACTION_TYPE_POWER_BUTTON:
            *out_state = inputs.power_button;
            break;
        case BSP_INPUT_ACTION_TYPE_AUDIO_JACK:
            *out_state = inputs.headphone_detect;
            break;
        case BSP_INPUT_ACTION_TYPE_SD_CARD:
            *out_state = inputs.sd_card_detect;
            break;
        default:
            *out_state = false;
            break;
    }
    return ESP_OK;
}
