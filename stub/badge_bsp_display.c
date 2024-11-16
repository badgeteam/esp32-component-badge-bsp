#include "bsp/display.h"
#include "esp_err.h"

esp_err_t __attribute__((weak)) bsp_display_initialize(void) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t __attribute__((weak)) bsp_display_get_parameters(size_t* h_res, size_t* v_res,
                                                           lcd_color_rgb_pixel_format_t* color_fmt) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t __attribute__((weak)) bsp_display_get_panel(esp_lcd_panel_handle_t* panel) {
    return ESP_ERR_NOT_SUPPORTED;
}
