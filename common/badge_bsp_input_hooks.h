// Board support package API: Input hook system
// SPDX-FileCopyrightText: 2025 Rene Schickbauer
// SPDX-License-Identifier: MIT

#pragma once

#include "bsp/input.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

// Maximum number of input hooks
#define BSP_INPUT_MAX_HOOKS 8

// Initialize the input hook system (call from bsp_input_initialize)
void bsp_input_hooks_init(void);

// Process an event through all registered hooks
// Returns true if any hook consumed the event (should not be queued)
bool bsp_input_hooks_process(bsp_input_event_t* event);
