#ifndef SSD1306_DRIVER_H
#define SSD1306_DRIVER_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "driver/i2c_master.h"

typedef i2c_master_dev_handle_t ssd1306_dev_handle_t;

// Display Initialization Functions

esp_err_t ssd1306_setup(ssd1306_dev_handle_t ssd1306_handle);

esp_err_t ssd1306_init(i2c_master_bus_handle_t bus_handle, ssd1306_dev_handle_t *ssd1306_ret_handle);

// Data Writing Functions

esp_err_t ssd1306_draw_pixel(uint8_t x, uint8_t y, bool color);

esp_err_t ssd1306_draw_character(uint8_t x_start, uint8_t y_start, char character);

esp_err_t ssd1306_draw_string(const char *text);

esp_err_t ssd1306_update(ssd1306_dev_handle_t ssd1306_handle);

esp_err_t ssd1306_clear(void);

#endif