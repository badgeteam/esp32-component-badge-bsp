// Board support package API: WHY2025 implementation
// SPDX-FileCopyrightText: 2024 Nicolai Electronics
// SPDX-License-Identifier: MIT

#include "bsp/input.h"
#include "bsp/why2025.h"
#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/projdefs.h"
#include "tanmatsu_coprocessor.h"

#include <stdint.h>

#include <string.h>

static char const *TAG = "BSP INPUT";

static QueueHandle_t event_queue              = NULL;
static TaskHandle_t  key_repeat_thread_handle = NULL;

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
            value &= (1 << 5); // Ignore meta key
        }
        if (value) {
            meta_key_modifier_used = true;
        }
    }
    if (keys->key_meta && (!prev_keys->key_meta)) {
        ESP_LOGI(TAG, "Meta key pressed");
        meta_key_modifier_used = false;
    } else if ((!keys->key_meta) && prev_keys->key_meta) {
        if (!meta_key_modifier_used) {
            ESP_LOGI(TAG, "Meta key triggered");
            bsp_input_event_t event = {
                .type                      = INPUT_EVENT_TYPE_NAVIGATION,
                .args_navigation.key       = BSP_INPUT_NAVIGATION_KEY_SUPER,
                .args_navigation.modifiers = modifiers,
                .args_navigation.state     = true,
            };
            xQueueSend(event_queue, &event, 0);
        } else {
            ESP_LOGI(TAG, "Meta key released");
        }
    }
    if (keys->key_esc != prev_keys->key_esc) {
        ESP_LOGI(TAG, "Esc %s", keys->key_esc ? "pressed" : "released");
        bsp_input_event_t event = {
            .type                      = INPUT_EVENT_TYPE_NAVIGATION,
            .args_navigation.key       = BSP_INPUT_NAVIGATION_KEY_ESC,
            .args_navigation.modifiers = modifiers,
            .args_navigation.state     = keys->key_esc,
        };
        xQueueSend(event_queue, &event, 0);
    }
    if (keys->key_f1 != prev_keys->key_f1) {
        ESP_LOGI(TAG, "F1 %s", keys->key_f1 ? "pressed" : "released");
        bsp_input_event_t event = {
            .type                      = INPUT_EVENT_TYPE_NAVIGATION,
            .args_navigation.key       = BSP_INPUT_NAVIGATION_KEY_F1,
            .args_navigation.modifiers = modifiers,
            .args_navigation.state     = keys->key_f1,
        };
        xQueueSend(event_queue, &event, 0);
    }
    if (keys->key_f2 != prev_keys->key_f2) {
        ESP_LOGI(TAG, "F2 %s", keys->key_f2 ? "pressed" : "released");
        bsp_input_event_t event = {
            .type                      = INPUT_EVENT_TYPE_NAVIGATION,
            .args_navigation.key       = BSP_INPUT_NAVIGATION_KEY_F2,
            .args_navigation.modifiers = modifiers,
            .args_navigation.state     = keys->key_f2,
        };
        xQueueSend(event_queue, &event, 0);
    }
    if (keys->key_f3 != prev_keys->key_f3) {
        ESP_LOGI(TAG, "F3 %s", keys->key_f3 ? "pressed" : "released");
        bsp_input_event_t event = {
            .type                      = INPUT_EVENT_TYPE_NAVIGATION,
            .args_navigation.key       = BSP_INPUT_NAVIGATION_KEY_F3,
            .args_navigation.modifiers = modifiers,
            .args_navigation.state     = keys->key_f3,
        };
        xQueueSend(event_queue, &event, 0);
    }
    if (keys->key_f4 != prev_keys->key_f4) {
        ESP_LOGI(TAG, "F4 %s", keys->key_f4 ? "pressed" : "released");
        bsp_input_event_t event = {
            .type                      = INPUT_EVENT_TYPE_NAVIGATION,
            .args_navigation.key       = BSP_INPUT_NAVIGATION_KEY_F4,
            .args_navigation.modifiers = modifiers,
            .args_navigation.state     = keys->key_f4,
        };
        xQueueSend(event_queue, &event, 0);
    }
    if (keys->key_f5 != prev_keys->key_f5) {
        ESP_LOGI(TAG, "F5 %s", keys->key_f5 ? "pressed" : "released");
        bsp_input_event_t event = {
            .type                      = INPUT_EVENT_TYPE_NAVIGATION,
            .args_navigation.key       = BSP_INPUT_NAVIGATION_KEY_F5,
            .args_navigation.modifiers = modifiers,
            .args_navigation.state     = keys->key_f5,
        };
        xQueueSend(event_queue, &event, 0);
    }
    if (keys->key_f6 != prev_keys->key_f6) {
        ESP_LOGI(TAG, "F6 %s", keys->key_f6 ? "pressed" : "released");
        bsp_input_event_t event = {
            .type                      = INPUT_EVENT_TYPE_NAVIGATION,
            .args_navigation.key       = BSP_INPUT_NAVIGATION_KEY_F6,
            .args_navigation.modifiers = modifiers,
            .args_navigation.state     = keys->key_f6,
        };
        xQueueSend(event_queue, &event, 0);
    }
    if (keys->key_return != prev_keys->key_return) {
        ESP_LOGI(TAG, "Return %s", keys->key_return ? "pressed" : "released");
        bsp_input_event_t event = {
            .type                      = INPUT_EVENT_TYPE_NAVIGATION,
            .args_navigation.key       = BSP_INPUT_NAVIGATION_KEY_RETURN,
            .args_navigation.modifiers = modifiers,
            .args_navigation.state     = keys->key_return,
        };
        xQueueSend(event_queue, &event, 0);
    }
    if (keys->key_up != prev_keys->key_up) {
        ESP_LOGI(TAG, "Up %s", keys->key_up ? "pressed" : "released");
        bsp_input_event_t event = {
            .type                      = INPUT_EVENT_TYPE_NAVIGATION,
            .args_navigation.key       = BSP_INPUT_NAVIGATION_KEY_UP,
            .args_navigation.modifiers = modifiers,
            .args_navigation.state     = keys->key_up,
        };
        xQueueSend(event_queue, &event, 0);
    }
    if (keys->key_left != prev_keys->key_left) {
        ESP_LOGI(TAG, "Left %s", keys->key_left ? "pressed" : "released");
        bsp_input_event_t event = {
            .type                      = INPUT_EVENT_TYPE_NAVIGATION,
            .args_navigation.key       = BSP_INPUT_NAVIGATION_KEY_LEFT,
            .args_navigation.modifiers = modifiers,
            .args_navigation.state     = keys->key_left,
        };
        xQueueSend(event_queue, &event, 0);
    }
    if (keys->key_down != prev_keys->key_down) {
        ESP_LOGI(TAG, "Down %s", keys->key_down ? "pressed" : "released");
        bsp_input_event_t event = {
            .type                      = INPUT_EVENT_TYPE_NAVIGATION,
            .args_navigation.key       = BSP_INPUT_NAVIGATION_KEY_DOWN,
            .args_navigation.modifiers = modifiers,
            .args_navigation.state     = keys->key_down,
        };
        xQueueSend(event_queue, &event, 0);
    }
    if (keys->key_right != prev_keys->key_right) {
        ESP_LOGI(TAG, "Right %s", keys->key_right ? "pressed" : "released");
        bsp_input_event_t event = {
            .type                      = INPUT_EVENT_TYPE_NAVIGATION,
            .args_navigation.key       = BSP_INPUT_NAVIGATION_KEY_RIGHT,
            .args_navigation.modifiers = modifiers,
            .args_navigation.state     = keys->key_right,
        };
        xQueueSend(event_queue, &event, 0);
    }
    if (keys->key_volume_up != prev_keys->key_volume_up) {
        ESP_LOGI(TAG, "Volume up %s", keys->key_right ? "pressed" : "released");
        bsp_input_event_t event = {
            .type                      = INPUT_EVENT_TYPE_NAVIGATION,
            .args_navigation.key       = BSP_INPUT_NAVIGATION_KEY_VOLUME_UP,
            .args_navigation.modifiers = modifiers,
            .args_navigation.state     = keys->key_volume_up,
        };
        xQueueSend(event_queue, &event, 0);
    }

    // Text entry keys
    if (keys->key_tilde && (!prev_keys->key_tilde)) {
        char  ascii = (modifiers & BSP_INPUT_MODIFIER_SHIFT) ? '~' : '`';
        char *utf8  = (modifiers & BSP_INPUT_MODIFIER_SHIFT) ? "~" : "`";
        ESP_LOGI(TAG, "Text: %c", ascii);
        bsp_input_event_t event = {
            .type                      = INPUT_EVENT_TYPE_KEYBOARD,
            .args_keyboard.ascii       = ascii,
            .args_keyboard.utf8        = utf8,
            .args_keyboard.utf8_length = strlen(utf8),
            .args_keyboard.modifiers   = modifiers,
        };
        xQueueSend(event_queue, &event, 0);
    }
    if (keys->key_1 && (!prev_keys->key_1)) {
        char  ascii = (modifiers & BSP_INPUT_MODIFIER_SHIFT) ? '!' : '1';
        char *utf8  = (modifiers & BSP_INPUT_MODIFIER_ALT_R) ? ((modifiers & BSP_INPUT_MODIFIER_SHIFT) ? "¹" : "¡") : ((modifiers & BSP_INPUT_MODIFIER_SHIFT) ? "!" : "1");
        ESP_LOGI(TAG, "Text: %c / %s", ascii, utf8);
        bsp_input_event_t event = {
            .type                      = INPUT_EVENT_TYPE_KEYBOARD,
            .args_keyboard.ascii       = ascii,
            .args_keyboard.utf8        = utf8,
            .args_keyboard.utf8_length = strlen(utf8),
            .args_keyboard.modifiers   = modifiers,
        };
        xQueueSend(event_queue, &event, 0);
    }
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
    printf(
        "Faults changed: %s %s %s %s %s %s %s %s %s\r\n",
        faults->watchdog ? "WATCHDOG" : "",
        faults->boost ? "BOOST" : "",
        faults->chrg_input ? "CHRG_INPUT" : "",
        faults->chrg_thermal ? "CHRG_THERMAL" : "",
        faults->chrg_safety ? "CHRG_SAFETY" : "",
        faults->batt_ovp ? "BATT_OVP" : "",
        faults->ntc_cold ? "NTC_COLD" : "",
        faults->ntc_hot ? "NTC_HOT" : "",
        faults->ntc_boost ? "NTC_BOOST" : ""
    );
}

static void key_repeat_thread(void *ignored) {
    (void)ignored;
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(100));
        // xQueueSend(outgoing_queue, &event, 0);
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