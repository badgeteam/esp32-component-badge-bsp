#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "input_types.h"

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

/// @brief Input hook callback type
/// @param event The input event to process
/// @param user_data User data passed during registration
/// @return true if the event was consumed (should not be queued), false to pass through
typedef bool (*bsp_input_hook_cb_t)(bsp_input_event_t* event, void* user_data);

/// @brief Register an input hook callback
/// Hooks are called for every input event before it is queued.
/// If a hook returns true, the event is consumed and not queued.
/// @param callback The callback function
/// @param user_data User data to pass to the callback
/// @return hook ID (>= 0) on success, -1 on failure
int bsp_input_hook_register(bsp_input_hook_cb_t callback, void* user_data);

/// @brief Unregister an input hook
/// @param hook_id The hook ID returned by bsp_input_hook_register
void bsp_input_hook_unregister(int hook_id);

/// @brief Inject an input event into the queue
/// This bypasses hooks and directly queues the event.
/// @param event The event to inject
/// @return ESP-IDF error code
esp_err_t bsp_input_inject_event(bsp_input_event_t* event);
