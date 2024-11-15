
// SPDX-License-Identifier: MIT

#pragma once

#include "bsp/input.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>



// Maximum possible wait time for a BSP event.
#define BSP_EVENT_MAX_WAIT INT64_MAX


// Possible types of BSP event.
typedef enum {
    // Magic to filter for all events.
    BSP_EVENT_ANY = -1,
    // Input changed event.
    BSP_EVENT_INPUT,
} bsp_event_type_t;

// Events sent by the BSP to the application.
typedef struct {
    // Event type.
    bsp_event_type_t  type;
    // Input event data.
    bsp_input_event_t input;
} bsp_event_t;

// Event callback function for use with `bsp_event_add_callback`.
typedef void (*bsp_event_cb_t)(bsp_event_t const *event, void *cookie);
// Handle for event callbacks registered with `bsp_event_add_callback`.
typedef void *bsp_cb_handle_t;



// Add an event to the BSP's event queue.
bool            bsp_event_queue(bsp_event_t *event);
// Add an event to the BSP's event queue from interrupt handler.
bool            bsp_event_queue_from_isr(bsp_event_t *event);
// Wait for a limited time for a BSP event to happen.
// If time is 0, only returns a valid event when there is one in the queue.
bool            bsp_event_wait(bsp_event_t *event_out, uint64_t wait_ms);
// Add a new callback to be run immediately when an event happens.
bsp_cb_handle_t bsp_event_add_callback(bsp_event_type_t filter, bsp_event_cb_t callback, void *cookie);
// Remove an event callback so it will no longer be called when an event happens.
void            bsp_event_remove_callback(bsp_cb_handle_t handle);
