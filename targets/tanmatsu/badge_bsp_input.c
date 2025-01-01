// Board support package API: Tanmatsu implementation
// SPDX-FileCopyrightText: 2024 Nicolai Electronics
// SPDX-License-Identifier: MIT

#include "bsp/input.h"
#include "bsp/tanmatsu.h"
#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/queue.h"
#include "tanmatsu_coprocessor.h"

#include <inttypes.h>
#include <stdint.h>

#include <string.h>

static char const *TAG = "BSP INPUT";

static QueueHandle_t event_queue              = NULL;
static TaskHandle_t  key_repeat_thread_handle = NULL;

static bool     key_repeat_wait      = false;
static bool     key_repeat_fast      = false;
static char     key_repeat_ascii     = '\0';
static char     key_repeat_utf8[5]   = {0};
static uint32_t key_repeat_modifiers = 0;

static void handle_keyboard_text_entry(
    bool curr_state, bool prev_state, char ascii, char ascii_shift, char const *utf8, char const *utf8_shift, char const *utf8_alt, char const *utf8_shift_alt, uint32_t modifiers
) {
    if (curr_state && (!prev_state)) {
        // Key pressed
        char        value_ascii = (modifiers & BSP_INPUT_MODIFIER_SHIFT) ? ascii_shift : ascii;
        char const *value_utf8
            = (modifiers & BSP_INPUT_MODIFIER_ALT_R) ? ((modifiers & BSP_INPUT_MODIFIER_SHIFT) ? utf8_shift_alt : utf8_alt) : ((modifiers & BSP_INPUT_MODIFIER_SHIFT) ? utf8_shift : utf8);
        // ESP_LOGI(TAG, "Text: %c / %s (modifiers: %08" PRIX32 ")", value_ascii, value_utf8, modifiers);
        bsp_input_event_t event = {
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

void bsp_internal_coprocessor_keyboard_callback(tanmatsu_coprocessor_handle_t handle, tanmatsu_coprocessor_keys_t *prev_keys, tanmatsu_coprocessor_keys_t *keys) {
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
            value &= ~(1 << 5); // Ignore meta key
        }
        if (value) {
            meta_key_modifier_used = true;
            break;
        }
    }
    if (keys->key_meta && (!prev_keys->key_meta)) {
        // ESP_LOGI(TAG, "Meta key pressed");
        meta_key_modifier_used = false;
    } else if ((!keys->key_meta) && prev_keys->key_meta) {
        if (!meta_key_modifier_used) {
            // ESP_LOGI(TAG, "Meta key triggered");
            bsp_input_event_t event = {
                .type                      = INPUT_EVENT_TYPE_NAVIGATION,
                .args_navigation.key       = BSP_INPUT_NAVIGATION_KEY_SUPER,
                .args_navigation.modifiers = modifiers,
                .args_navigation.state     = true,
            };
            xQueueSend(event_queue, &event, 0);
            event.args_navigation.state = false;
            xQueueSend(event_queue, &event, 0);
        }
    }
    if (keys->key_esc != prev_keys->key_esc) {
        // ESP_LOGI(TAG, "Esc %s", keys->key_esc ? "pressed" : "released");
        bsp_input_event_t event = {
            .type                      = INPUT_EVENT_TYPE_NAVIGATION,
            .args_navigation.key       = BSP_INPUT_NAVIGATION_KEY_ESC,
            .args_navigation.modifiers = modifiers,
            .args_navigation.state     = keys->key_esc,
        };
        xQueueSend(event_queue, &event, 0);
    }
    if (keys->key_f1 != prev_keys->key_f1) {
        // ESP_LOGI(TAG, "F1 %s", keys->key_f1 ? "pressed" : "released");
        bsp_input_event_t event = {
            .type                      = INPUT_EVENT_TYPE_NAVIGATION,
            .args_navigation.key       = BSP_INPUT_NAVIGATION_KEY_F1,
            .args_navigation.modifiers = modifiers,
            .args_navigation.state     = keys->key_f1,
        };
        xQueueSend(event_queue, &event, 0);
    }
    if (keys->key_f2 != prev_keys->key_f2) {
        // ESP_LOGI(TAG, "F2 %s", keys->key_f2 ? "pressed" : "released");
        bsp_input_event_t event = {
            .type                      = INPUT_EVENT_TYPE_NAVIGATION,
            .args_navigation.key       = BSP_INPUT_NAVIGATION_KEY_F2,
            .args_navigation.modifiers = modifiers,
            .args_navigation.state     = keys->key_f2,
        };
        xQueueSend(event_queue, &event, 0);
    }
    if (keys->key_f3 != prev_keys->key_f3) {
        // ESP_LOGI(TAG, "F3 %s", keys->key_f3 ? "pressed" : "released");
        bsp_input_event_t event = {
            .type                      = INPUT_EVENT_TYPE_NAVIGATION,
            .args_navigation.key       = BSP_INPUT_NAVIGATION_KEY_F3,
            .args_navigation.modifiers = modifiers,
            .args_navigation.state     = keys->key_f3,
        };
        xQueueSend(event_queue, &event, 0);
    }
    if (keys->key_f4 != prev_keys->key_f4) {
        // ESP_LOGI(TAG, "F4 %s", keys->key_f4 ? "pressed" : "released");
        bsp_input_event_t event = {
            .type                      = INPUT_EVENT_TYPE_NAVIGATION,
            .args_navigation.key       = BSP_INPUT_NAVIGATION_KEY_F4,
            .args_navigation.modifiers = modifiers,
            .args_navigation.state     = keys->key_f4,
        };
        xQueueSend(event_queue, &event, 0);
    }
    if (keys->key_f5 != prev_keys->key_f5) {
        // ESP_LOGI(TAG, "F5 %s", keys->key_f5 ? "pressed" : "released");
        bsp_input_event_t event = {
            .type                      = INPUT_EVENT_TYPE_NAVIGATION,
            .args_navigation.key       = BSP_INPUT_NAVIGATION_KEY_F5,
            .args_navigation.modifiers = modifiers,
            .args_navigation.state     = keys->key_f5,
        };
        xQueueSend(event_queue, &event, 0);
    }
    if (keys->key_f6 != prev_keys->key_f6) {
        // ESP_LOGI(TAG, "F6 %s", keys->key_f6 ? "pressed" : "released");
        bsp_input_event_t event = {
            .type                      = INPUT_EVENT_TYPE_NAVIGATION,
            .args_navigation.key       = BSP_INPUT_NAVIGATION_KEY_F6,
            .args_navigation.modifiers = modifiers,
            .args_navigation.state     = keys->key_f6,
        };
        xQueueSend(event_queue, &event, 0);
    }
    if (keys->key_return != prev_keys->key_return) {
        // ESP_LOGI(TAG, "Return %s", keys->key_return ? "pressed" : "released");
        bsp_input_event_t event = {
            .type                      = INPUT_EVENT_TYPE_NAVIGATION,
            .args_navigation.key       = BSP_INPUT_NAVIGATION_KEY_RETURN,
            .args_navigation.modifiers = modifiers,
            .args_navigation.state     = keys->key_return,
        };
        xQueueSend(event_queue, &event, 0);
    }
    if (keys->key_up != prev_keys->key_up) {
        // ESP_LOGI(TAG, "Up %s", keys->key_up ? "pressed" : "released");
        bsp_input_event_t event = {
            .type                      = INPUT_EVENT_TYPE_NAVIGATION,
            .args_navigation.key       = BSP_INPUT_NAVIGATION_KEY_UP,
            .args_navigation.modifiers = modifiers,
            .args_navigation.state     = keys->key_up,
        };
        xQueueSend(event_queue, &event, 0);
    }
    if (keys->key_left != prev_keys->key_left) {
        // ESP_LOGI(TAG, "Left %s", keys->key_left ? "pressed" : "released");
        bsp_input_event_t event = {
            .type                      = INPUT_EVENT_TYPE_NAVIGATION,
            .args_navigation.key       = BSP_INPUT_NAVIGATION_KEY_LEFT,
            .args_navigation.modifiers = modifiers,
            .args_navigation.state     = keys->key_left,
        };
        xQueueSend(event_queue, &event, 0);
    }
    if (keys->key_down != prev_keys->key_down) {
        // ESP_LOGI(TAG, "Down %s", keys->key_down ? "pressed" : "released");
        bsp_input_event_t event = {
            .type                      = INPUT_EVENT_TYPE_NAVIGATION,
            .args_navigation.key       = BSP_INPUT_NAVIGATION_KEY_DOWN,
            .args_navigation.modifiers = modifiers,
            .args_navigation.state     = keys->key_down,
        };
        xQueueSend(event_queue, &event, 0);
    }
    if (keys->key_right != prev_keys->key_right) {
        // ESP_LOGI(TAG, "Right %s", keys->key_right ? "pressed" : "released");
        bsp_input_event_t event = {
            .type                      = INPUT_EVENT_TYPE_NAVIGATION,
            .args_navigation.key       = BSP_INPUT_NAVIGATION_KEY_RIGHT,
            .args_navigation.modifiers = modifiers,
            .args_navigation.state     = keys->key_right,
        };
        xQueueSend(event_queue, &event, 0);
    }
    if (keys->key_volume_up != prev_keys->key_volume_up) {
        // ESP_LOGI(TAG, "Volume up %s", keys->key_volume_up ? "pressed" : "released");
        bsp_input_event_t event = {
            .type                      = INPUT_EVENT_TYPE_NAVIGATION,
            .args_navigation.key       = BSP_INPUT_NAVIGATION_KEY_VOLUME_UP,
            .args_navigation.modifiers = modifiers,
            .args_navigation.state     = keys->key_volume_up,
        };
        xQueueSend(event_queue, &event, 0);
    }
    if (keys->key_tab != prev_keys->key_tab) {
        // ESP_LOGI(TAG, "Tab %s", keys->key_tab ? "pressed" : "released");
        bsp_input_event_t event = {
            .type                      = INPUT_EVENT_TYPE_NAVIGATION,
            .args_navigation.key       = BSP_INPUT_NAVIGATION_KEY_TAB,
            .args_navigation.modifiers = modifiers,
            .args_navigation.state     = keys->key_tab,
        };
        xQueueSend(event_queue, &event, 0);
    }
    if (keys->key_backspace != prev_keys->key_backspace) {
        // ESP_LOGI(TAG, "Backspace %s", keys->key_backspace ? "pressed" : "released");
        bsp_input_event_t event = {
            .type                      = INPUT_EVENT_TYPE_NAVIGATION,
            .args_navigation.key       = BSP_INPUT_NAVIGATION_KEY_BACKSPACE,
            .args_navigation.modifiers = modifiers,
            .args_navigation.state     = keys->key_backspace,
        };
        xQueueSend(event_queue, &event, 0);
    }

    // Text entry keys
    handle_keyboard_text_entry(keys->key_backspace, prev_keys->key_backspace, '\b', '\b', "\b", "\b", "\b", "\b", modifiers);
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
    handle_keyboard_text_entry(keys->key_sqbracket_open, prev_keys->key_sqbracket_open, '[', '{', "[", "{", "«", "“", modifiers);
    handle_keyboard_text_entry(keys->key_sqbracket_close, prev_keys->key_sqbracket_close, ']', '}', "]", "}", "»", "”", modifiers);
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
    handle_keyboard_text_entry(keys->key_backslash, prev_keys->key_backslash, '\\', '|', "\\", "|", "¬", "¦", modifiers);
    handle_keyboard_text_entry(
        keys->key_space_l | keys->key_space_m | keys->key_space_r,
        prev_keys->key_space_l | prev_keys->key_space_m | prev_keys->key_space_r,
        ' ',
        ' ',
        " ",
        " ",
        " ",
        " ",
        modifiers
    );
}

void bsp_internal_coprocessor_input_callback(tanmatsu_coprocessor_handle_t handle, tanmatsu_coprocessor_inputs_t *prev_inputs, tanmatsu_coprocessor_inputs_t *inputs) {
    if (inputs->sd_card_detect != prev_inputs->sd_card_detect) {
        ESP_LOGW(TAG, "SD card %s", inputs->sd_card_detect ? "inserted" : "removed");
    }

    if (inputs->headphone_detect != prev_inputs->headphone_detect) {
        ESP_LOGW(TAG, "Audio jack %s", inputs->headphone_detect ? "inserted" : "removed");
    }

    if (inputs->power_button != prev_inputs->power_button) {
        ESP_LOGW(TAG, "Power button %s", inputs->power_button ? "pressed" : "released");
    }
}

void bsp_internal_coprocessor_faults_callback(tanmatsu_coprocessor_handle_t handle, tanmatsu_coprocessor_pmic_faults_t *prev_faults, tanmatsu_coprocessor_pmic_faults_t *faults) {
    ESP_LOGE(
        TAG,
        "Faults changed: %s%s%s%s%s%s%s%s%s\r\n",
        faults->watchdog ? "WATCHDOG " : "",
        faults->boost ? "BOOST " : "",
        faults->chrg_input ? "CHRG_INPUT " : "",
        faults->chrg_thermal ? "CHRG_THERMAL " : "",
        faults->chrg_safety ? "CHRG_SAFETY " : "",
        faults->batt_ovp ? "BATT_OVP " : "",
        faults->ntc_cold ? "NTC_COLD " : "",
        faults->ntc_hot ? "NTC_HOT " : "",
        faults->ntc_boost ? "NTC_BOOST " : ""
    );
}

static void key_repeat_thread(void *ignored) {
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
}

esp_err_t bsp_input_initialize(void) {
    if (event_queue == NULL) {
        event_queue = xQueueCreate(32, sizeof(bsp_input_event_t));
        ESP_RETURN_ON_FALSE(event_queue, ESP_ERR_NO_MEM, TAG, "Failed to create input event queue");
    }
    if (key_repeat_thread_handle == NULL) {
        xTaskCreate(key_repeat_thread, "Key repeat thread", 4096, NULL, tskIDLE_PRIORITY, &key_repeat_thread_handle);
        ESP_RETURN_ON_FALSE(key_repeat_thread_handle, ESP_ERR_NO_MEM, TAG, "Failed to create key repeat task");
    }
    return ESP_OK;
}

esp_err_t bsp_input_get_queue(QueueHandle_t *out_queue) {
    if (out_queue == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    if (event_queue == NULL) {
        return ESP_FAIL;
    }
    *out_queue = event_queue;
    return ESP_OK;
}

bool needs_on_screen_keyboard() {
    return false;
}
esp_err_t bsp_input_get_backlight_brightness(uint8_t *out_percentage) {
    ESP_RETURN_ON_FALSE(out_percentage, ESP_ERR_INVALID_ARG, TAG, "Percentage output argument is NULL");
    tanmatsu_coprocessor_handle_t handle = NULL;
    ESP_RETURN_ON_ERROR(bsp_tanmatsu_coprocessor_get_handle(&handle), TAG, "Failed to get coprocessor handle");
    uint8_t raw_value;
    ESP_RETURN_ON_ERROR(tanmatsu_coprocessor_get_keyboard_backlight(handle, &raw_value), TAG, "Failed to get keyboard backlight brightness");
    *out_percentage = (raw_value * 100) / 255;
    return ESP_OK;
}

esp_err_t bsp_input_set_backlight_brightness(uint8_t percentage) {
    tanmatsu_coprocessor_handle_t handle = NULL;
    ESP_RETURN_ON_ERROR(bsp_tanmatsu_coprocessor_get_handle(&handle), TAG, "Failed to get coprocessor handle");
    ESP_RETURN_ON_ERROR(tanmatsu_coprocessor_set_keyboard_backlight(handle, (percentage * 255) / 100), TAG, "Failed to configure keyboard backlight brightness");
    return ESP_OK;
}
