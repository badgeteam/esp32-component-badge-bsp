// Board support package API: Input hook system implementation
// SPDX-FileCopyrightText: 2025 Nicolai Electronics
// SPDX-License-Identifier: MIT

#include "badge_bsp_input_hooks.h"
#include "bsp/input.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include <stddef.h>

typedef struct {
    bsp_input_hook_cb_t callback;
    void*               user_data;
    bool                in_use;
} bsp_input_hook_entry_t;

static bsp_input_hook_entry_t input_hooks[BSP_INPUT_MAX_HOOKS] = {0};
static SemaphoreHandle_t      input_hooks_mutex                = NULL;

void bsp_input_hooks_init(void) {
    if (input_hooks_mutex == NULL) {
        input_hooks_mutex = xSemaphoreCreateMutex();
    }
}

bool bsp_input_hooks_process(bsp_input_event_t* event) {
    if (input_hooks_mutex == NULL) {
        return false;
    }

    bool consumed = false;

    if (xSemaphoreTake(input_hooks_mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        for (int i = 0; i < BSP_INPUT_MAX_HOOKS; i++) {
            if (input_hooks[i].in_use && input_hooks[i].callback != NULL) {
                if (input_hooks[i].callback(event, input_hooks[i].user_data)) {
                    consumed = true;
                    break;  // Event consumed, stop processing
                }
            }
        }
        xSemaphoreGive(input_hooks_mutex);
    }

    return consumed;
}

int bsp_input_hook_register(bsp_input_hook_cb_t callback, void* user_data) {
    if (callback == NULL || input_hooks_mutex == NULL) {
        return -1;
    }

    int hook_id = -1;

    if (xSemaphoreTake(input_hooks_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        for (int i = 0; i < BSP_INPUT_MAX_HOOKS; i++) {
            if (!input_hooks[i].in_use) {
                input_hooks[i].callback  = callback;
                input_hooks[i].user_data = user_data;
                input_hooks[i].in_use    = true;
                hook_id                  = i;
                break;
            }
        }
        xSemaphoreGive(input_hooks_mutex);
    }

    return hook_id;
}

void bsp_input_hook_unregister(int hook_id) {
    if (hook_id < 0 || hook_id >= BSP_INPUT_MAX_HOOKS || input_hooks_mutex == NULL) {
        return;
    }

    if (xSemaphoreTake(input_hooks_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        input_hooks[hook_id].callback  = NULL;
        input_hooks[hook_id].user_data = NULL;
        input_hooks[hook_id].in_use    = false;
        xSemaphoreGive(input_hooks_mutex);
    }
}
