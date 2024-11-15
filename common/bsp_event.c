
// SPDX-License-Identifier: MIT

#include "bsp_event.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "list.h"



// Callback list entry.
typedef struct {
    dlist_node_t     node;
    bsp_event_type_t filter;
    bsp_event_cb_t   func;
    void            *cookie;
} cb_ent_t;

static dlist_t       callbacks;
static QueueHandle_t incoming_queue;
static QueueHandle_t outgoing_queue;
static TaskHandle_t  event_thread_handle;



// Event handler thread function.
static void event_thread(void *ignored) {
    (void)ignored;
    while (1) {
        bsp_event_t event;
        xQueueReceive(incoming_queue, &event, portMAX_DELAY);
        cb_ent_t *node = (cb_ent_t *)callbacks.head;
        while (node) {
            if (node->filter == BSP_EVENT_ANY || node->filter == event.type) {
                node->func(&event, node->cookie);
            }
            node = (cb_ent_t *)node->node.next;
        }
        xQueueSend(outgoing_queue, &event, 0);
    }
}

// Initialize the event queues.
void bsp_event_queue_init() {
    incoming_queue = xQueueCreate(8, sizeof(bsp_event_t));
    outgoing_queue = xQueueCreate(32, sizeof(bsp_event_t));
    xTaskCreate(event_thread, "bsp_event_worker", 8192, NULL, tskIDLE_PRIORITY, &event_thread_handle);
}


// Add an event to the BSP's event queue.
bool bsp_event_queue(bsp_event_t *event) {
    return xQueueSend(incoming_queue, event, 0) == pdTRUE;
}

// Add an event to the BSP's event queue from interrupt handler.
bool bsp_event_queue_from_isr(bsp_event_t *event) {
    return xQueueSendFromISR(incoming_queue, event, 0) == pdTRUE;
}

// Wait for a limited time for a BSP event to happen.
// If time is 0, only returns a valid event when there is one in the queue.
bool bsp_event_wait(bsp_event_t *event_out, uint64_t wait_ms) {
    // Clamp max wait time.
    TickType_t ticks;
    if (wait_ms > pdTICKS_TO_MS(portMAX_DELAY)) {
        ticks = portMAX_DELAY;
    } else {
        ticks = pdMS_TO_TICKS(wait_ms);
    }
    return xQueueReceive(outgoing_queue, event_out, ticks) == pdTRUE;
}


// Add a new callback to be run immediately when an event happens.
bsp_cb_handle_t bsp_event_add_callback(bsp_event_type_t filter, bsp_event_cb_t callback, void *cookie) {
    cb_ent_t *ent = malloc(sizeof(cb_ent_t));
    if (!ent)
        return NULL;
    ent->filter = filter;
    ent->func   = callback;
    ent->cookie = cookie;
    dlist_append(&callbacks, &ent->node);
    return ent;
}

// Remove an event callback so it will no longer be called when an event happens.
void bsp_event_remove_callback(bsp_cb_handle_t handle) {
    dlist_remove(&callbacks, (dlist_node_t *)handle);
}
